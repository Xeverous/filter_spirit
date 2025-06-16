#include <fs/compiler/detail/conditions.hpp>
#include <fs/compiler/detail/evaluate.hpp>
#include <fs/lang/position_tag.hpp>
#include <fs/lang/conditions.hpp>
#include <fs/lang/limits.hpp>
#include <fs/utility/assert.hpp>

#include <boost/container/static_vector.hpp>
#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>
#include <boost/optional.hpp>

#include <utility>

using boost::spirit::x3::make_lambda_visitor;

namespace fs::compiler::detail
{

using parser::position_tag_of;

namespace
{

struct protocondition
{
	parser::ast::common::comparison_expression comparison;
	lang::object obj;
	lang::position_tag condition_origin;
};

template <typename R, typename... Args>
using condition_factory_func = std::shared_ptr<R> (Args...);

using boolean_condition_factory_func = condition_factory_func<
	lang::boolean_condition, lang::boolean, lang::position_tag>;
template <typename T>
using range_bound_condition_factory_func = condition_factory_func<
	lang::range_bound_condition<T>, lang::range_bound<T>, bool, lang::position_tag>;
template <typename T>
using value_list_condition_factory_func = condition_factory_func<
	lang::value_list_condition<T>,
	typename lang::value_list_condition<T>::container_type,
	bool,
	lang::position_tag>;
using string_comparison_condition_factory_func = condition_factory_func<
	lang::string_comparison_condition,
	lang::equality_comparison_type,
	lang::string_comparison_condition::container_type,
	lang::position_tag>;
using counted_string_comparison_condition_factory_func = condition_factory_func<
	lang::counted_string_comparison_condition,
	lang::comparison_type,
	std::optional<lang::integer>,
	lang::counted_string_comparison_condition::container_type,
	lang::position_tag>;
using socket_specification_condition_factory_func = condition_factory_func<
	lang::socket_specification_condition,
	lang::comparison_type,
	lang::socket_specification_condition::container_type,
	lang::position_tag>;

[[nodiscard]] bool
check_no_counted_comparison(
	boost::optional<parser::ast::common::integer_literal> count,
	diagnostics_store& diagnostics)
{
	if (count) {
		diagnostics.push_error_invalid_expression(
			position_tag_of(*count),
			"this condition does not support counted comparison");
		return false;
	}

	return true;
}

[[nodiscard]] std::shared_ptr<lang::boolean_condition> // null on failure
make_boolean_condition(
	protocondition pc,
	boolean_condition_factory_func& func,
	diagnostics_store& diagnostics)
{
	if (!check_no_counted_comparison(pc.comparison.integer, diagnostics))
		return nullptr;

	if (pc.comparison.operator_.value != lang::comparison_type::equal) {
		// Game client accepts all operators for boolean conditions.
		// While all work as expected (True is 1 and False is 0) only equality makes sense.
		diagnostics.push_error_invalid_operator(
			position_tag_of(pc.comparison.operator_),
			"Invalid operator for boolean condition: use only nothing/=");
		return nullptr;
	}

	if (!check_object_size(pc.obj, 1, 1, diagnostics))
		return nullptr;

	FS_ASSERT(pc.obj.values.size() == 1u);

	return get_as<lang::boolean>(pc.obj.values.front(), diagnostics)
		.map([&](lang::boolean b) { return func(b, pc.condition_origin); })
		.value_or(nullptr);
}

[[nodiscard]] boost::optional<lang::influence_spec>
make_influence_spec(settings st, const lang::object& obj, diagnostics_store& diagnostics)
{
	if (!check_object_size(obj, 1, boost::none, diagnostics))
		return boost::none;

	lang::influence_spec spec;

	if (obj.values.size() == 1u) {
		diagnostics_store diagnostics_none; // ignore logs and move on if get_as<none> fails
		if (auto maybe_none = get_as<lang::none_type>(obj.values.front(), diagnostics_none); maybe_none) {
			spec.none = (*maybe_none).origin;
			return spec;
		}
	}

	for (const lang::single_object& sobj : obj.values) {
		auto maybe_infl = get_as<lang::influence>(sobj, diagnostics);
		if (!maybe_infl)
			return boost::none;

		bool error = false;
		auto set_influence = [&](std::optional<lang::position_tag>& field, lang::position_tag infl) {
			if (field) {
				diagnostics.push_message(make_warning(
					diagnostic_message_id::duplicate_influence, infl, "duplicate influence"));
				diagnostics.push_message(make_note_first_defined_here(*field));

				if (st.error_handling.treat_warnings_as_errors)
					error = true;
			}
			else {
				field = infl;
			}
		};

		const lang::influence& infl = *maybe_infl;
		switch (infl.value) {
			case lang::influence_type::shaper:
				set_influence(spec.shaper, infl.origin);
				break;
			case lang::influence_type::elder:
				set_influence(spec.elder, infl.origin);
				break;
			case lang::influence_type::crusader:
				set_influence(spec.crusader, infl.origin);
				break;
			case lang::influence_type::redeemer:
				set_influence(spec.redeemer, infl.origin);
				break;
			case lang::influence_type::hunter:
				set_influence(spec.hunter, infl.origin);
				break;
			case lang::influence_type::warlord:
				set_influence(spec.warlord, infl.origin);
				break;
		}

		if (error)
			return boost::none;
	}

	return spec;
}

[[nodiscard]] std::shared_ptr<lang::has_influence_condition> // null on failure
make_has_influence_condition(settings st, protocondition pc, diagnostics_store& diagnostics)
{
	if (!check_no_counted_comparison(pc.comparison.integer, diagnostics))
		return nullptr;

	if (pc.comparison.operator_.value != lang::comparison_type::equal
		&& pc.comparison.operator_.value != lang::comparison_type::exact_match) {
		diagnostics.push_error_invalid_operator(
			position_tag_of(pc.comparison.operator_),
			"HasInfluence condition supports only "
			"nothing/= for matching one of specified influences "
			"and == for matching all of specified influences");
		return nullptr;
	}

	auto maybe_spec = make_influence_spec(st, pc.obj, diagnostics);
	if (!maybe_spec)
		return nullptr;

	const bool exact_match = pc.comparison.operator_.value == lang::comparison_type::exact_match;
	return lang::make_has_influence_condition(*maybe_spec, exact_match, pc.condition_origin);
}

template <typename T>
[[nodiscard]] boost::optional<lang::range_bound<T>>
make_range_bound(
	settings st,
	const lang::object& obj,
	bool is_less,
	bool allows_equality,
	diagnostics_store& diagnostics)
{
	boost::optional<lang::range_bound<T>> result;

	for (const lang::single_object& sobj : obj.values) {
		const auto val = get_as<T>(sobj, diagnostics);
		if (!val) {
			if (st.error_handling.stop_on_error)
				return boost::none;
			else
				continue;
		}

		if (result) {
			if ((is_less && (*result).value.value < (*val).value)
				|| (!is_less && (*result).value.value > (*val).value))
			{
				result = lang::range_bound<T>{*val, allows_equality};
			}
		}
		else {
			result = lang::range_bound<T>{*val, allows_equality};
		}
	}

	return result;
}

template <typename T>
[[nodiscard]] boost::optional<lang::condition_values_container<T>>
make_condition_values_container(settings /* st */, const lang::object& obj, diagnostics_store& diagnostics)
{
	lang::condition_values_container<T> values;
	diagnostics_store diagnostics_none;

	for (const lang::single_object& sobj : obj.values) {
		// Skip "None"s - they may come from variables.
		if (get_as<lang::none_type>(sobj, diagnostics_none))
			continue;

		auto val = get_as<T>(sobj, diagnostics);

		// If an object can not be T, fail immediately - this might force
		// the caller to attempt different "rule overload". Returning valid
		// object with errors could result in suboptimal overload choices.
		if (val)
			values.push_back(std::move(*val));
		else
			return boost::none;
	}

	return values;
}

template <typename T>
[[nodiscard]] std::shared_ptr<lang::range_or_list_condition> // null on failure
make_range_or_list_condition(
	settings st,
	protocondition pc,
	range_bound_condition_factory_func<T>& range_bound_func,
	value_list_condition_factory_func<T>& value_list_func,
	diagnostics_store& diagnostics)
{
	if (!check_no_counted_comparison(pc.comparison.integer, diagnostics))
		return nullptr;

	using cmp_t = lang::comparison_type;
	const cmp_t& op = pc.comparison.operator_.value;

	switch (op) {
		case cmp_t::less:
		case cmp_t::less_equal:
		case cmp_t::greater:
		case cmp_t::greater_equal: {
			const bool is_less = op == cmp_t::less || op == cmp_t::less_equal;
			const bool allows_equality = op == cmp_t::less_equal || op == cmp_t::greater_equal;
			return make_range_bound<T>(st, pc.obj, is_less, allows_equality, diagnostics)
				.map([&](lang::range_bound<T> bound) {
					return range_bound_func(bound, !is_less, pc.condition_origin);
				})
				.value_or(nullptr);
		}
		case cmp_t::equal:
		case cmp_t::exact_match:
		case cmp_t::not_equal: {
			return make_condition_values_container<T>(st, pc.obj, diagnostics)
				.map([&](lang::condition_values_container<T> values) {
					const bool allowed = op != cmp_t::not_equal;
					return value_list_func(std::move(values), allowed, pc.condition_origin);
				})
				.value_or(nullptr);
		}
	}

	diagnostics.push_error_internal_compiler_error(__func__, pc.condition_origin);
	return nullptr;
}

[[nodiscard]] std::shared_ptr<lang::string_comparison_condition> // null on failure
make_string_comparison_condition(
	settings st,
	protocondition pc,
	string_comparison_condition_factory_func& func,
	diagnostics_store& diagnostics)
{
	if (!check_no_counted_comparison(pc.comparison.integer, diagnostics))
		return nullptr;

	const auto maybe_cmp = lang::to_equality_comparison_type(pc.comparison.operator_.value);
	if (!maybe_cmp) {
		diagnostics.push_error_invalid_operator(
			position_tag_of(pc.comparison.operator_),
			"string comparison conditions support only =, == and !=");
		return nullptr;
	}

	if (!check_object_size(pc.obj, 1, boost::none, diagnostics))
		return nullptr;

	auto maybe_strings = make_condition_values_container<lang::string>(st, pc.obj, diagnostics);
	if (!maybe_strings)
		return nullptr;

	return func(*maybe_cmp, std::move(*maybe_strings), pc.condition_origin);
}

[[nodiscard]] std::shared_ptr<lang::counted_string_comparison_condition> // null on failure
make_counted_string_comparison_condition(
	settings st,
	protocondition pc,
	counted_string_comparison_condition_factory_func& func,
	diagnostics_store& diagnostics)
{
	if (pc.comparison.operator_.value == lang::comparison_type::not_equal) {
		diagnostics.push_error_invalid_operator(
			position_tag_of(pc.comparison.operator_),
			"counted string comparison conditions do not support !=");
		return nullptr;
	}

	std::optional<lang::integer> count;
	if (pc.comparison.integer)
		count = evaluate(*pc.comparison.integer);

	if (!count
		&& pc.comparison.operator_.value != lang::comparison_type::equal
		&& pc.comparison.operator_.value != lang::comparison_type::exact_match)
	{
		diagnostics.push_error_invalid_operator(
			position_tag_of(pc.comparison.operator_),
			"counted string comparison conditions allow only equality comparison (nothing/=/==) if there is no count");
		return nullptr;
	}

	if (!check_object_size(pc.obj, 1, boost::none, diagnostics))
		return nullptr;

	auto maybe_strings = make_condition_values_container<lang::string>(st, pc.obj, diagnostics);
	if (!maybe_strings)
		return nullptr;

	return func(pc.comparison.operator_.value, count, std::move(*maybe_strings), pc.condition_origin);
}

[[nodiscard]] std::shared_ptr<lang::socket_specification_condition> // null on failure
make_socket_specification_condition(
	settings st,
	protocondition pc,
	socket_specification_condition_factory_func& func,
	diagnostics_store& diagnostics)
{
	if (!check_no_counted_comparison(pc.comparison.integer, diagnostics))
		return nullptr;

	if (!check_object_size(pc.obj, 1, boost::none, diagnostics))
		return nullptr;

	auto maybe_specs = make_condition_values_container<lang::socket_spec>(st, pc.obj, diagnostics);
	if (!maybe_specs)
		return nullptr;

	return func(pc.comparison.operator_.value, std::move(*maybe_specs), pc.condition_origin);
}

[[nodiscard]] std::shared_ptr<lang::official_condition> // null on failure
make_official_condition(
	settings st,
	lang::official_condition_property property,
	protocondition pc,
	diagnostics_store& diagnostics)
{
	using property_t = lang::official_condition_property;

	switch (property) {
		// boolean conditions
		case property_t::identified:
			return make_boolean_condition(std::move(pc), lang::make_identified_condition, diagnostics);
		case property_t::corrupted:
			return make_boolean_condition(std::move(pc), lang::make_corrupted_condition, diagnostics);
		case property_t::mirrored:
			return make_boolean_condition(std::move(pc), lang::make_mirrored_condition, diagnostics);
		case property_t::elder_item:
			return make_boolean_condition(std::move(pc), lang::make_elder_item_condition, diagnostics);
		case property_t::shaper_item:
			return make_boolean_condition(std::move(pc), lang::make_shaper_item_condition, diagnostics);
		case property_t::fractured_item:
			return make_boolean_condition(std::move(pc), lang::make_fractured_item_condition, diagnostics);
		case property_t::synthesised_item:
			return make_boolean_condition(std::move(pc), lang::make_synthesised_item_condition, diagnostics);
		case property_t::any_enchantment:
			return make_boolean_condition(std::move(pc), lang::make_any_enchantment_condition, diagnostics);
		case property_t::shaped_map:
			return make_boolean_condition(std::move(pc), lang::make_shaped_map_condition, diagnostics);
		case property_t::elder_map:
			return make_boolean_condition(std::move(pc), lang::make_elder_map_condition, diagnostics);
		case property_t::blighted_map:
			return make_boolean_condition(std::move(pc), lang::make_blighted_map_condition, diagnostics);
		case property_t::replica:
			return make_boolean_condition(std::move(pc), lang::make_replica_condition, diagnostics);
		case property_t::scourged:
			return make_boolean_condition(std::move(pc), lang::make_scourged_condition, diagnostics);
		case property_t::uber_blighted_map:
			return make_boolean_condition(std::move(pc), lang::make_uber_blighted_map_condition, diagnostics);
		case property_t::has_implicit_mod:
			return make_boolean_condition(std::move(pc), lang::make_has_implicit_mod_condition, diagnostics);
		case property_t::has_crucible_passive_tree:
			return make_boolean_condition(std::move(pc), lang::make_has_crucible_passive_tree_condition, diagnostics);
		case property_t::alternate_quality:
			diagnostics.push_warning_dead_condition(pc.condition_origin, "no item can match it (use TransfiguredGem for gems)");
			return make_boolean_condition(std::move(pc), lang::make_alternate_quality_condition, diagnostics);
		case property_t::zana_memory:
			return make_boolean_condition(std::move(pc), lang::make_zana_memory_condition, diagnostics);

		// comparison with influence
		case property_t::has_influence:
			return make_has_influence_condition(st, std::move(pc), diagnostics);

		// comparison with rarities
		case property_t::rarity:
			return make_range_or_list_condition(
				st,
				std::move(pc),
				lang::make_rarity_range_bound_condition,
				lang::make_rarity_value_list_condition,
				diagnostics);
		// comparison with integers
		case property_t::item_level:
			return make_range_or_list_condition(
				st,
				std::move(pc),
				lang::make_item_level_range_bound_condition,
				lang::make_item_level_value_list_condition,
				diagnostics);
		case property_t::drop_level:
			return make_range_or_list_condition(
				st,
				std::move(pc),
				lang::make_drop_level_range_bound_condition,
				lang::make_drop_level_value_list_condition,
				diagnostics);
		case property_t::quality:
			return make_range_or_list_condition(
				st,
				std::move(pc),
				lang::make_quality_range_bound_condition,
				lang::make_quality_value_list_condition,
				diagnostics);
		case property_t::linked_sockets:
			return make_range_or_list_condition(
				st,
				std::move(pc),
				lang::make_linked_sockets_range_bound_condition,
				lang::make_linked_sockets_value_list_condition,
				diagnostics);
		case property_t::height:
			return make_range_or_list_condition(
				st,
				std::move(pc),
				lang::make_height_range_bound_condition,
				lang::make_height_value_list_condition,
				diagnostics);
		case property_t::width:
			return make_range_or_list_condition(
				st,
				std::move(pc),
				lang::make_width_range_bound_condition,
				lang::make_width_value_list_condition,
				diagnostics);
		case property_t::stack_size:
			return make_range_or_list_condition(
				st,
				std::move(pc),
				lang::make_stack_size_range_bound_condition,
				lang::make_stack_size_value_list_condition,
				diagnostics);
		case property_t::gem_level:
			return make_range_or_list_condition(
				st,
				std::move(pc),
				lang::make_gem_level_range_bound_condition,
				lang::make_gem_level_value_list_condition,
				diagnostics);
		case property_t::map_tier:
			return make_range_or_list_condition(
				st,
				std::move(pc),
				lang::make_map_tier_range_bound_condition,
				lang::make_map_tier_value_list_condition,
				diagnostics);
		case property_t::area_level:
			return make_range_or_list_condition(
				st,
				std::move(pc),
				lang::make_area_level_range_bound_condition,
				lang::make_area_level_value_list_condition,
				diagnostics);
		case property_t::corrupted_mods:
			return make_range_or_list_condition(
				st,
				std::move(pc),
				lang::make_corrupted_mods_range_bound_condition,
				lang::make_corrupted_mods_value_list_condition,
				diagnostics);
		case property_t::enchantment_passive_num:
			return make_range_or_list_condition(
				st,
				std::move(pc),
				lang::make_enchantment_passive_num_range_bound_condition,
				lang::make_enchantment_passive_num_value_list_condition,
				diagnostics);
		case property_t::base_defence_percentile:
			return make_range_or_list_condition(
				st,
				std::move(pc),
				lang::make_base_defence_percentile_range_bound_condition,
				lang::make_base_defence_percentile_value_list_condition,
				diagnostics);
		case property_t::base_armour:
			return make_range_or_list_condition(
				st,
				std::move(pc),
				lang::make_base_armour_range_bound_condition,
				lang::make_base_armour_value_list_condition,
				diagnostics);
		case property_t::base_evasion:
			return make_range_or_list_condition(
				st,
				std::move(pc),
				lang::make_base_evasion_range_bound_condition,
				lang::make_base_evasion_value_list_condition,
				diagnostics);
		case property_t::base_energy_shield:
			return make_range_or_list_condition(
				st,
				std::move(pc),
				lang::make_base_energy_shield_range_bound_condition,
				lang::make_base_energy_shield_value_list_condition,
				diagnostics);
		case property_t::base_ward:
			return make_range_or_list_condition(
				st,
				std::move(pc),
				lang::make_base_ward_range_bound_condition,
				lang::make_base_ward_value_list_condition,
				diagnostics);
		case property_t::has_searing_exarch_implicit:
			return make_range_or_list_condition(
				st,
				std::move(pc),
				lang::make_has_searing_exarch_implicit_range_bound_condition,
				lang::make_has_searing_exarch_implicit_value_list_condition,
				diagnostics);
		case property_t::has_eater_of_worlds_implicit:
			return make_range_or_list_condition(
				st,
				std::move(pc),
				lang::make_has_eater_of_worlds_implicit_range_bound_condition,
				lang::make_has_eater_of_worlds_implicit_value_list_condition,
				diagnostics);
		case property_t::memory_strands:
			return make_range_or_list_condition(
				st,
				std::move(pc),
				lang::make_memory_strands_range_bound_condition,
				lang::make_memory_strands_value_list_condition,
				diagnostics);

		// comparison with an array of strings
		case property_t::class_:
			return make_string_comparison_condition(st, std::move(pc), lang::make_class_condition, diagnostics);
		case property_t::base_type:
			return make_string_comparison_condition(st, std::move(pc), lang::make_base_type_condition, diagnostics);
		case property_t::enchantment_passive_node:
			return make_string_comparison_condition(st, std::move(pc), lang::make_enchantment_passive_node_condition, diagnostics);
		case property_t::archnemesis_mod:
			diagnostics.push_warning_dead_condition(pc.condition_origin, "Archnemesis items no longer exist");
			return make_string_comparison_condition(st, std::move(pc), lang::make_archnemesis_mod_condition, diagnostics);

		// comparison (+ optional integer) with an array of strings
		case property_t::has_explicit_mod:
			return make_counted_string_comparison_condition(st, std::move(pc), lang::make_has_explicit_mod_condition, diagnostics);
		case property_t::has_enchantment:
			return make_counted_string_comparison_condition(st, std::move(pc), lang::make_has_enchantment_condition, diagnostics);

		// socket conditions
		case property_t::sockets:
			return make_socket_specification_condition(st, std::move(pc), lang::make_sockets_condition, diagnostics);
		case property_t::socket_group:
			return make_socket_specification_condition(st, std::move(pc), lang::make_socket_group_condition, diagnostics);

		// special case - TransfiguredGem
		// it supports both a boolean test (e.g. TransfiguredGem True)
		// but also a string test (e.g. TransfiguredGem "Heavy Strike of Trarthus")
		case property_t::transfigured_gem: {
			diagnostics_store diagnostics_boolean_version;
			auto boolean_version_result = make_boolean_condition(
				pc, lang::make_transfigured_gem_condition_boolean_version, diagnostics_boolean_version);

			if (boolean_version_result) {
				diagnostics.move_messages_from(diagnostics_boolean_version);
				return boolean_version_result;
			}

			diagnostics_store diagnostics_string_version;
			auto string_version_result = make_string_comparison_condition(
				st, pc, lang::make_transfigured_gem_condition_string_version, diagnostics_string_version);

			if (string_version_result) {
				diagnostics.move_messages_from(diagnostics_string_version);
				return string_version_result;
			}

			// main error
			diagnostics.push_message(make_error(
				diagnostic_message_id::invalid_expression, pc.condition_origin, "invalid ", lang::keywords::rf::transfigured_gem));
			// errors specifying each variant problems
			diagnostics.push_message(make_note_attempt_description("in attempt of TransfiguredGem Boolean"));
			diagnostics.move_messages_from(diagnostics_boolean_version);
			diagnostics.push_message(make_note_attempt_description("in attempt of TransfiguredGem [EQ] String+"));
			diagnostics.move_messages_from(diagnostics_string_version);
			return nullptr;
		}
	}

	diagnostics.push_error_internal_compiler_error(__func__, pc.condition_origin);
	return nullptr;
}

// Only for spirit filter templates to block undesirable filter structures.
// Not intended for real filters as they allow pretty much unconstrained duplicates.
[[nodiscard]] bool
spirit_filter_is_condition_allowed(
	const parser::ast::sf::official_condition& condition,
	const lang::official_conditions& existing_conditions,
	diagnostics_store& diagnostics)
{
	using property_t = lang::official_condition_property;

	if (condition.property == property_t::has_explicit_mod
		|| condition.property == property_t::has_enchantment
		|| condition.property == property_t::sockets
		|| condition.property == property_t::socket_group)
	{
		// duplicates of these should always be allowed
		return true;
	}

	const lang::official_conditions::search_results results =
		existing_conditions.search(condition.property);

	switch (condition.property) {
		// boolean - only 1 is allowed
		case property_t::identified:
		case property_t::corrupted:
		case property_t::mirrored:
		case property_t::elder_item:
		case property_t::shaper_item:
		case property_t::fractured_item:
		case property_t::synthesised_item:
		case property_t::any_enchantment:
		case property_t::shaped_map:
		case property_t::elder_map:
		case property_t::blighted_map:
		case property_t::replica:
		case property_t::scourged:
		case property_t::uber_blighted_map:
		case property_t::has_implicit_mod:
		case property_t::has_crucible_passive_tree:
		case property_t::alternate_quality:
		case property_t::zana_memory:
		// influence - only 1 is allowed
		case property_t::has_influence: {
			FS_ASSERT_MSG(
				!results.lower_bound && !results.upper_bound,
				"boolean conditions should never appear with a bound");

			if (results.other) {
				diagnostics.push_error_condition_redefinition(position_tag_of(condition), *results.other);
				return false;
			}

			return true;
		}
		// range - duplicates allowed only if they have distinct bounds
		case property_t::rarity:
		case property_t::item_level:
		case property_t::drop_level:
		case property_t::quality:
		case property_t::linked_sockets:
		case property_t::height:
		case property_t::width:
		case property_t::stack_size:
		case property_t::gem_level:
		case property_t::map_tier:
		case property_t::area_level:
		case property_t::corrupted_mods:
		case property_t::enchantment_passive_num:
		case property_t::base_defence_percentile:
		case property_t::base_armour:
		case property_t::base_evasion:
		case property_t::base_energy_shield:
		case property_t::base_ward:
		case property_t::has_searing_exarch_implicit:
		case property_t::has_eater_of_worlds_implicit:
		case property_t::memory_strands:
		{
			switch (condition.comparison.operator_.value) {
				case lang::comparison_type::less:
				case lang::comparison_type::less_equal:
					if (results.upper_bound) {
						diagnostics.push_error_upper_bound_redefinition(position_tag_of(condition), *results.upper_bound);
						return false;
					}

					if (results.values_equal) {
						diagnostics.push_error_condition_after_equality(position_tag_of(condition), *results.values_equal);
						return false;
					}

					return true;
				case lang::comparison_type::greater:
				case lang::comparison_type::greater_equal:
					if (results.lower_bound) {
						diagnostics.push_error_lower_bound_redefinition(position_tag_of(condition), *results.lower_bound);
						return false;
					}

					if (results.values_equal) {
						diagnostics.push_error_condition_after_equality(position_tag_of(condition), *results.values_equal);
						return false;
					}

					return true;
				case lang::comparison_type::equal:
				case lang::comparison_type::exact_match:
					if (results.values_equal) {
						diagnostics.push_error_condition_after_equality(position_tag_of(condition), *results.values_equal);
						return false;
					}

					return true;
				case lang::comparison_type::not_equal:
					// This one is actually fine because it increases
					// specificity in a unique way and thus does not conflict.
					return true;
			}

			diagnostics.push_error_internal_compiler_error(__func__, position_tag_of(condition));
			return false;
		}
		// string - only 1 is allowed
		case property_t::class_:
		case property_t::base_type:
		case property_t::enchantment_passive_node:
		case property_t::archnemesis_mod: {
			FS_ASSERT_MSG(
				!results.lower_bound && !results.upper_bound,
				"string conditions should never appear with a bound");

			if (results.other) {
				diagnostics.push_error_condition_redefinition(position_tag_of(condition), *results.other);
				return false;
			}

			return true;
		}
		// These are always allowed. Checked at the top to avoid searching.
		// Repeated here to silence non-exhaustive switch warning.
		case property_t::has_explicit_mod:
		case property_t::has_enchantment:
		case property_t::sockets:
		case property_t::socket_group:
		// This is always allowed because it has multiple variants
		case property_t::transfigured_gem:
			return true;
	}

	diagnostics.push_error_internal_compiler_error(__func__, position_tag_of(condition));
	return false;
}

[[nodiscard]] bool
spirit_filter_add_official_condition(
	settings st,
	const parser::ast::sf::official_condition& condition,
	const symbol_table& symbols,
	lang::official_conditions& block_conditions,
	diagnostics_store& diagnostics)
{
	if (!spirit_filter_is_condition_allowed(condition, block_conditions, diagnostics))
		return false;

	auto maybe_obj = evaluate_sequence(st, condition.seq, symbols, diagnostics);
	if (!maybe_obj)
		return false;

	std::shared_ptr<lang::official_condition> new_condition = make_official_condition(
		st,
		condition.property,
		protocondition{condition.comparison, std::move(*maybe_obj), position_tag_of(condition)},
		diagnostics);

	if (!new_condition)
		return false;

	block_conditions.conditions.push_back(std::move(new_condition));
	return true;
}

[[nodiscard]] bool
spirit_filter_add_autogen_condition(
	settings st,
	const parser::ast::sf::autogen_condition& condition,
	const symbol_table& symbols,
	std::optional<autogen_protocondition>& autogen,
	diagnostics_store& diagnostics)
{
	const auto maybe_obj = evaluate_sequence(st, condition.seq, symbols, diagnostics);
	if (!maybe_obj)
		return false;

	const lang::object& obj = *maybe_obj;

	if (!check_object_size(obj, 1, 1, diagnostics))
		return false;

	FS_ASSERT(obj.values.size() == 1u);

	const auto maybe_str = get_as<lang::string>(obj.values.front(), diagnostics);
	if (!maybe_str)
		return false;

	const lang::string& autogen_name = *maybe_str;

	const auto category = lang::autogen_category::_from_string_nothrow(autogen_name.value.c_str());
	if (!category) {
		diagnostics.push_error_invalid_expression(autogen_name.origin, "invalid autogeneration name");
		return false;
	}

	if (autogen.has_value()) {
		diagnostics.push_error_condition_redefinition(position_tag_of(condition), (*autogen).origin);
		return false;
	}

	autogen = autogen_protocondition{*category, position_tag_of(condition)};
	return true;
}

[[nodiscard]] bool
spirit_filter_add_price_comparison_condition(
	settings st,
	const parser::ast::sf::price_comparison_condition& condition,
	const symbol_table& symbols,
	lang::price_range_condition& price_condition,
	diagnostics_store& diagnostics)
{
	if (!check_no_counted_comparison(condition.comparison.integer, diagnostics))
		return false;

	const auto maybe_obj = evaluate_sequence(st, condition.seq, symbols, diagnostics);
	if (!maybe_obj)
		return false;

	using cmp_t = lang::comparison_type;
	const cmp_t& op = condition.comparison.operator_.value;
	const lang::object& obj = *maybe_obj;
	const lang::position_tag condition_origin = position_tag_of(condition);

	switch (op) {
		case cmp_t::less:
		case cmp_t::less_equal:
		case cmp_t::greater:
		case cmp_t::greater_equal: {
			const bool is_less = op == cmp_t::less || op == cmp_t::less_equal;
			const bool allows_equality = op == cmp_t::less_equal || op == cmp_t::greater_equal;
			const auto maybe_bound = make_range_bound<lang::fractional>(
				st, obj, is_less, allows_equality, diagnostics);

			if (!maybe_bound)
				return false;

			if (is_less) {
				if (auto upper_bound = price_condition.upper_bound(); upper_bound.has_value()) {
					diagnostics.push_error_upper_bound_redefinition(condition_origin, (*upper_bound).origin);
					return false;
				}

				price_condition.set_upper_bound(*maybe_bound, condition_origin);
				return true;
			}
			else {
				if (auto lower_bound = price_condition.lower_bound(); lower_bound.has_value()) {
					diagnostics.push_error_lower_bound_redefinition(condition_origin, (*lower_bound).origin);
					return false;
				}

				price_condition.set_lower_bound(*maybe_bound, condition_origin);
				return true;
			}
		}
		case cmp_t::equal:
		case cmp_t::exact_match: {
			const auto maybe_values = make_condition_values_container<lang::fractional>(st, obj, diagnostics);

			if (!maybe_values)
				return false;

			const auto& values = *maybe_values;
			if (values.size() != 1u) {
				diagnostics.push_error_invalid_amount_of_arguments(1, 1, static_cast<int>(values.size()), obj.origin);
				return false;
			}

			if (auto upper_bound = price_condition.upper_bound(); upper_bound.has_value()) {
				diagnostics.push_error_upper_bound_redefinition(condition_origin, (*upper_bound).origin);
				return false;
			}

			if (auto lower_bound = price_condition.lower_bound(); lower_bound.has_value()) {
				diagnostics.push_error_lower_bound_redefinition(condition_origin, (*lower_bound).origin);
				return false;
			}

			price_condition.set_exact(values.front(), condition_origin);
			return true;
		}
		case cmp_t::not_equal: {
			diagnostics.push_error_invalid_operator(
				position_tag_of(condition.comparison.operator_),
				"Price condition does not support inequality (!/!=)");
			return false;
		}
	}

	diagnostics.push_error_internal_compiler_error(__func__, condition_origin);
	return false;
}

} // namespace

bool
real_filter_add_condition(
	settings st,
	const parser::ast::rf::condition& condition,
	lang::official_conditions& block_conditions,
	diagnostics_store& diagnostics)
{
	auto maybe_obj = evaluate_literal_sequence(st, condition.seq, diagnostics);
	if (!maybe_obj)
		return false;

	std::shared_ptr<lang::official_condition> new_condition = make_official_condition(
		st,
		condition.property,
		protocondition{condition.comparison, std::move(*maybe_obj), position_tag_of(condition)},
		diagnostics);

	if (!new_condition)
		return false;

	block_conditions.conditions.push_back(std::move(new_condition));
	return true;
}

bool
spirit_filter_add_condition(
	settings st,
	const parser::ast::sf::condition& condition,
	const symbol_table& symbols,
	spirit_protoconditions& block_conditions,
	diagnostics_store& diagnostics)
{
	return condition.apply_visitor(make_lambda_visitor<bool>(
		[&](const parser::ast::sf::official_condition& cond) {
			return spirit_filter_add_official_condition(st, cond, symbols, block_conditions.official, diagnostics);
		},
		[&](const parser::ast::sf::autogen_condition& cond) {
			return spirit_filter_add_autogen_condition(st, cond, symbols, block_conditions.autogen, diagnostics);
		},
		[&](const parser::ast::sf::price_comparison_condition& cond) {
			return spirit_filter_add_price_comparison_condition(st, cond, symbols, block_conditions.price_range, diagnostics);
		}
	));
}

}
