#include <fs/parser/parser.hpp>
#include <fs/parser/ast_adapted.hpp> // required adaptation info for log::structure_printer
#include <fs/compiler/compiler.hpp>
#include <fs/compiler/detail/autogen.hpp>
#include <fs/compiler/detail/evaluate.hpp>
#include <fs/compiler/detail/actions.hpp>
#include <fs/compiler/detail/conditions.hpp>
#include <fs/lang/item_filter.hpp>
#include <fs/lang/item.hpp>
#include <fs/lang/keywords.hpp>
#include <fs/log/structure_printer.hpp>
#include <fs/utility/assert.hpp>
#include <fs/utility/string_helpers.hpp>
#include <fs/utility/monadic.hpp>
#include <fs/utility/visitor.hpp>
#include <fs/version.hpp>

#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <iterator>
#include <optional>
#include <sstream>

namespace fs::compiler {

namespace ast = fs::parser::ast;
using parser::position_tag_of;
using boost::spirit::x3::make_lambda_visitor;

namespace {

[[nodiscard]] lang::import_block
evaluate(const ast::common::import_statement& statement)
{
	return lang::import_block{detail::evaluate(statement.path), statement.is_optional, position_tag_of(statement)};
}

[[nodiscard]] boost::optional<lang::item_visibility>
evaluate(settings st, ast::common::static_visibility_statement visibility, diagnostics_store& diagnostics)
{
	if (st.ruthless_mode && visibility.value == lang::item_visibility_policy::hide) {
		diagnostics.push_error_invalid_statement(
			position_tag_of(visibility),
			"Hide is only available in Normal filters - use Minimal in Ruthless");

		if (st.error_handling.stop_on_error)
			return boost::none;
		else
			return lang::item_visibility{lang::item_visibility_policy::minimal, position_tag_of(visibility)};
	}

	if (!st.ruthless_mode && visibility.value == lang::item_visibility_policy::minimal) {
		diagnostics.push_error_invalid_statement(
			position_tag_of(visibility),
			"Minimal is only available in Ruthless filters - use Hide instead");

		if (st.error_handling.stop_on_error)
			return boost::none;
		else
			return lang::item_visibility{lang::item_visibility_policy::hide, position_tag_of(visibility)};
	}

	return lang::item_visibility{visibility.value, position_tag_of(visibility)};
}

[[nodiscard]] boost::optional<lang::boolean>
fold_booleans_using_and(const lang::object& obj, diagnostics_store& diagnostics)
{
	lang::boolean result{true, obj.origin};
	for (const lang::single_object& so : obj.values) {
		/*
		 * This implementation could short-circuit (return false on first false)
		 * but it tries to run full loop to ensure that all subobjects are booleans.
		 */
		auto b = detail::get_as<lang::boolean>(so, diagnostics);
		if (!b)
			return boost::none;

		result.value = result.value && (*b).value;
	}

	return result;
}

[[nodiscard]] boost::optional<lang::item_visibility>
evaluate(
	settings st,
	ast::sf::dynamic_visibility_statement visibility,
	const symbol_table& symbols,
	diagnostics_store& diagnostics)
{
	lang::item_visibility_policy policy = visibility.policy.value;

	if (st.ruthless_mode && policy == lang::item_visibility_policy::hide) {
		diagnostics.push_error_invalid_statement(
			position_tag_of(visibility),
			"ShowHide is only available in Normal filters - use ShowMinimal in Ruthless");

		if (st.error_handling.stop_on_error)
			return boost::none;
		else
			policy = lang::item_visibility_policy::minimal;
	}

	if (!st.ruthless_mode && policy == lang::item_visibility_policy::minimal) {
		diagnostics.push_error_invalid_statement(
			position_tag_of(visibility),
			"ShowMinimal is only available in Ruthless filters - use ShowHide instead");

		if (st.error_handling.stop_on_error)
			return boost::none;
		else
			policy = lang::item_visibility_policy::hide;
	}

	return detail::evaluate_sequence(st, visibility.seq, symbols, diagnostics)
		.flat_map([&](lang::object obj) {
			return fold_booleans_using_and(obj, diagnostics);
		})
		.map([&](lang::boolean b) {
			const lang::position_tag origin = position_tag_of(visibility);

			if (b.value)
				return lang::item_visibility{lang::item_visibility_policy::show, origin};
			else
				return lang::item_visibility{policy, origin};
		});
}

[[nodiscard]] boost::optional<lang::item_visibility>
evaluate(
	settings st,
	ast::sf::visibility_statement visibility,
	const symbol_table& symbols,
	diagnostics_store& diagnostics)
{
	return visibility.apply_visitor(make_lambda_visitor<boost::optional<lang::item_visibility>>(
		[&](ast::common::static_visibility_statement visibility) { return evaluate(st, visibility, diagnostics); },
		[&](ast::sf::dynamic_visibility_statement visibility) { return evaluate(st, visibility, symbols, diagnostics); }
	));
}

[[nodiscard]] boost::optional<lang::spirit_item_filter_block>
make_spirit_filter_block(
	settings st,
	lang::item_visibility visibility,
	detail::spirit_protoconditions conditions,
	lang::action_set actions,
	lang::block_continuation continuation,
	diagnostics_store& diagnostics)
{
	if (conditions.price_range.has_bound() && !conditions.autogen.has_value()) {
		std::optional<lang::position_tag> price_first_origin = conditions.price_range.first_origin();
		FS_ASSERT_MSG(
			price_first_origin.has_value(),
			"Price condition has a bound. It must have at least 1 origin.");
		diagnostics.push_message(make_error(
			diagnostic_message_id::price_without_autogen,
			visibility.origin,
			"generation of a block with price bound has missing autogeneration specifier"));
		diagnostics.push_message(make_note_minor(*price_first_origin, "price bound specified here"));
		if (auto price_second_origin = conditions.price_range.second_origin(); price_second_origin.has_value()) {
			diagnostics.push_message(make_note_minor(*price_second_origin, "another price bound specified here"));
		}
		return boost::none;
	}

	std::optional<lang::autogen_extension> result_autogen;
	if (conditions.autogen) {
		const detail::autogen_protocondition& autogen = *conditions.autogen;

		auto func = detail::make_autogen_func(
			st, conditions.official, conditions.price_range, autogen, visibility.origin, diagnostics);
		// If the user specified autogeneration, func creation should succeed.
		// Otherwise the entire block is invalid and thus none is returned.
		if (!func)
			return boost::none;

		result_autogen = lang::autogen_extension{std::move(func), conditions.price_range, autogen.origin};
	}

	return lang::spirit_item_filter_block{
		lang::item_filter_block(
			visibility,
			std::move(conditions.official),
			std::move(actions),
			continuation
		),
		std::move(result_autogen)};
}

[[nodiscard]] lang::block_continuation
to_block_continuation(
	boost::optional<ast::common::continue_statement> statement)
{
	if (statement)
		return lang::block_continuation{position_tag_of(*statement)};
	else
		return lang::block_continuation{std::nullopt};
}

[[nodiscard]] const named_tree*
evaluate_name_as_tree(
	const ast::sf::name& name,
	const symbol_table& symbols,
	diagnostics_store& diagnostics)
{
	const auto it = symbols.trees.find(name.value.value);
	if (it == symbols.trees.end()) {
		diagnostics.push_error_no_such_name(position_tag_of(name));
		return nullptr;
	}

	return &it->second;
}

/*
 * This function is large, but:
 * - it contains uniquely recursive non-duplicate code
 * - any extracted function would have ~10-line-long function header
 *
 * Thus, it is better to leave it as it is.
 */
[[nodiscard]] bool
compile_statements_recursively_impl(
	settings st,
	const std::vector<ast::sf::statement>& statements,
	const symbol_table& symbols,
	detail::spirit_protoconditions& conditions,
	lang::action_set& actions,
	std::vector<lang::spirit_block_variant>& blocks,
	diagnostics_store& diagnostics)
{
	// Conditions can only be followed by a nested block
	bool condition_present = false;
	// explicitly make copies of parent conditions and actions - the call stack will preserve
	// old instances while nested blocks can add additional ones that have limited lifetime
	detail::spirit_protoconditions nested_conditions = conditions;

	for (const ast::sf::statement& statement : statements) {
		auto status = statement.apply_visitor(make_lambda_visitor<bool>(
			[&](const ast::common::import_statement& statement) {
				blocks.push_back(lang::spirit_block_variant(evaluate(statement)));
				return true;
			},
			[&](const ast::sf::condition& cond) {
				condition_present = true;
				return detail::spirit_filter_add_condition(
					st, cond, symbols, nested_conditions, diagnostics);
			},
			[&](const ast::sf::action& action) {
				if (condition_present) {
					diagnostics.push_error_invalid_statement(
						position_tag_of(action),
						"action after condition - place it before condition or inside condition's block");
					return false;
				}

				return detail::spirit_filter_add_action(st, action, symbols, actions, diagnostics);
			},
			[&](const ast::sf::expand_statement& statement) {
				if (condition_present) {
					diagnostics.push_error_invalid_statement(
						position_tag_of(statement),
						"expansion after condition - place it before condition or inside condition's block");
					return false;
				}

				if (statement.seq.size() != 1u) {
					diagnostics.push_error_invalid_amount_of_arguments(
						1, 1, static_cast<int>(statement.seq.size()), position_tag_of(statement.seq));
					return false;
				}

				const ast::sf::primitive_value& primitive = statement.seq.front();
				return primitive.apply_visitor(make_lambda_visitor<bool>(
					[&](const ast::common::unknown_expression& expr) {
						diagnostics.push_error_unknown_expression(position_tag_of(expr));
						return false;
					},
					[&](const ast::common::literal_expression& expr) {
						diagnostics.push_message(make_error(
							diagnostic_message_id::type_mismatch,
							position_tag_of(expr),
							"type mismatch, expected a block but got a literal expression"));
						return false;
					},
					[&](const ast::sf::name& name) {
						const named_tree* tree = evaluate_name_as_tree(name, symbols, diagnostics);
						if (!tree)
							return false;

						diagnostics.push_layer(position_tag_of(statement));
						const auto status = compile_statements_recursively_impl(
							st, tree->statements, symbols, conditions, actions, blocks, diagnostics);
						diagnostics.pop_layer();
						return status;
					}
				));
			},
			[&](const ast::sf::behavior_statement& bs) {
				if (condition_present) {
					diagnostics.push_error_invalid_statement(
						position_tag_of(bs),
						"visibility statement after condition - place it before condition or inside condition's block");
					return false;
				}

				boost::optional<lang::item_visibility> visibility = evaluate(st, bs.visibility, symbols, diagnostics);
				if (!visibility)
					return false;

				boost::optional<lang::spirit_item_filter_block> block = make_spirit_filter_block(
					st,
					*visibility,
					conditions,
					actions,
					to_block_continuation(bs.continue_),
					diagnostics);

				if (!block)
					return false;

				blocks.push_back(lang::spirit_block_variant(std::move(*block)));
				return true;
			},
			[&](const ast::sf::rule_block& nested_block) {
				lang::action_set nested_actions = actions;

				const auto status = compile_statements_recursively_impl(
					st, nested_block, symbols, nested_conditions, nested_actions, blocks, diagnostics);

				// conditions used for the nested block, restore them to the state from the parent block
				nested_conditions = conditions;
				condition_present = false;

				return status;
			},
			[&](const ast::sf::unknown_statement& statement) {
				const auto dead_conditions = {
					lang::keywords::rf::prophecy, lang::keywords::rf::gem_quality_type
				};

				for (auto cond : dead_conditions) {
					if (statement.name.value == cond) {
						diagnostics.push_error_dead_condition(
							position_tag_of(statement.name), "this condition no longer exists");
						return false;
					}
				}

				diagnostics.push_error_unknown_statement(position_tag_of(statement.name));
				return false;
			}
		));

		if (!status) {
			for (lang::position_tag origin : diagnostics.layers())
				diagnostics.push_message(make_note_minor(origin, "happened inside expansion"));
		}

		if (!status && st.error_handling.stop_on_error)
			return false;
	}

	return true;
}

[[nodiscard]] boost::optional<std::vector<lang::spirit_block_variant>>
compile_statements_recursively(
	settings st,
	const std::vector<ast::sf::statement>& statements,
	const symbol_table& symbols,
	diagnostics_store& diagnostics)
{
	// start with empty conditions and actions
	// that's the default state before any nesting
	detail::spirit_protoconditions root_conditions;
	lang::action_set root_actions;
	std::vector<lang::spirit_block_variant> blocks;

	const auto status = compile_statements_recursively_impl(
		st, statements, symbols, root_conditions, root_actions, blocks, diagnostics);

	// This is the place where non-fatal errors are no longer tolerable.
	// The entire filter has already been processed (with all possible warnings and (non-)fatal errors).
	// The user should see a lot of diagnostics at this point but should not be allowed to continue with invalid state.
	// Returning filter blocks constructed from (non-)fatal errors can produce invalid filtering results.
	if (status)
		return blocks;
	else
		return boost::none;
}

std::string make_preamble(const lang::market::item_price_metadata& metadata)
{
	std::string preamble =
R"(# autogenerated by Filter Spirit - an advanced item filter generator for Path of Exile
# Write filters in an enhanced language with the ability to query item prices. Refresh whenever you want.
#
# read tutorial, browse documentation, ask questions, report bugs on: github.com/Xeverous/filter_spirit
#
# or contact directly:
#     reddit : /u/Xeverous
#     Discord: Xeverous_2151
#     in game: pathofexile.com/account/view-profile/Xeverous
#
# Generation info:
)";
	preamble +=
"#     Filter Spirit version     : " + to_string(version::current()) + "\n"
"#     filter generation date    : " + utility::ptime_to_pretty_string(boost::posix_time::microsec_clock::universal_time()) + "\n"
"#     item price data downloaded: " + utility::ptime_to_pretty_string(metadata.download_date) + "\n"
"#     item price data from      : " + std::string(lang::to_string(metadata.data_source)) + "\n"
"#     item price data for league: " + metadata.league_name + "\n"
"#\n"
"# May the drops be with you.\n"
"\n";

	return preamble;
}

std::optional<lang::spirit_item_filter> parse_and_compile_spirit_filter(
	std::string_view input,
	settings st,
	log::logger& logger)
{
	logger.info() << "Parsing filter template...\n";
	std::variant<parser::parsed_spirit_filter, parser::parse_failure_data> parse_result = parser::parse_spirit_filter(input);

	if (std::holds_alternative<parser::parse_failure_data>(parse_result)) {
		parser::print_parse_errors(std::get<parser::parse_failure_data>(parse_result), logger);
		return std::nullopt;
	}

	logger.info() << "Parse successful.\n";
	const auto& parse_data = std::get<parser::parsed_spirit_filter>(parse_result);

	if (st.print_ast)
		log::structure_printer()(parse_data.ast);

	logger.info() << "Resolving filter template symbols...\n";

	diagnostics_store diagnostics_symbols;
	const std::optional<symbol_table> symbols =
		resolve_spirit_filter_symbols(st, parse_data.ast.definitions, diagnostics_symbols);
	diagnostics_symbols.output_messages(parse_data.metadata, logger);

	if (!symbols)
		return std::nullopt;

	logger.info() << "Symbols resolved.\n";
	logger.info() << "Compiling filter template...\n";

	diagnostics_store diagnostics_spirit_filter;
	std::optional<lang::spirit_item_filter> spirit_filter =
		compile_spirit_filter_statements(st, parse_data.ast.statements, *symbols, diagnostics_spirit_filter);
	diagnostics_spirit_filter.output_messages(parse_data.metadata, logger);

	return spirit_filter;
}

} // namespace

// placed in this file to reuse code and avoid creating symbol_table.cpp for just 1 function
bool
symbol_table::add_symbol(
	settings st,
	const ast::sf::constant_definition& def,
	diagnostics_store& diagnostics)
{
	const lang::position_tag definition_origin = position_tag_of(def.value_name);
	const std::string& name = def.value_name.value.value;

	if (const auto it = objects.find(name); it != objects.end()) {
		const lang::position_tag existing_origin = position_tag_of(it->second.name_origin);
		diagnostics.push_error_name_already_exists(existing_origin, definition_origin);
		return false;
	}

	if (const auto it = trees.find(name); it != trees.end()) {
		const lang::position_tag existing_origin = position_tag_of(it->second.name_origin);
		diagnostics.push_error_name_already_exists(existing_origin, definition_origin);
		return false;
	}

	return def.value.apply_visitor(make_lambda_visitor<bool>(
		[&](const ast::sf::sequence& seq) {
			return detail::evaluate_sequence(st, seq, *this, diagnostics)
				.map([&](lang::object obj) {
					const bool success = objects.emplace(
						name,
						named_object{std::move(obj), definition_origin}).second;
					FS_ASSERT_MSG(success, "Insertion should succeed.");
					return success;
				})
				.value_or(false);
		},
		[&](const ast::sf::statement_list_expression& expr) {
			const bool success = trees.emplace(
				name,
				named_tree{expr, definition_origin}).second;
			FS_ASSERT_MSG(success, "Insertion should succeed.");
			return success;
		}
	));
}

std::optional<lang::item_filter>
compile_real_filter(
	settings st,
	const ast::rf::ast_type& ast,
	diagnostics_store& diagnostics)
{
	lang::item_filter filter;
	filter.blocks.reserve(ast.size());

	for (const ast::rf::block_variant& block_variant : ast) {
		using result_type = boost::optional<lang::block_variant>;

		auto result_block = block_variant.apply_visitor(make_lambda_visitor<result_type>(
			[&](const ast::common::import_statement& statement) -> result_type {
				return lang::block_variant(evaluate(statement));
			},
			[&](const ast::rf::filter_block& block) -> result_type {
				auto maybe_visibility = evaluate(st, block.visibility, diagnostics);
				if (!maybe_visibility)
					return boost::none;

				lang::item_filter_block filter_block(*maybe_visibility);

				for (const auto& rule : block.rules) {
					const bool result = rule.apply_visitor(make_lambda_visitor<bool>(
						[&](const ast::rf::condition& c) {
							return detail::real_filter_add_condition(
								st, c, filter_block.conditions, diagnostics);
						},
						[&](const ast::rf::action& a) {
							return detail::real_filter_add_action(
								st, a, filter_block.actions, diagnostics);
						},
						[&](ast::rf::continue_statement cont) {
							filter_block.continuation.origin = position_tag_of(cont);
							return true;
						}
					));

					if (!result && st.error_handling.stop_on_error)
						return boost::none;
				}

				return lang::block_variant(filter_block);
			}
		));

		if (result_block)
			filter.blocks.push_back(std::move(*result_block));
		else if (st.error_handling.stop_on_error)
			return std::nullopt;
	}

	if (!diagnostics.is_ok(st.error_handling.treat_warnings_as_errors))
		return std::nullopt;

	return filter;
}

std::optional<symbol_table>
resolve_spirit_filter_symbols(
	settings st,
	const std::vector<ast::sf::definition>& definitions,
	diagnostics_store& diagnostics)
{
	symbol_table symbols;

	for (const auto& def : definitions) {
		const bool result = symbols.add_symbol(st, def.def, diagnostics);

		if (!result && st.error_handling.stop_on_error)
			return std::nullopt;
	}

	return symbols;
}

std::optional<lang::spirit_item_filter>
compile_spirit_filter_statements(
	settings st,
	const std::vector<ast::sf::statement>& statements,
	const symbol_table& symbols,
	diagnostics_store& diagnostics)
{
	boost::optional<std::vector<lang::spirit_block_variant>> blocks =
		compile_statements_recursively(st, statements, symbols, diagnostics);

	// This function is the point when we want to stop proceeding on any errors.
	// Otherwise the returned filter could contain broken state.
	if (!blocks)
		return std::nullopt;

	if (!diagnostics.is_ok(st.error_handling.treat_warnings_as_errors))
		return std::nullopt;

	return lang::spirit_item_filter{std::move(*blocks)};
}

lang::item_filter
make_item_filter(
	const lang::spirit_item_filter& filter_template,
	const lang::market::item_price_data& item_price_data)
{
	std::vector<lang::block_variant> result_blocks;
	result_blocks.reserve(filter_template.blocks.size());

	for (const lang::spirit_block_variant& block_variant : filter_template.blocks) {
		std::visit(utility::visitor{
			[&](const lang::import_block& block) {
				result_blocks.push_back(lang::block_variant(block));
			},
			[&](const lang::spirit_item_filter_block& block) {
				if (block.autogen) {
					const auto& autogen = *block.autogen;

					lang::block_generation_info block_gen_info{
						block.block.visibility,
						block.block.actions,
						block.block.continuation,
						autogen.origin,
						autogen.price_range
					};

					autogen.blocks_generator(
						block_gen_info, item_price_data, lang::generated_blocks_consumer{std::ref(result_blocks)});
				}
				else {
					result_blocks.push_back(lang::block_variant(block.block));
				}
			}
		}, block_variant);
	}

	return lang::item_filter{std::move(result_blocks)};
}

std::string item_filter_to_string_without_preamble(const lang::item_filter& filter)
{
	std::stringstream ss;
	filter.print(ss);
	return ss.str();
}

std::string
item_filter_to_string_with_preamble(
	const lang::item_filter& filter,
	const lang::market::item_price_metadata& item_price_metadata)
{
	return make_preamble(item_price_metadata) + item_filter_to_string_without_preamble(filter);
}

std::optional<std::string> parse_compile_generate_spirit_filter_without_preamble(
	std::string_view input,
	const lang::market::item_price_data& item_price_data,
	settings st,
	log::logger& logger)
{
	logger.info() << "" << item_price_data; // add << "" to workaround calling <<(rvalue, item_price_data)

	std::optional<fs::lang::spirit_item_filter> spirit_filter = parse_and_compile_spirit_filter(input, st, logger);

	if (!spirit_filter)
		return std::nullopt;

	lang::item_filter filter = make_item_filter(*spirit_filter, item_price_data);
	logger.info() << "Compilation successful.\n";

	return item_filter_to_string_without_preamble(filter);
}

std::optional<std::string> parse_compile_generate_spirit_filter_with_preamble(
	std::string_view input,
	const lang::market::item_price_report& report,
	settings st,
	log::logger& logger)
{
	std::optional<std::string> maybe_filter =
		parse_compile_generate_spirit_filter_without_preamble(input, report.data, st, logger);

	if (!maybe_filter)
		return std::nullopt;

	return make_preamble(report.metadata) + std::move(*maybe_filter);
}

}
