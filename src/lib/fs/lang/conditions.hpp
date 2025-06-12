#pragma once

#include <fs/lang/enum_types.hpp>
#include <fs/lang/primitive_types.hpp>
#include <fs/lang/position_tag.hpp>
#include <fs/lang/item.hpp>
#include <fs/utility/type_traits.hpp>

#include <boost/container/small_vector.hpp>

#include <algorithm>
#include <iosfwd>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace fs::lang {

class condition_match_result
{
public:
	condition_match_result(
		bool is_successful,
		lang::position_tag condition_origin,
		std::optional<lang::position_tag> value_origin = std::nullopt)
	: m_is_successful(is_successful)
	, m_condition_origin(condition_origin)
	, m_value_origin(value_origin)
	{}

	static condition_match_result success(
		lang::position_tag condition_origin,
		std::optional<lang::position_tag> value_origin = std::nullopt)
	{
		return condition_match_result(true, condition_origin, value_origin);
	}

	static condition_match_result failure(
		lang::position_tag condition_origin,
		std::optional<lang::position_tag> value_origin = std::nullopt)
	{
		return condition_match_result(false, condition_origin, value_origin);
	}

	bool is_successful() const { return m_is_successful; }
	lang::position_tag condition_origin() const { return m_condition_origin; }
	std::optional<lang::position_tag> value_origin() const { return m_value_origin; }

private:
	bool m_is_successful;
	lang::position_tag m_condition_origin;
	std::optional<lang::position_tag> m_value_origin;
};

// base type for all conditions
// (currently nothing specific to add here)
class condition
{
public:
	virtual ~condition() = default;
};

// base type for all real-filter conditions
// both RF and SF might generate them
class official_condition : public condition
{
public:
	enum class test_type {
		lower_bound, upper_bound, values_equal, values_unequal, sockets, boolean_state
	};

	official_condition(official_condition_property tested_property, test_type type, position_tag origin)
	: m_tested_property(tested_property)
	, m_type(type)
	, m_origin(origin)
	{}

	position_tag origin() const { return m_origin; }

	// These 2 functions are mostly for filter templates.
	// Template compiler can use these to block duplicate or conflicting conditions.
	official_condition_property tested_property() const { return m_tested_property; }
	test_type type() const { return m_type; }

	// For autogeneration conditions. They need to check for conflicts.
	virtual bool allows_item_class(std::string_view /* class_name */) const { return true; }
	virtual bool allows_item_rarity(rarity_type /* r */) const { return true; }

	// For loot generation and filter debug.
	virtual condition_match_result test_item(const item& itm, int area_level) const = 0;

	// Some conditions may have valid state but would not be accepted by the game client.
	// Examples: invalid operator, empty list of values. Such conditons should not be printed.
	virtual bool is_valid() const = 0;
	virtual void print(std::ostream& os) const = 0;

private:
	official_condition_property m_tested_property;
	test_type m_type;
	position_tag m_origin;
};

// majority of conditions that support multiple values hold just 1
template <typename T>
using condition_values_container = boost::container::small_vector<T, 1>;

// ---- boolean ----

class boolean_condition : public official_condition
{
public:
	boolean_condition(official_condition_property tested_property, boolean value, position_tag origin)
	: official_condition(tested_property, test_type::boolean_state, origin)
	, m_value(value)
	{}

	bool is_valid() const final { return true; }

	void print(std::ostream& os) const final;

protected:
	boolean value() const { return m_value; }

private:
	boolean m_value;
};

// a boolean_condition which's test_item implementation directly tests a bool field in the item struct
class boolean_condition_with_field_test : public boolean_condition
{
public:
	boolean_condition_with_field_test(official_condition_property tested_property, bool item::* tested_field, boolean value, position_tag origin)
	: boolean_condition(tested_property, value, origin)
	, m_tested_field(tested_field)
	{}

	condition_match_result test_item(const item& itm, int /* area_level */) const final
	{
		const boolean val = value();
		return condition_match_result(itm.*m_tested_field == val.value, origin(), val.origin);
	}

private:
	bool item::* m_tested_field;
};

inline std::shared_ptr<boolean_condition> make_identified_condition(boolean value, position_tag origin)
{
	return std::make_shared<boolean_condition_with_field_test>(official_condition_property::identified, &item::is_identified, value, origin);
}

inline std::shared_ptr<boolean_condition> make_mirrored_condition(boolean value, position_tag origin)
{
	return std::make_shared<boolean_condition_with_field_test>(official_condition_property::mirrored, &item::is_mirrored, value, origin);
}

inline std::shared_ptr<boolean_condition> make_fractured_item_condition(boolean value, position_tag origin)
{
	return std::make_shared<boolean_condition_with_field_test>(official_condition_property::fractured_item, &item::is_fractured, value, origin);
}

inline std::shared_ptr<boolean_condition> make_synthesised_item_condition(boolean value, position_tag origin)
{
	return std::make_shared<boolean_condition_with_field_test>(official_condition_property::synthesised_item, &item::is_synthesised, value, origin);
}

inline std::shared_ptr<boolean_condition> make_shaped_map_condition(boolean value, position_tag origin)
{
	return std::make_shared<boolean_condition_with_field_test>(official_condition_property::shaped_map, &item::is_shaped_map, value, origin);
}

inline std::shared_ptr<boolean_condition> make_elder_map_condition(boolean value, position_tag origin)
{
	return std::make_shared<boolean_condition_with_field_test>(official_condition_property::elder_map, &item::is_elder_map, value, origin);
}

inline std::shared_ptr<boolean_condition> make_replica_condition(boolean value, position_tag origin)
{
	return std::make_shared<boolean_condition_with_field_test>(official_condition_property::replica, &item::is_replica, value, origin);
}

inline std::shared_ptr<boolean_condition> make_has_crucible_passive_tree_condition(boolean value, position_tag origin)
{
	return std::make_shared<boolean_condition_with_field_test>(official_condition_property::has_crucible_passive_tree, &item::has_crucible_passive_tree, value, origin);
}

inline std::shared_ptr<boolean_condition> make_transfigured_gem_condition_boolean_version(boolean value, position_tag origin)
{
	return std::make_shared<boolean_condition_with_field_test>(official_condition_property::transfigured_gem, &item::is_transfigured_gem, value, origin);
}

inline std::shared_ptr<boolean_condition> make_zana_memory_condition(boolean value, position_tag origin)
{
	return std::make_shared<boolean_condition_with_field_test>(official_condition_property::zana_memory, &item::zana_memory, value, origin);
}

// a boolean_condition which's test_item implementation calls a boolean function in the item struct
class boolean_condition_with_function_test : public boolean_condition
{
public:
	using test_function_type = bool (item::*)() const;

	boolean_condition_with_function_test(official_condition_property tested_property, test_function_type test_func, boolean value, position_tag origin)
	: boolean_condition(tested_property, value, origin)
	, m_test_func(test_func)
	{}

	condition_match_result test_item(const item& itm, int /* area_level */) const final
	{
		const boolean val = value();
		return condition_match_result((itm.*m_test_func)() == val.value, origin(), val.origin);
	}

private:
	test_function_type m_test_func;
};

inline std::shared_ptr<boolean_condition> make_has_implicit_mod_condition(boolean value, position_tag origin)
{
	return std::make_shared<boolean_condition_with_function_test>(official_condition_property::has_implicit_mod, &item::has_implicit_mod, value, origin);
}

inline std::shared_ptr<boolean_condition> make_corrupted_condition(boolean value, position_tag origin)
{
	return std::make_shared<boolean_condition_with_function_test>(official_condition_property::corrupted, &item::is_corrupted, value, origin);
}

inline std::shared_ptr<boolean_condition> make_scourged_condition(boolean value, position_tag origin)
{
	return std::make_shared<boolean_condition_with_function_test>(official_condition_property::scourged, &item::is_scourged, value, origin);
}

inline std::shared_ptr<boolean_condition> make_blighted_map_condition(boolean value, position_tag origin)
{
	return std::make_shared<boolean_condition_with_function_test>(official_condition_property::blighted_map, &item::is_blighted_map, value, origin);
}

inline std::shared_ptr<boolean_condition> make_uber_blighted_map_condition(boolean value, position_tag origin)
{
	return std::make_shared<boolean_condition_with_function_test>(official_condition_property::uber_blighted_map, &item::is_uber_blighted_map, value, origin);
}

inline std::shared_ptr<boolean_condition> make_any_enchantment_condition(boolean value, position_tag origin)
{
	return std::make_shared<boolean_condition_with_function_test>(official_condition_property::any_enchantment, &item::has_enchantment, value, origin);
}

inline std::shared_ptr<boolean_condition> make_elder_item_condition(boolean value, position_tag origin)
{
	return std::make_shared<boolean_condition_with_function_test>(official_condition_property::elder_item, &item::is_elder_item, value, origin);
}

inline std::shared_ptr<boolean_condition> make_shaper_item_condition(boolean value, position_tag origin)
{
	return std::make_shared<boolean_condition_with_function_test>(official_condition_property::shaper_item, &item::is_shaper_item, value, origin);
}

class alternate_quality_condition : public boolean_condition
{
public:
	alternate_quality_condition(boolean value, position_tag origin)
	: boolean_condition(official_condition_property::alternate_quality, value, origin)
	{}

	condition_match_result test_item(const item& /* itm */, int /* area_level */) const final
	{
		// dead condition, no item can satisfy it
		return condition_match_result::failure(origin());
	}
};

inline std::shared_ptr<boolean_condition> make_alternate_quality_condition(boolean value, position_tag origin)
{
	return std::make_shared<alternate_quality_condition>(value, origin);
}

// ---- has influence ----

class has_influence_condition : public official_condition
{
public:
	has_influence_condition(influence_spec spec, bool exact_match, position_tag origin)
	: official_condition(official_condition_property::has_influence, test_type::values_equal, origin)
	, m_exact_match(exact_match)
	, m_influence_spec(spec)
	{}

	condition_match_result test_item(const item& itm, int area_level) const final;

	bool is_valid() const final { return true; }

	void print(std::ostream& os) const final;

private:
	bool m_exact_match;
	influence_spec m_influence_spec;
};

inline auto make_has_influence_condition(influence_spec spec, bool exact_match, position_tag origin)
{
	return std::make_shared<has_influence_condition>(spec, exact_match, origin);
}

// ---- range or list ----

/**
 * In order to correctly implement Rarity, ItemLevel, MapTier, GemLevel and
 * similar conditions, there must be a support for 2 different usages:
 * - less/more comparison with 1 value - e.g. Rarity < Unique, ItemLevel >= 60
 * - equality comparison with (potentially multiple) values: e.g. Rarity Normal Rare
 *
 * The less/more variant (range_condition) stores only min/max bound because
 * any such condition can be simplified to the most boundary value:
 * ItemLevel < 20 80 40 86
 * can be simplified to
 * ItemLevel < 86
 * because all items that satisfy others will satisfy < 86 aswell.
 */
class range_or_list_condition : public official_condition
{
public:
	using official_condition::official_condition;
};

class range_bound_condition_base : public range_or_list_condition
{
public:
	using range_or_list_condition::range_or_list_condition;

	bool is_valid() const final { return true; }

protected:
	// template-less overloads to avoid dragging ostream and other dependencies
	// (add more overloads if new type instantiations are needed)
	void print_impl(comparison_type comparison, rarity bound_value, std::ostream& os) const;
	void print_impl(comparison_type comparison, integer bound_value, std::ostream& os) const;
};

class value_list_condition_base : public range_or_list_condition
{
public:
	using range_or_list_condition::range_or_list_condition;

protected:
	// template-less overloads to avoid dragging ostream and other dependencies
	// (add more overloads if new type instantiations are needed)
	void print_impl(bool allowed, const condition_values_container<rarity>& values, std::ostream& os) const;
	void print_impl(bool allowed, const condition_values_container<integer>& values, std::ostream& os) const;
};

template <typename T>
struct range_bound
{
	// used in the implementation
	// T should be a lang type
	static_assert(traits::has_origin_member_v<T>);
	static_assert(traits::has_value_member_v<T>);

	T value;
	bool inclusive;
};

template <typename T> constexpr
bool operator==(range_bound<T> left, range_bound<T> right) noexcept
{
	return left.value == right.value && left.inclusive == right.inclusive;
}

template <typename T> constexpr
bool operator!=(range_bound<T> left, range_bound<T> right) noexcept
{
	return !(left == right);
}

template <typename T>
class range_bound_condition : public range_bound_condition_base
{
public:
	using value_type = decltype(T::value);

	range_bound_condition(
		official_condition_property tested_property,
		range_bound<T> bound,
		bool is_lower_bound,
		position_tag origin)
	: range_bound_condition_base(tested_property, is_lower_bound ? test_type::lower_bound : test_type::upper_bound, origin)
	, m_bound(bound)
	{}

	bool allows_item_rarity(rarity_type r) const final
	{
		if constexpr (std::is_same_v<value_type, rarity_type>)
			return test_property_value(r);
		else
			return true;
	}

	condition_match_result test_item(const item& itm, int area_level) const final
	{
		const value_type property_value = get_tested_property_value(itm, area_level);
		return condition_match_result(test_property_value(property_value), origin(), m_bound.value.origin);
	}

	void print(std::ostream& os) const final
	{
		const auto cmp_type = is_lower_bound() ?
			  (m_bound.inclusive ? comparison_type::greater_equal : comparison_type::greater)
			: (m_bound.inclusive ? comparison_type::less_equal : comparison_type::less);
		print_impl(cmp_type, m_bound.value, os);
	}

protected:
	virtual value_type get_tested_property_value(const item& itm, int area_level) const = 0;

	bool test_property_value(value_type property_value) const
	{
		return is_lower_bound() ?
			  (m_bound.inclusive ? property_value >= m_bound.value.value : property_value > m_bound.value.value)
			: (m_bound.inclusive ? property_value <= m_bound.value.value : property_value < m_bound.value.value);
	}

private:
	bool is_lower_bound() const { return type() == test_type::lower_bound; }

	range_bound<T> m_bound;
};

template <typename T>
class range_bound_condition_with_field_test : public range_bound_condition<T>
{
public:
	using value_type = typename range_bound_condition<T>::value_type;

	range_bound_condition_with_field_test(
		official_condition_property tested_property,
		value_type item::* tested_field,
		range_bound<T> bound,
		bool is_lower_bound,
		position_tag origin)
	: range_bound_condition<T>(tested_property, bound, is_lower_bound, origin)
	, m_tested_field(tested_field)
	{}

protected:
	value_type get_tested_property_value(const item& itm, int /* area_level */) const final
	{
		return itm.*m_tested_field;
	}

private:
	value_type item::* m_tested_field;
};

class range_bound_condition_with_function_test : public range_bound_condition<integer>
{
public:
	using value_type = typename range_bound_condition<integer>::value_type;
	using test_function_type = value_type (item::*)() const;

	range_bound_condition_with_function_test(
		official_condition_property tested_property,
		test_function_type test_func,
		range_bound<integer> bound,
		bool is_lower_bound,
		position_tag origin)
	: range_bound_condition<integer>(tested_property, bound, is_lower_bound, origin)
	, m_test_func(test_func)
	{}

protected:
	value_type get_tested_property_value(const item& itm, int /* area_level */) const final
	{
		return (itm.*m_test_func)();
	}

private:
	test_function_type m_test_func;
};

class area_level_range_bound_condition : public range_bound_condition<integer>
{
public:
	using value_type = typename range_bound_condition<integer>::value_type;

	area_level_range_bound_condition(
		official_condition_property tested_property,
		range_bound<integer> bound,
		bool is_lower_bound,
		position_tag origin)
	: range_bound_condition<integer>(tested_property, bound, is_lower_bound, origin)
	{}

protected:
	value_type get_tested_property_value(const item& /* itm */, int area_level) const final
	{
		return area_level;
	}
};

template <typename T>
class value_list_condition : public value_list_condition_base
{
private:
	// Small implementation detail to avoid code duplication while
	// implementing test_item override and allows_item_rarity override.
	struct property_test_result
	{
		bool is_successful;
		std::optional<position_tag> value_origin;
	};

public:
	using container_type = condition_values_container<T>;
	using value_type = decltype(T::value);

	value_list_condition(
		official_condition_property tested_property,
		container_type values,
		bool allowed,
		position_tag origin)
	: value_list_condition_base(tested_property, allowed ? test_type::values_equal : test_type::values_unequal, origin)
	, m_values(std::move(values))
	, m_allowed(allowed)
	{}

	bool allows_item_rarity(rarity_type r) const final
	{
		if constexpr (std::is_same_v<value_type, rarity_type>)
			return test_property_value(r).is_successful;
		else
			return true;
	}

	condition_match_result test_item(const item& itm, int area_level) const final
	{
		const property_test_result result = test_property_value(get_tested_property_value(itm, area_level));
		return condition_match_result(result.is_successful, origin(), result.value_origin);
	}

	bool is_valid() const final { return !m_values.empty(); }

	void print(std::ostream& os) const final { print_impl(m_allowed, m_values, os); }

protected:
	virtual value_type get_tested_property_value(const item& itm, int area_level) const = 0;

	property_test_result test_property_value(value_type property_value) const
	{
		const auto it = std::find_if(m_values.begin(), m_values.end(), [&](T value){ return value.value == property_value; });
		const bool is_successful = m_allowed == (it != m_values.end());
		const auto value_origin = (it != m_values.end()) ? it->origin : std::optional<position_tag>();
		return property_test_result{is_successful, value_origin};
	}

private:
	container_type m_values;
	bool m_allowed;
};

template <typename T>
class value_list_condition_with_field_test : public value_list_condition<T>
{
public:
	using container_type = typename value_list_condition<T>::container_type;
	using value_type = typename value_list_condition<T>::value_type;

	value_list_condition_with_field_test(
		official_condition_property tested_property,
		value_type item::* tested_field,
		container_type values,
		bool allowed,
		position_tag origin)
	: value_list_condition<T>(tested_property, std::move(values), allowed, origin)
	, m_tested_field(tested_field)
	{}

protected:
	value_type get_tested_property_value(const item& itm, int /* area_level */) const final
	{
		return itm.*m_tested_field;
	}

private:
	value_type item::* m_tested_field;
};

template <typename T>
class value_list_condition_with_function_test : public value_list_condition<T>
{
public:
	using container_type = typename value_list_condition<T>::container_type;
	using value_type = typename value_list_condition<T>::value_type;
	using test_function_type = value_type (item::*)() const;

	value_list_condition_with_function_test(
		official_condition_property tested_property,
		test_function_type test_func,
		container_type values,
		bool allowed,
		position_tag origin)
	: value_list_condition<T>(tested_property, std::move(values), allowed, origin)
	, m_test_func(test_func)
	{}

protected:
	value_type get_tested_property_value(const item& itm, int /* area_level */) const final
	{
		return (itm.*m_test_func)();
	}

private:
	test_function_type m_test_func;
};

class area_level_value_list_condition : public value_list_condition<integer>
{
public:
	using value_type = typename value_list_condition<integer>::value_type;

	area_level_value_list_condition(
		official_condition_property tested_property,
		container_type values,
		bool allowed,
		position_tag origin)
	: value_list_condition<integer>(tested_property, std::move(values), allowed, origin)
	{}

protected:
	value_type get_tested_property_value(const item& /* itm */, int area_level) const final
	{
		return area_level;
	}
};

// -- field test --

inline std::shared_ptr<range_bound_condition<rarity>> make_rarity_range_bound_condition(
	range_bound<rarity> bound, bool is_lower_bound, position_tag origin)
{
	return std::make_shared<range_bound_condition_with_field_test<rarity>>(
		official_condition_property::rarity, &item::rarity_, bound, is_lower_bound, origin);
}

inline std::shared_ptr<value_list_condition<rarity>> make_rarity_value_list_condition(
	value_list_condition<rarity>::container_type rarities, bool allowed, position_tag origin)
{
	return std::make_shared<value_list_condition_with_field_test<rarity>>(
		official_condition_property::rarity, &item::rarity_, std::move(rarities), allowed, origin);
}

inline std::shared_ptr<range_bound_condition<integer>> make_item_level_range_bound_condition(
	range_bound<integer> bound, bool is_lower_bound, position_tag origin)
{
	return std::make_shared<range_bound_condition_with_field_test<integer>>(
		official_condition_property::item_level, &item::item_level, bound, is_lower_bound, origin);
}

inline std::shared_ptr<value_list_condition<integer>> make_item_level_value_list_condition(
	value_list_condition<integer>::container_type values, bool allowed, position_tag origin)
{
	return std::make_shared<value_list_condition_with_field_test<integer>>(
		official_condition_property::item_level, &item::item_level, std::move(values), allowed, origin);
}

inline std::shared_ptr<range_bound_condition<integer>> make_drop_level_range_bound_condition(
	range_bound<integer> bound, bool is_lower_bound, position_tag origin)
{
	return std::make_shared<range_bound_condition_with_field_test<integer>>(
		official_condition_property::drop_level, &item::drop_level, bound, is_lower_bound, origin);
}

inline std::shared_ptr<value_list_condition<integer>> make_drop_level_value_list_condition(
	value_list_condition<integer>::container_type values, bool allowed, position_tag origin)
{
	return std::make_shared<value_list_condition_with_field_test<integer>>(
		official_condition_property::drop_level, &item::drop_level, std::move(values), allowed, origin);
}

inline std::shared_ptr<range_bound_condition<integer>> make_quality_range_bound_condition(
	range_bound<integer> bound, bool is_lower_bound, position_tag origin)
{
	return std::make_shared<range_bound_condition_with_field_test<integer>>(
		official_condition_property::quality, &item::quality, bound, is_lower_bound, origin);
}

inline std::shared_ptr<value_list_condition<integer>> make_quality_value_list_condition(
	value_list_condition<integer>::container_type values, bool allowed, position_tag origin)
{
	return std::make_shared<value_list_condition_with_field_test<integer>>(
		official_condition_property::quality, &item::quality, std::move(values), allowed, origin);
}

inline std::shared_ptr<range_bound_condition<integer>> make_height_range_bound_condition(
	range_bound<integer> bound, bool is_lower_bound, position_tag origin)
{
	return std::make_shared<range_bound_condition_with_field_test<integer>>(
		official_condition_property::height, &item::height, bound, is_lower_bound, origin);
}

inline std::shared_ptr<value_list_condition<integer>> make_height_value_list_condition(
	value_list_condition<integer>::container_type values, bool allowed, position_tag origin)
{
	return std::make_shared<value_list_condition_with_field_test<integer>>(
		official_condition_property::height, &item::height, std::move(values), allowed, origin);
}

inline std::shared_ptr<range_bound_condition<integer>> make_width_range_bound_condition(
	range_bound<integer> bound, bool is_lower_bound, position_tag origin)
{
	return std::make_shared<range_bound_condition_with_field_test<integer>>(
		official_condition_property::width, &item::width, bound, is_lower_bound, origin);
}

inline std::shared_ptr<value_list_condition<integer>> make_width_value_list_condition(
	value_list_condition<integer>::container_type values, bool allowed, position_tag origin)
{
	return std::make_shared<value_list_condition_with_field_test<integer>>(
		official_condition_property::width, &item::width, std::move(values), allowed, origin);
}

inline std::shared_ptr<range_bound_condition<integer>> make_stack_size_range_bound_condition(
	range_bound<integer> bound, bool is_lower_bound, position_tag origin)
{
	return std::make_shared<range_bound_condition_with_field_test<integer>>(
		official_condition_property::stack_size, &item::stack_size, bound, is_lower_bound, origin);
}

inline std::shared_ptr<value_list_condition<integer>> make_stack_size_value_list_condition(
	value_list_condition<integer>::container_type values, bool allowed, position_tag origin)
{
	return std::make_shared<value_list_condition_with_field_test<integer>>(
		official_condition_property::stack_size, &item::stack_size, std::move(values), allowed, origin);
}

inline std::shared_ptr<range_bound_condition<integer>> make_gem_level_range_bound_condition(
	range_bound<integer> bound, bool is_lower_bound, position_tag origin)
{
	return std::make_shared<range_bound_condition_with_field_test<integer>>(
		official_condition_property::gem_level, &item::gem_level, bound, is_lower_bound, origin);
}

inline std::shared_ptr<value_list_condition<integer>> make_gem_level_value_list_condition(
	value_list_condition<integer>::container_type values, bool allowed, position_tag origin)
{
	return std::make_shared<value_list_condition_with_field_test<integer>>(
		official_condition_property::gem_level, &item::gem_level, std::move(values), allowed, origin);
}

inline std::shared_ptr<range_bound_condition<integer>> make_map_tier_range_bound_condition(
	range_bound<integer> bound, bool is_lower_bound, position_tag origin)
{
	return std::make_shared<range_bound_condition_with_field_test<integer>>(
		official_condition_property::map_tier, &item::map_tier, bound, is_lower_bound, origin);
}

inline std::shared_ptr<value_list_condition<integer>> make_map_tier_value_list_condition(
	value_list_condition<integer>::container_type values, bool allowed, position_tag origin)
{
	return std::make_shared<value_list_condition_with_field_test<integer>>(
		official_condition_property::map_tier, &item::map_tier, std::move(values), allowed, origin);
}

inline std::shared_ptr<range_bound_condition<integer>> make_corrupted_mods_range_bound_condition(
	range_bound<integer> bound, bool is_lower_bound, position_tag origin)
{
	return std::make_shared<range_bound_condition_with_field_test<integer>>(
		official_condition_property::corrupted_mods, &item::corrupted_mods, bound, is_lower_bound, origin);
}

inline std::shared_ptr<value_list_condition<integer>> make_corrupted_mods_value_list_condition(
	value_list_condition<integer>::container_type values, bool allowed, position_tag origin)
{
	return std::make_shared<value_list_condition_with_field_test<integer>>(
		official_condition_property::corrupted_mods, &item::corrupted_mods, std::move(values), allowed, origin);
}

inline std::shared_ptr<range_bound_condition<integer>> make_enchantment_passive_num_range_bound_condition(
	range_bound<integer> bound, bool is_lower_bound, position_tag origin)
{
	return std::make_shared<range_bound_condition_with_field_test<integer>>(
		official_condition_property::enchantment_passive_num, &item::enchantment_passive_num, bound, is_lower_bound, origin);
}

inline std::shared_ptr<value_list_condition<integer>> make_enchantment_passive_num_value_list_condition(
	value_list_condition<integer>::container_type values, bool allowed, position_tag origin)
{
	return std::make_shared<value_list_condition_with_field_test<integer>>(
		official_condition_property::enchantment_passive_num, &item::enchantment_passive_num, std::move(values), allowed, origin);
}

inline std::shared_ptr<range_bound_condition<integer>> make_base_armour_range_bound_condition(
	range_bound<integer> bound, bool is_lower_bound, position_tag origin)
{
	return std::make_shared<range_bound_condition_with_field_test<integer>>(
		official_condition_property::base_armour, &item::base_armour, bound, is_lower_bound, origin);
}

inline std::shared_ptr<value_list_condition<integer>> make_base_armour_value_list_condition(
	value_list_condition<integer>::container_type values, bool allowed, position_tag origin)
{
	return std::make_shared<value_list_condition_with_field_test<integer>>(
		official_condition_property::base_armour, &item::base_armour, std::move(values), allowed, origin);
}

inline std::shared_ptr<range_bound_condition<integer>> make_base_evasion_range_bound_condition(
	range_bound<integer> bound, bool is_lower_bound, position_tag origin)
{
	return std::make_shared<range_bound_condition_with_field_test<integer>>(
		official_condition_property::base_evasion, &item::base_evasion, bound, is_lower_bound, origin);
}

inline std::shared_ptr<value_list_condition<integer>> make_base_evasion_value_list_condition(
	value_list_condition<integer>::container_type values, bool allowed, position_tag origin)
{
	return std::make_shared<value_list_condition_with_field_test<integer>>(
		official_condition_property::base_evasion, &item::base_evasion, std::move(values), allowed, origin);
}

inline std::shared_ptr<range_bound_condition<integer>> make_base_energy_shield_range_bound_condition(
	range_bound<integer> bound, bool is_lower_bound, position_tag origin)
{
	return std::make_shared<range_bound_condition_with_field_test<integer>>(
		official_condition_property::base_energy_shield, &item::base_energy_shield, bound, is_lower_bound, origin);
}

inline std::shared_ptr<value_list_condition<integer>> make_base_energy_shield_value_list_condition(
	value_list_condition<integer>::container_type values, bool allowed, position_tag origin)
{
	return std::make_shared<value_list_condition_with_field_test<integer>>(
		official_condition_property::base_energy_shield, &item::base_energy_shield, std::move(values), allowed, origin);
}

inline std::shared_ptr<range_bound_condition<integer>> make_base_ward_range_bound_condition(
	range_bound<integer> bound, bool is_lower_bound, position_tag origin)
{
	return std::make_shared<range_bound_condition_with_field_test<integer>>(
		official_condition_property::base_ward, &item::base_ward, bound, is_lower_bound, origin);
}

inline std::shared_ptr<value_list_condition<integer>> make_base_ward_value_list_condition(
	value_list_condition<integer>::container_type values, bool allowed, position_tag origin)
{
	return std::make_shared<value_list_condition_with_field_test<integer>>(
		official_condition_property::base_ward, &item::base_ward, std::move(values), allowed, origin);
}

inline std::shared_ptr<range_bound_condition<integer>> make_base_defence_percentile_range_bound_condition(
	range_bound<integer> bound, bool is_lower_bound, position_tag origin)
{
	return std::make_shared<range_bound_condition_with_field_test<integer>>(
		official_condition_property::base_defence_percentile, &item::base_defence_percentile, bound, is_lower_bound, origin);
}

inline std::shared_ptr<value_list_condition<integer>> make_base_defence_percentile_value_list_condition(
	value_list_condition<integer>::container_type values, bool allowed, position_tag origin)
{
	return std::make_shared<value_list_condition_with_field_test<integer>>(
		official_condition_property::base_defence_percentile, &item::base_defence_percentile, std::move(values), allowed, origin);
}

inline std::shared_ptr<range_bound_condition<integer>> make_memory_strands_range_bound_condition(
	range_bound<integer> bound, bool is_lower_bound, position_tag origin)
{
	return std::make_shared<range_bound_condition_with_field_test<integer>>(
		official_condition_property::memory_strands, &item::memory_strands, bound, is_lower_bound, origin);
}

inline std::shared_ptr<value_list_condition<integer>> make_memory_strands_value_list_condition(
	value_list_condition<integer>::container_type values, bool allowed, position_tag origin)
{
	return std::make_shared<value_list_condition_with_field_test<integer>>(
		official_condition_property::memory_strands, &item::memory_strands, std::move(values), allowed, origin);
}

// -- function test --

inline std::shared_ptr<range_bound_condition<integer>> make_linked_sockets_range_bound_condition(
	range_bound<integer> bound, bool is_lower_bound, position_tag origin)
{
	return std::make_shared<range_bound_condition_with_function_test>(
		official_condition_property::linked_sockets, &item::linked_sockets, bound, is_lower_bound, origin);
}

inline std::shared_ptr<value_list_condition<integer>> make_linked_sockets_value_list_condition(
	value_list_condition<integer>::container_type values, bool allowed, position_tag origin)
{
	return std::make_shared<value_list_condition_with_function_test<integer>>(
		official_condition_property::linked_sockets, &item::linked_sockets, std::move(values), allowed, origin);
}

inline std::shared_ptr<range_bound_condition<integer>> make_has_searing_exarch_implicit_range_bound_condition(
	range_bound<integer> bound, bool is_lower_bound, position_tag origin)
{
	return std::make_shared<range_bound_condition_with_function_test>(
		official_condition_property::has_searing_exarch_implicit, &item::implicit_exarch_value, bound, is_lower_bound, origin);
}

inline std::shared_ptr<value_list_condition<integer>> make_has_searing_exarch_implicit_value_list_condition(
	value_list_condition<integer>::container_type values, bool allowed, position_tag origin)
{
	return std::make_shared<value_list_condition_with_function_test<integer>>(
		official_condition_property::has_searing_exarch_implicit, &item::implicit_exarch_value, std::move(values), allowed, origin);
}

inline std::shared_ptr<range_bound_condition<integer>> make_has_eater_of_worlds_implicit_range_bound_condition(
	range_bound<integer> bound, bool is_lower_bound, position_tag origin)
{
	return std::make_shared<range_bound_condition_with_function_test>(
		official_condition_property::has_eater_of_worlds_implicit, &item::implicit_eater_value, bound, is_lower_bound, origin);
}

inline std::shared_ptr<value_list_condition<integer>> make_has_eater_of_worlds_implicit_value_list_condition(
	value_list_condition<integer>::container_type values, bool allowed, position_tag origin)
{
	return std::make_shared<value_list_condition_with_function_test<integer>>(
		official_condition_property::has_eater_of_worlds_implicit, &item::implicit_eater_value, std::move(values), allowed, origin);
}

// -- area level --

inline std::shared_ptr<range_bound_condition<integer>> make_area_level_range_bound_condition(
	range_bound<integer> bound, bool is_lower_bound, position_tag origin)
{
	return std::make_shared<area_level_range_bound_condition>(
		official_condition_property::area_level, bound, is_lower_bound, origin);
}

inline std::shared_ptr<value_list_condition<integer>> make_area_level_value_list_condition(
	value_list_condition<integer>::container_type values, bool allowed, position_tag origin)
{
	return std::make_shared<area_level_value_list_condition>(
		official_condition_property::area_level, std::move(values), allowed, origin);
}

// ---- string comparison ----
/*
 * Even though some of these work with </<=/>/>= it is better to error on such.
 * The behavior with such comparison is very surprising or inconsistent.
 *
 * ArchnemesisMod actually still exists:
 * - </<=/!= matches all
 * - >/>=/= rejects all
 * - == works as expected
 */

// Base type for all string-based comparisons.
// Adds a common set of function implementations
// but does not implement actual item test.
class string_comparison_condition : public official_condition
{
public:
	using container_type = condition_values_container<string>;

	string_comparison_condition(
		official_condition_property tested_property,
		equality_comparison_type cmp,
		container_type values,
		position_tag origin)
	: official_condition(tested_property, test_type::values_equal, origin)
	, m_comparison_type(cmp)
	, m_values(std::move(values))
	{}

	bool is_valid() const final { return !m_values.empty(); }

	void print(std::ostream& os) const final;

	bool allows_item_class(std::string_view class_name) const final;

protected:
	condition_match_result test_item_impl(const std::string* item_field) const;

private:
	equality_comparison_type m_comparison_type;
	container_type m_values;
};

// Adds an item test implementation and a protected interface function
// to supply the property from the item to match.
// A potential design alternative would be to take pointer-to-member
// but virtual function is more flexible - it allows to return null
// in case item's property is std::optional<std::string>.
class string_comparison_condition_with_field_test : public string_comparison_condition
{
public:
	string_comparison_condition_with_field_test(
		official_condition_property tested_property,
		equality_comparison_type cmp,
		container_type values,
		position_tag origin)
	: string_comparison_condition(tested_property, cmp, values, origin)
	{}

	condition_match_result test_item(const item& itm, int /* area_level */) const final
	{
		return test_item_impl(get_item_field(itm));
	}

protected:
	virtual const std::string* get_item_field(const item& itm) const = 0;
};

class string_comparison_condition_with_string_field_test : public string_comparison_condition_with_field_test
{
public:
	string_comparison_condition_with_string_field_test(
		official_condition_property tested_property,
		std::string item::* tested_field,
		equality_comparison_type cmp,
		container_type values,
		position_tag origin)
	: string_comparison_condition_with_field_test(tested_property, cmp, std::move(values), origin)
	, m_tested_field(tested_field)
	{}

protected:
	const std::string* get_item_field(const item& itm) const final { return &(itm.*m_tested_field); }

private:
	std::string item::* m_tested_field;
};

inline std::shared_ptr<string_comparison_condition> make_class_condition(
	equality_comparison_type cmp,
	string_comparison_condition::container_type values,
	position_tag origin)
{
	return std::make_shared<string_comparison_condition_with_string_field_test>(
		official_condition_property::class_, &item::class_, cmp, std::move(values), origin);
}

inline std::shared_ptr<string_comparison_condition> make_base_type_condition(
	equality_comparison_type cmp,
	string_comparison_condition::container_type values,
	position_tag origin)
{
	return std::make_shared<string_comparison_condition_with_string_field_test>(
		official_condition_property::base_type, &item::base_type, cmp, std::move(values), origin);
}

class string_comparison_condition_with_optional_string_field_test : public string_comparison_condition_with_field_test
{
public:
	string_comparison_condition_with_optional_string_field_test(
		official_condition_property tested_property,
		std::optional<std::string> item::* tested_field,
		equality_comparison_type cmp,
		container_type values,
		position_tag origin)
	: string_comparison_condition_with_field_test(tested_property, cmp, std::move(values), origin)
	, m_tested_field(tested_field)
	{}

protected:
	const std::string* get_item_field(const item& itm) const final
	{
		const std::optional<std::string>& opt = itm.*m_tested_field;
		if (opt)
			return &opt.value();
		else
			return nullptr;
	}

private:
	std::optional<std::string> item::* m_tested_field;
};

inline std::shared_ptr<string_comparison_condition> make_enchantment_passive_node_condition(
	equality_comparison_type cmp, string_comparison_condition::container_type values, position_tag origin)
{
	return std::make_shared<string_comparison_condition_with_optional_string_field_test>(
		official_condition_property::enchantment_passive_node, &item::enchantment_cluster_jewel, cmp, std::move(values), origin);
}

inline std::shared_ptr<string_comparison_condition> make_archnemesis_mod_condition(
	equality_comparison_type cmp, string_comparison_condition::container_type values, position_tag origin)
{
	return std::make_shared<string_comparison_condition_with_optional_string_field_test>(
		official_condition_property::archnemesis_mod, &item::archnemesis_mod, cmp, std::move(values), origin);
}

// string variant of the TransfiguredGem condition
class transfigured_gem_string_comparison_condition : public string_comparison_condition
{
public:
	transfigured_gem_string_comparison_condition(
		equality_comparison_type cmp,
		container_type values,
		position_tag origin)
	: string_comparison_condition(official_condition_property::transfigured_gem, cmp, std::move(values), origin)
	{}

	condition_match_result test_item(const item& itm, int /* area_level */) const final
	{
		// Fail early if the item is not a TransfiguredGem,
		// otherwise testing the string (gem name) has no point.
		if (!itm.is_transfigured_gem)
			return condition_match_result(false, origin());

		return test_item_impl(&itm.base_type);
	}
};

inline std::shared_ptr<string_comparison_condition> make_transfigured_gem_condition_string_version(
	equality_comparison_type cmp,
	string_comparison_condition::container_type values,
	position_tag origin)
{
	return std::make_shared<transfigured_gem_string_comparison_condition>(cmp, std::move(values), origin);
}

// ---- counted string comparison ----
/*
 * != completely bugged, like random results
 * all other work as expected (number of (exact) matches)
 *
 * - warn if "HasExplicitMod >= 3" is encountered
 *   (it won't work as expected - 3 treated as a mod, not as a number)
 * - do not warn on "HasExplicitMod >= 3 <less than 3 mods>" because
 *   some mods (e.g. "Shaper's" can appear multiple times)
 */

class counted_string_comparison_condition : public official_condition
{
public:
	using container_type = condition_values_container<string>;

	counted_string_comparison_condition(
		official_condition_property tested_property,
		comparison_type cmp,
		std::optional<integer> count,
		container_type values,
		position_tag origin)
	: official_condition(tested_property, test_type::values_equal, origin)
	, m_comparison_type(cmp)
	, m_count(count)
	, m_values(std::move(values))
	{}

	condition_match_result test_item(const item& itm, int area_level) const final;

	bool is_valid() const final
	{
		if (m_values.empty())
			return false;

		// != does not work for these
		if (m_comparison_type == comparison_type::not_equal)
			return false;

		if (!m_count) {
			// without the count, only = and == make sense
			if (m_comparison_type != comparison_type::equal && m_comparison_type != comparison_type::exact_match)
				return false;
		}

		return true;
	}

	void print(std::ostream& os) const final;

protected:
	virtual int count_matches(const item& itm, const container_type& values, bool exact_match_required) const = 0;

private:
	comparison_type m_comparison_type;
	std::optional<integer> m_count;
	container_type m_values;
};

class has_explicit_mod_condition : public counted_string_comparison_condition
{
public:
	has_explicit_mod_condition(
		comparison_type cmp,
		std::optional<integer> count,
		container_type values,
		position_tag origin)
	: counted_string_comparison_condition(
		official_condition_property::has_explicit_mod, cmp, count, std::move(values), origin)
	{}

protected:
	int count_matches(const item& itm, const container_type& values, bool exact_match_required) const final;
};

inline std::shared_ptr<counted_string_comparison_condition> make_has_explicit_mod_condition(
	comparison_type cmp,
	std::optional<integer> count,
	counted_string_comparison_condition::container_type values,
	position_tag origin)
{
	return std::make_shared<has_explicit_mod_condition>(cmp, count, std::move(values), origin);
}

// GGG documents this condition similar to Class but actually it supports counted comparison
class has_enchantment_condition : public counted_string_comparison_condition
{
public:
	has_enchantment_condition(
		comparison_type cmp,
		std::optional<integer> count,
		container_type values,
		position_tag origin)
	: counted_string_comparison_condition(
		official_condition_property::has_enchantment, cmp, count, std::move(values), origin)
	{}

protected:
	int count_matches(const item& itm, const container_type& values, bool exact_match_required) const final;
};

inline std::shared_ptr<counted_string_comparison_condition> make_has_enchantment_condition(
	comparison_type cmp,
	std::optional<integer> count,
	counted_string_comparison_condition::container_type values,
	position_tag origin)
{
	return std::make_shared<has_enchantment_condition>(cmp, count, std::move(values), origin);
}

// ---- socket ----

class socket_specification_condition : public official_condition
{
public:
	using container_type = condition_values_container<socket_spec>;

	socket_specification_condition(comparison_type cmp, container_type specs, bool group_matters, position_tag origin)
	: official_condition(
		group_matters ? official_condition_property::socket_group : official_condition_property::sockets,
		test_type::sockets,
		origin)
	, m_comparison_type(cmp)
	, m_values(std::move(specs))
	{}

	bool is_valid() const final { return !m_values.empty(); }

	condition_match_result test_item(const item& itm, int area_level) const final;

	void print(std::ostream& os) const final;

private:
	comparison_type m_comparison_type;
	container_type m_values;
};

inline auto make_sockets_condition(comparison_type cmp, socket_specification_condition::container_type specs, position_tag origin)
{
	return std::make_shared<socket_specification_condition>(cmp, std::move(specs), false, origin);
}

inline auto make_socket_group_condition(comparison_type cmp, socket_specification_condition::container_type specs, position_tag origin)
{
	return std::make_shared<socket_specification_condition>(cmp, std::move(specs), true, origin);
}

// ---- spirit filter extensions ----

struct price_bound
{
	range_bound<fractional> bound;
	position_tag origin;
};

class price_range_condition // : public condition // not needed currently
{
public:
	price_range_condition() = default;
	price_range_condition(fractional val, position_tag origin)
	{
		set_exact(val, origin);
	}

	bool is_exact() const noexcept
	{
		if (m_lower_bound.has_value() && m_upper_bound.has_value())
			return (*m_lower_bound).bound == (*m_upper_bound).bound;

		return false;
	}

	bool test_lower_bound(double value) const noexcept
	{
		if (m_lower_bound.has_value()) {
			if ((*m_lower_bound).bound.value.value > value)
				return false;

			if (!(*m_lower_bound).bound.inclusive && compare_doubles((*m_lower_bound).bound.value.value, value))
				return false;
		}

		return true;
	}

	bool test_upper_bound(double value) const noexcept
	{
		if (m_upper_bound.has_value()) {
			if ((*m_upper_bound).bound.value.value < value)
				return false;

			if (!(*m_upper_bound).bound.inclusive && compare_doubles((*m_upper_bound).bound.value.value, value))
				return false;
		}

		return true;
	}

	// check whether 'value' can fit into currently specified range
	bool includes(double value) const noexcept
	{
		return test_lower_bound(value) && test_upper_bound(value);
	}

	bool has_bound() const noexcept
	{
		return m_lower_bound.has_value() || m_upper_bound.has_value();
	}

	auto lower_bound() const noexcept
	{
		return m_lower_bound;
	}

	auto upper_bound() const noexcept
	{
		return m_upper_bound;
	}

	void set_exact(fractional value, position_tag origin) noexcept
	{
		m_lower_bound = m_upper_bound = price_bound{range_bound<fractional>{value, true}, origin};
	}

	void set_lower_bound(range_bound<fractional> bound, position_tag origin) noexcept
	{
		m_lower_bound = price_bound{bound, origin};
	}

	void set_upper_bound(range_bound<fractional> bound, position_tag origin) noexcept
	{
		m_upper_bound = price_bound{bound, origin};
	}

	std::optional<lang::position_tag> first_origin() const noexcept
	{
		if (m_lower_bound.has_value() && m_upper_bound.has_value()) {
			if (compare((*m_lower_bound).origin, (*m_upper_bound).origin) < 0)
				return (*m_lower_bound).origin;
			else
				return (*m_upper_bound).origin;
		}
		else if (m_lower_bound.has_value()) {
			return (*m_lower_bound).origin;
		}
		else if (m_upper_bound.has_value()) {
			return (*m_upper_bound).origin;
		}
		else {
			return std::nullopt;
		}
	}

	std::optional<lang::position_tag> second_origin() const noexcept
	{
		if (m_lower_bound.has_value() && m_upper_bound.has_value()) {
			if (compare((*m_lower_bound).origin, (*m_upper_bound).origin) < 0)
				return (*m_upper_bound).origin;
			else
				return (*m_lower_bound).origin;
		}
		else {
			return std::nullopt;
		}
	}

private:
	std::optional<price_bound> m_lower_bound;
	std::optional<price_bound> m_upper_bound;
};

// ---- high-level types ----

struct official_conditions
{
	struct search_results
	{
		std::optional<position_tag> lower_bound;
		std::optional<position_tag> upper_bound;
		std::optional<position_tag> values_equal;
		std::optional<position_tag> values_unequal;
		std::optional<position_tag> other;
	};

	search_results search(official_condition_property property) const
	{
		search_results results;

		for (const auto& cond : conditions) {
			if (cond->tested_property() != property)
				continue;

			// If there are duplicates, only 1 will be reported.
			// It is up to the caller to prevent duplicates
			// (e.g. by using this function to block adding them).
			switch (cond->type()) {
				case official_condition::test_type::lower_bound:
					results.lower_bound = cond->origin();
					continue;
				case official_condition::test_type::upper_bound:
					results.upper_bound = cond->origin();
					continue;
				case official_condition::test_type::values_equal:
					results.values_equal = cond->origin();
					continue;
				case official_condition::test_type::values_unequal:
					results.values_unequal = cond->origin();
					continue;
				case official_condition::test_type::sockets:
				case official_condition::test_type::boolean_state:
					results.other = cond->origin();
					continue;
			}
		}

		return results;
	}

	/**
	 * @brief determines whether given set of conditions is valid
	 *
	 * @details if a string condition (e.g. BaseType) is an empty array
	 * the resulting filter would not be valid - PoE client will not
	 * accept BaseType conditions that do not specify any base name.
	 *
	 * Invalid conditions may be a result of autogeneration (no items matching given price)
	 * or (a combination of) variables that result in an empty list.
	 *
	 * Since no matches can be made against invalid (empty) conditions,
	 * such filter blocks should be simply discarded (that is, not printed).
	 *
	 * Note: a filter block without conditions is fine. Typically used as
	 * a starter block (with default styles and Continue) or as an end/error block.
	 */
	[[nodiscard]] bool is_valid() const
	{
		return std::all_of(conditions.begin(), conditions.end(), [](const auto& cond) { return cond->is_valid(); });
	}

	void print(std::ostream& os) const
	{
		for (const auto& cond : conditions)
			cond->print(os);
	}

	std::vector<std::shared_ptr<official_condition>> conditions; // never null
};

}
