#include <fs/compiler/detail/add_conditions.hpp>
#include <fs/compiler/detail/evaluate_as.hpp>
#include <fs/lang/position_tag.hpp>

#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>

#include <utility>

namespace
{

using namespace fs;
using namespace fs::compiler;
namespace ast = fs::parser::ast;

template <typename T>
[[nodiscard]] std::optional<compile_error>
add_range_condition(
	lang::comparison_type comparison_type,
	T value,
	lang::position_tag condition_origin,
	lang::range_condition<T>& target)
{
	switch (comparison_type)
	{
		case lang::comparison_type::equal:
		{
			if (target.lower_bound.has_value())
				return errors::lower_bound_redefinition{condition_origin, (*target.lower_bound).origin};

			if (target.upper_bound.has_value())
				return errors::upper_bound_redefinition{condition_origin, (*target.upper_bound).origin};

			target.set_exact(value, condition_origin);
			return std::nullopt;
		}
		case lang::comparison_type::less:
		{
			if (target.upper_bound.has_value())
				return errors::upper_bound_redefinition{condition_origin, (*target.upper_bound).origin};

			target.set_upper_bound(value, false, condition_origin);
			return std::nullopt;
		}
		case lang::comparison_type::less_equal:
		{
			if (target.upper_bound.has_value())
				return errors::upper_bound_redefinition{condition_origin, (*target.upper_bound).origin};

			target.set_upper_bound(value, true, condition_origin);
			return std::nullopt;
		}
		case lang::comparison_type::greater:
		{
			if (target.lower_bound.has_value())
				return errors::lower_bound_redefinition{condition_origin, (*target.lower_bound).origin};

			target.set_lower_bound(value, false, condition_origin);
			return std::nullopt;
		}
		case lang::comparison_type::greater_equal:
		{
			if (target.lower_bound.has_value())
				return errors::lower_bound_redefinition{condition_origin, (*target.lower_bound).origin};

			target.set_lower_bound(value, true, condition_origin);
			return std::nullopt;
		}
		default:
		{
			return errors::internal_compiler_error_during_range_evaluation{condition_origin};
		}
	}
}

[[nodiscard]] std::optional<compile_error>
add_comparison_condition(
	const ast::comparison_condition& condition,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data,
	lang::condition_set& condition_set)
{
	lang::position_tag condition_origin = parser::get_position_info(condition);

	switch (condition.property)
	{
		using detail::evaluate_as;

		case lang::comparison_condition_property::item_level:
		{
			std::variant<lang::level, compile_error> level_or_error = evaluate_as<lang::level>(condition.value, symbols, item_price_data);
			if (std::holds_alternative<compile_error>(level_or_error))
				return std::get<compile_error>(std::move(level_or_error));

			const auto& level = std::get<lang::level>(level_or_error);
			return add_range_condition(condition.comparison_type.value, level.value, condition_origin, condition_set.item_level);
		}
		case lang::comparison_condition_property::drop_level:
		{
			std::variant<lang::level, compile_error> level_or_error = evaluate_as<lang::level>(condition.value, symbols, item_price_data);
			if (std::holds_alternative<compile_error>(level_or_error))
				return std::get<compile_error>(std::move(level_or_error));

			const auto& level = std::get<lang::level>(level_or_error);
			return add_range_condition(condition.comparison_type.value, level.value, condition_origin, condition_set.drop_level);
		}
		case lang::comparison_condition_property::quality:
		{
			std::variant<lang::integer, compile_error> integer_or_error = evaluate_as<lang::integer>(condition.value, symbols, item_price_data);
			if (std::holds_alternative<compile_error>(integer_or_error))
				return std::get<compile_error>(std::move(integer_or_error));

			const auto& integer = std::get<lang::integer>(integer_or_error);
			return add_range_condition(condition.comparison_type.value, integer.value, condition_origin, condition_set.quality);
		}
		case lang::comparison_condition_property::rarity:
		{
			std::variant<lang::rarity, compile_error> rarity_or_error = evaluate_as<lang::rarity>(condition.value, symbols, item_price_data);
			if (std::holds_alternative<compile_error>(rarity_or_error))
				return std::get<compile_error>(std::move(rarity_or_error));

			const auto& rarity = std::get<lang::rarity>(rarity_or_error);
			return add_range_condition(condition.comparison_type.value, rarity, condition_origin, condition_set.rarity);
		}
		case lang::comparison_condition_property::sockets:
		{
			std::variant<lang::integer, compile_error> integer_or_error = evaluate_as<lang::integer>(condition.value, symbols, item_price_data);
			if (std::holds_alternative<compile_error>(integer_or_error))
				return std::get<compile_error>(std::move(integer_or_error));

			const auto& integer = std::get<lang::integer>(integer_or_error);
			return add_range_condition(condition.comparison_type.value, integer.value, condition_origin, condition_set.sockets);
		}
		case lang::comparison_condition_property::links:
		{
			std::variant<lang::integer, compile_error> integer_or_error = evaluate_as<lang::integer>(condition.value, symbols, item_price_data);
			if (std::holds_alternative<compile_error>(integer_or_error))
				return std::get<compile_error>(std::move(integer_or_error));

			const auto& integer = std::get<lang::integer>(integer_or_error);
			return add_range_condition(condition.comparison_type.value, integer.value, condition_origin, condition_set.links);
		}
		case lang::comparison_condition_property::height:
		{
			std::variant<lang::integer, compile_error> integer_or_error = evaluate_as<lang::integer>(condition.value, symbols, item_price_data);
			if (std::holds_alternative<compile_error>(integer_or_error))
				return std::get<compile_error>(std::move(integer_or_error));

			const auto& integer = std::get<lang::integer>(integer_or_error);
			return add_range_condition(condition.comparison_type.value, integer.value, condition_origin, condition_set.height);
		}
		case lang::comparison_condition_property::width:
		{
			std::variant<lang::integer, compile_error> integer_or_error = evaluate_as<lang::integer>(condition.value, symbols, item_price_data);
			if (std::holds_alternative<compile_error>(integer_or_error))
				return std::get<compile_error>(std::move(integer_or_error));

			const auto& integer = std::get<lang::integer>(integer_or_error);
			return add_range_condition(condition.comparison_type.value, integer.value, condition_origin, condition_set.width);
		}
		case lang::comparison_condition_property::stack_size:
		{
			std::variant<lang::integer, compile_error> integer_or_error = evaluate_as<lang::integer>(condition.value, symbols, item_price_data);
			if (std::holds_alternative<compile_error>(integer_or_error))
				return std::get<compile_error>(std::move(integer_or_error));

			const auto& integer = std::get<lang::integer>(integer_or_error);
			return add_range_condition(condition.comparison_type.value, integer.value, condition_origin, condition_set.stack_size);
		}
		case lang::comparison_condition_property::gem_level:
		{
			std::variant<lang::integer, compile_error> integer_or_error = evaluate_as<lang::integer>(condition.value, symbols, item_price_data);
			if (std::holds_alternative<compile_error>(integer_or_error))
				return std::get<compile_error>(std::move(integer_or_error));

			const auto& integer = std::get<lang::integer>(integer_or_error);
			return add_range_condition(condition.comparison_type.value, integer.value, condition_origin, condition_set.gem_level);
		}
		case lang::comparison_condition_property::map_tier:
		{
			std::variant<lang::integer, compile_error> integer_or_error = evaluate_as<lang::integer>(condition.value, symbols, item_price_data);
			if (std::holds_alternative<compile_error>(integer_or_error))
				return std::get<compile_error>(std::move(integer_or_error));

			const auto& integer = std::get<lang::integer>(integer_or_error);
			return add_range_condition(condition.comparison_type.value, integer.value, condition_origin, condition_set.map_tier);
		}
		default:
		{
			return errors::internal_compiler_error_during_comparison_condition_evaluation{parser::get_position_info(condition)};
		}
	}
}

[[nodiscard]] std::variant<std::vector<std::string>, compile_error>
array_to_strings(
	lang::array_object array)
{
	std::vector<std::string> result;
	for (lang::object& obj : array)
	{
		if (!std::holds_alternative<lang::string>(obj.value))
			return errors::type_mismatch{
				lang::object_type::string,
				obj.type(),
				obj.value_origin};

		auto& string = std::get<lang::string>(obj.value);
		result.push_back(std::move(string.value));
	}

	return result;
}

[[nodiscard]] std::optional<compile_error>
add_string_condition_impl(
	std::vector<std::string> strings,
	bool is_exact_match,
	lang::position_tag condition_origin,
	lang::strings_condition& target)
{
	if (target.strings != nullptr)
		return errors::condition_redefinition{condition_origin, target.origin};

	target.strings = std::make_shared<std::vector<std::string>>(std::move(strings));
	target.exact_match_required = is_exact_match;
	target.origin = condition_origin;
	return std::nullopt;
}

[[nodiscard]] std::optional<compile_error>
add_string_condition(
	const parser::ast::string_condition& condition,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data,
	lang::condition_set& condition_set)
{
	std::variant<lang::array_object, compile_error> array_or_error = detail::evaluate_as<lang::array_object>(condition.value, symbols, item_price_data);
	if (std::holds_alternative<compile_error>(array_or_error))
		return std::get<compile_error>(std::move(array_or_error));

	auto& array = std::get<lang::array_object>(array_or_error);
	std::variant<std::vector<std::string>, compile_error> strings_or_error = array_to_strings(std::move(array));
	if (std::holds_alternative<compile_error>(strings_or_error))
		return std::get<compile_error>(std::move(strings_or_error));

	auto& strings = std::get<std::vector<std::string>>(strings_or_error);
	const bool is_exact_match = condition.exact_match.required;
	const lang::position_tag condition_origin = parser::get_position_info(condition);

	switch (condition.property)
	{
		case lang::string_condition_property::class_:
		{
			return add_string_condition_impl(std::move(strings), is_exact_match, condition_origin, condition_set.class_);
		}
		case lang::string_condition_property::base_type:
		{
			return add_string_condition_impl(std::move(strings), is_exact_match, condition_origin, condition_set.base_type);
		}
		case lang::string_condition_property::has_explicit_mod:
		{
			return add_string_condition_impl(std::move(strings), is_exact_match, condition_origin, condition_set.has_explicit_mod);
		}
		case lang::string_condition_property::has_enchantment:
		{
			return add_string_condition_impl(std::move(strings), is_exact_match, condition_origin, condition_set.has_enchantment);
		}
		default:
		{
			return errors::internal_compiler_error_during_string_condition_evaluation{condition_origin};
		}
	}

}

[[nodiscard]] std::optional<compile_error>
add_boolean_condition_impl(
	lang::boolean boolean,
	lang::position_tag condition_origin,
	std::optional<lang::boolean_condition>& target)
{
	if (target.has_value())
		return errors::condition_redefinition{condition_origin, (*target).origin};

	target = lang::boolean_condition{boolean, condition_origin};
	return std::nullopt;
}

[[nodiscard]] std::optional<compile_error>
add_boolean_condition(
	const parser::ast::boolean_condition& condition,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data,
	lang::condition_set& condition_set)
{
	std::variant<lang::boolean, compile_error> boolean_or_error = detail::evaluate_as<lang::boolean>(condition.value, symbols, item_price_data);
	if (std::holds_alternative<compile_error>(boolean_or_error))
		return std::get<compile_error>(std::move(boolean_or_error));

	auto& boolean = std::get<lang::boolean>(boolean_or_error);
	lang::position_tag condition_origin = parser::get_position_info(condition);

	switch (condition.property)
	{
		case lang::boolean_condition_property::identified:
		{
			return add_boolean_condition_impl(boolean, condition_origin, condition_set.is_identified);
		}
		case lang::boolean_condition_property::corrupted:
		{
			return add_boolean_condition_impl(boolean, condition_origin, condition_set.is_corrupted);
		}
		case lang::boolean_condition_property::elder_item:
		{
			return add_boolean_condition_impl(boolean, condition_origin, condition_set.is_elder_item);
		}
		case lang::boolean_condition_property::shaper_item:
		{
			return add_boolean_condition_impl(boolean, condition_origin, condition_set.is_shaper_item);
		}
		case lang::boolean_condition_property::fractured_item:
		{
			return add_boolean_condition_impl(boolean, condition_origin, condition_set.is_fractured_item);
		}
		case lang::boolean_condition_property::synthesised_item:
		{
			return add_boolean_condition_impl(boolean, condition_origin, condition_set.is_synthesised_item);
		}
		case lang::boolean_condition_property::any_enchantment:
		{
			return add_boolean_condition_impl(boolean, condition_origin, condition_set.is_enchanted);
		}
		case lang::boolean_condition_property::shaped_map:
		{
			return add_boolean_condition_impl(boolean, condition_origin, condition_set.is_shaped_map);
		}
		default:
		{
			return errors::internal_compiler_error_during_boolean_condition_evaluation{condition_origin};
		}
	}
}

[[nodiscard]] std::optional<compile_error>
add_socket_group_condition_impl(
	lang::socket_group socket_group,
	lang::position_tag condition_origin,
	std::optional<lang::socket_group_condition>& target)
{
	if (target.has_value())
		return errors::condition_redefinition{condition_origin, (*target).origin};

	target = lang::socket_group_condition{socket_group, condition_origin};
	return std::nullopt;
}

[[nodiscard]] std::optional<compile_error>
add_socket_group_condition(
	const parser::ast::socket_group_condition& condition,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data,
	lang::condition_set& condition_set)
{
	std::variant<lang::socket_group, compile_error> socket_group_or_error = detail::evaluate_as<lang::socket_group>(condition.value, symbols, item_price_data);
	if (std::holds_alternative<compile_error>(socket_group_or_error))
		return std::get<compile_error>(std::move(socket_group_or_error));

	const auto& socket_group = std::get<lang::socket_group>(socket_group_or_error);
	lang::position_tag condition_origin = parser::get_position_info(condition);
	return add_socket_group_condition_impl(socket_group, condition_origin, condition_set.socket_group);
}

} // namespace

namespace fs::compiler::detail
{

namespace ast = parser::ast;
namespace x3 = boost::spirit::x3;

std::optional<compile_error> add_conditions(
	const std::vector<ast::condition>& conditions,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data,
	lang::condition_set& condition_set)
{
	for (const ast::condition& condition : conditions)
	{
		auto error = condition.apply_visitor(x3::make_lambda_visitor<std::optional<compile_error>>(
			[&](const ast::comparison_condition& comparison_condition)
			{
				return add_comparison_condition(comparison_condition, symbols, item_price_data, condition_set);
			},
			[&](const ast::string_condition& string_condition)
			{
				return add_string_condition(string_condition, symbols, item_price_data, condition_set);
			},
			[&](const ast::boolean_condition& boolean_condition)
			{
				return add_boolean_condition(boolean_condition, symbols, item_price_data, condition_set);
			},
			[&](const ast::socket_group_condition& socket_group_condition)
			{
				return add_socket_group_condition(socket_group_condition, symbols, item_price_data, condition_set);
			}));

		if (error)
			return error;
	}

	return std::nullopt;
}

}
