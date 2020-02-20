/**
 * @file AST definitions
 *
 * @brief This file is intended for type definitions that will be used for AST
 */
#pragma once

#include <fs/lang/primitive_types.hpp>
#include <fs/lang/action_properties.hpp>
#include <fs/lang/condition_properties.hpp>
#include <fs/lang/queries.hpp>

#include <boost/optional.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>

#include <utility>
#include <string>
#include <type_traits>

/*
 * Warning: std::optional and std::tuple are not yet supported,
 * use Boost counterparts instead.
 *
 * Do not use std::variant, inherit from x3::variant instead.
 *
 * https://github.com/boostorg/spirit/issues/270
 */

namespace fs::parser::ast
{

namespace x3 = boost::spirit::x3;

// tokens both exist in spirit filter and in real filter
namespace common
{
	struct identifier : x3::position_tagged
	{
		identifier& operator=(std::string str)
		{
			value = std::move(str);
			return *this;
		}

		const std::string& get_value() const { return value; }

		std::string value;
	};

	struct boolean_literal : x3::position_tagged
	{
		boolean_literal& operator=(bool b)
		{
			value = b;
			return *this;
		}

		bool get_value() const { return value; }

		bool value;
	};

	struct integer_literal : x3::position_tagged
	{
		integer_literal& operator=(int n)
		{
			value = n;
			return *this;
		}

		int get_value() const { return value; }

		// add an initializer here because this type
		// is exposed using x3::attr in the grammar
		int value = 0;
	};

	struct string_literal : std::string, x3::position_tagged {};

	struct rarity_literal : x3::position_tagged
	{
		rarity_literal& operator=(lang::rarity r)
		{
			value = r;
			return *this;
		}

		lang::rarity get_value() const { return value; }

		lang::rarity value;
	};

	struct shape_literal : x3::position_tagged
	{
		shape_literal& operator=(lang::shape s)
		{
			value = s;
			return *this;
		}

		lang::shape get_value() const { return value; }

		lang::shape value;
	};

	struct suit_literal : x3::position_tagged
	{
		suit_literal& operator=(lang::suit s)
		{
			value = s;
			return *this;
		}

		lang::suit get_value() const { return value; }

		lang::suit value;
	};

	struct influence_literal : x3::position_tagged
	{
		influence_literal& operator=(lang::influence i)
		{
			value = i;
			return *this;
		}

		lang::influence get_value() const { return value; }

		lang::influence value;
	};

	struct socket_spec_literal : x3::position_tagged
	{
		integer_literal socket_count;
		identifier socket_colors;
	};

	struct none_literal : x3::position_tagged
	{
		void get_value() const {}
	};

	// < or > or <= or >= or = or nothing (defaults to =)
	struct comparison_operator_expression : x3::position_tagged
	{
		comparison_operator_expression& operator=(lang::comparison_type ct)
		{
			value = ct;
			return *this;
		}

		lang::comparison_type get_value() const { return value; }

		lang::comparison_type value;
	};

	// optional == token
	struct exact_matching_policy : x3::position_tagged
	{
		exact_matching_policy& operator=(bool v)
		{
			required = v;
			return *this;
		}

		bool get_value() const { return required; }

		bool required;
	};

	struct visibility_statement : x3::position_tagged
	{
		visibility_statement operator=(bool value)
		{
			show = value;
			return *this;
		}

		bool get_value() const { return show; }

		bool show;
	};
}

// tokens that exist only in spirit filter
namespace sf
{
	// ---- whitespace ----

	// all whitespace and comments are ignored
	// no point in saving them, hence nothing here

	// ---- fundamental tokens ----

	using identifier = common::identifier;

	// ---- literal types ----

	using integer_literal = common::integer_literal;

	struct floating_point_literal : x3::position_tagged
	{
		floating_point_literal& operator=(double n)
		{
			value = n;
			return *this;
		}

		double get_value() const { return value; }

		double value;
	};

	using string_literal = common::string_literal;
	using boolean_literal = common::boolean_literal;
	using rarity_literal = common::rarity_literal;
	using shape_literal = common::shape_literal;
	using suit_literal = common::suit_literal;
	using influence_literal = common::influence_literal;
	using none_literal = common::none_literal;
	using socket_spec_literal = common::socket_spec_literal;

	// ---- expressions ----

	struct literal_expression : x3::variant<
			floating_point_literal,
			socket_spec_literal,
			integer_literal,
			string_literal,
			boolean_literal,
			rarity_literal,
			shape_literal,
			suit_literal,
			influence_literal,
			none_literal
		>, x3::position_tagged
	{
		using base_type::base_type;
		using base_type::operator=;
	};

	struct query : x3::position_tagged
	{
		auto& operator=(lang::query qry)
		{
			q = qry;
			return *this;
		}

		auto get_value() const { return q; }

		lang::query q;
	};

	struct primitive_value : x3::variant<literal_expression, identifier>, x3::position_tagged
	{
		using base_type::base_type;
		using base_type::operator=;
	};

	struct sequence : std::vector<primitive_value>, x3::position_tagged {};

	struct action;

	struct compound_action_expression : std::vector<action>, x3::position_tagged {};

	struct value_expression : x3::variant<
			sequence,
			compound_action_expression
		>, x3::position_tagged
	{
		using base_type::base_type;
		using base_type::operator=;
	};

	/*
	* https://github.com/boostorg/spirit/issues/465
	* There are 2 bugs in GCC that may cause problems:
	*
	* GCC 6.4 - 9.0-: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=89381
	* Using declarations for base_type's operator= and base_type's ctor in types
	* inheritted from base_type which was a class template did not actually
	* created copy/move ctor/operators - ending in very unexpected compiler errors.
	* that an object can not be copied/moved or copy/move assigned.
	*
	* Workaround: place these usings after each problematic type T definition,
	* they will add missing copy and move ctors.
	* using workaround1 = decltype(T{*static_cast<const T*>(nullptr)});
	* using workaround2 = decltype(T{static_cast<T&&>(*static_cast<T*>(nullptr))});
	*
	* GCC 9.0+: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=92145
	* The above bug is fixed but the newly added -Wdeprecated-copy has
	* a false positive on the same type definition. Workaround above does not help.
	* Simply use -Wno-deprecated-copy.
	*/

	// ---- definitions ----

	struct constant_definition : x3::position_tagged
	{
		identifier name;
		value_expression value;
	};

	struct definition : x3::position_tagged
	{
		auto& operator=(constant_definition cd)
		{
			def = std::move(cd);
			return *this;
		}

		const constant_definition& get_value() const { return def; }

		constant_definition def;
	};

	// ---- conditions ----

	using comparison_operator_expression = common::comparison_operator_expression;
	using exact_matching_policy = common::exact_matching_policy;

	struct rarity_comparison_condition : x3::position_tagged
	{
		comparison_operator_expression comparison_type;
		sequence seq;
	};

	struct numeric_comparison_condition : x3::position_tagged
	{
		lang::numeric_comparison_condition_property property;
		comparison_operator_expression comparison_type;
		sequence seq;
	};

	struct string_array_condition : x3::position_tagged
	{
		lang::string_array_condition_property property;
		exact_matching_policy exact_match;
		sequence seq;
	};

	struct has_influence_condition : x3::position_tagged
	{
		exact_matching_policy exact_match;
		sequence seq;
	};

	struct socket_spec_condition : x3::position_tagged
	{
		bool links_matter;
		lang::socket_spec_comparison_type comparison_type;
		sequence seq;
	};

	struct boolean_condition : x3::position_tagged
	{
		lang::boolean_condition_property property;
		sequence seq;
	};

	struct condition : x3::variant<
			rarity_comparison_condition,
			numeric_comparison_condition,
			string_array_condition,
			has_influence_condition,
			socket_spec_condition,
			boolean_condition
		>, x3::position_tagged
	{
		using base_type::base_type;
		using base_type::operator=;
	};

	// ---- actions ----

	struct set_color_action : x3::position_tagged
	{
		lang::color_action_type action_type;
		sequence seq;
	};

	struct set_font_size_action : x3::position_tagged
	{
		auto& operator=(sequence s)
		{
			seq = std::move(s);
			return *this;
		}

		const auto& get_value() const { return seq; }

		sequence seq;
	};

	struct minimap_icon_action : x3::position_tagged
	{
		auto& operator=(sequence s)
		{
			seq = std::move(s);
			return *this;
		}

		const auto& get_value() const { return seq; }

		sequence seq;
	};

	struct play_effect_action : x3::position_tagged
	{
		auto& operator=(sequence s)
		{
			seq = std::move(s);
			return *this;
		}

		const auto& get_value() const { return seq; }

		sequence seq;
	};

	struct play_alert_sound_action : x3::position_tagged
	{
		bool positional;
		sequence seq;
	};

	struct custom_alert_sound_action : x3::position_tagged
	{
		auto& operator=(sequence s)
		{
			seq = std::move(s);
			return *this;
		}

		const auto& get_value() const { return seq; }

		sequence seq;
	};

	struct disable_drop_sound_action : x3::position_tagged
	{
		void get_value() const {}
	};

	struct compound_action : x3::position_tagged
	{
		auto& operator=(sequence s)
		{
			seq = std::move(s);
			return *this;
		}

		const auto& get_value() const { return seq; }

		sequence seq;
	};

	struct action : x3::variant<
			set_color_action,
			set_font_size_action,
			minimap_icon_action,
			play_effect_action,
			play_alert_sound_action,
			custom_alert_sound_action,
			disable_drop_sound_action,
			compound_action
		>, x3::position_tagged
	{
		using base_type::base_type;
		using base_type::operator=;
	};

	// ---- filter structure ----

	using visibility_statement = common::visibility_statement;

	// rule_block defined earlier than statement due to circular dependency
	// note: we could use x3::forward_ast but it would have a worse memory layout
	struct rule_block : x3::position_tagged
	{
		std::vector<condition> conditions;
		std::vector<struct statement> statements;
	};

	struct statement : x3::variant<
			action,
			visibility_statement,
			rule_block
		>, x3::position_tagged
	{
		using base_type::base_type;
		using base_type::operator=;
	};

	struct filter_structure : x3::position_tagged
	{
		std::vector<definition> definitions;
		std::vector<statement> statements;
	};

	using ast_type = filter_structure;

} // namespace sf

// tokens that exist only in real filters
namespace rf
{
	using identifier = common::identifier;

	using boolean_literal = common::boolean_literal;
	using integer_literal = common::integer_literal;
	using suit_literal = common::suit_literal;
	using shape_literal = common::shape_literal;
	using rarity_literal = common::rarity_literal;
	using string_literal = common::string_literal;
	using influence_literal = common::influence_literal;
	using socket_spec_literal = common::socket_spec_literal;

	struct color_literal : x3::position_tagged
	{
		integer_literal r;
		integer_literal g;
		integer_literal b;
		boost::optional<integer_literal> a;
	};

	struct icon_literal : x3::position_tagged
	{
		integer_literal size;
		suit_literal suit;
		shape_literal shape;
	};

	struct string_literal_array : std::vector<string_literal>, x3::position_tagged {};

	struct influence_literal_array : std::vector<influence_literal>, x3::position_tagged {};

	using comparison_operator_expression = common::comparison_operator_expression;
	using exact_matching_policy = common::exact_matching_policy;

	struct rarity_condition : x3::position_tagged
	{
		comparison_operator_expression comparison_type;
		rarity_literal rarity;
	};

	struct numeric_condition : x3::position_tagged
	{
		lang::numeric_comparison_condition_property property;
		comparison_operator_expression comparison_type;
		integer_literal integer;
	};

	struct string_array_condition : x3::position_tagged
	{
		lang::string_array_condition_property property;
		exact_matching_policy exact_match;
		string_literal_array string_literals;
	};

	struct has_influence_condition : x3::position_tagged
	{
		exact_matching_policy exact_match;
		influence_literal_array influence_literals;
	};

	struct socket_spec_condition : x3::position_tagged
	{
		bool links_matter;
		lang::socket_spec_comparison_type comparison_type;
		std::vector<socket_spec_literal> specs;
	};

	struct boolean_condition : x3::position_tagged
	{
		lang::boolean_condition_property property;
		boolean_literal value;
	};

	struct condition : x3::variant<
		rarity_condition,
		numeric_condition,
		string_array_condition,
		has_influence_condition,
		socket_spec_condition,
		boolean_condition
	>, x3::position_tagged
	{
		using base_type::base_type;
		using base_type::operator=;
	};

	struct color_action : x3::position_tagged
	{
		lang::color_action_type action;
		color_literal color;
	};

	struct set_font_size_action : x3::position_tagged
	{
		auto& operator=(integer_literal il)
		{
			font_size = il;
			return *this;
		}

		const auto& get_value() const { return font_size; }

		integer_literal font_size;
	};

	struct play_alert_sound_action : x3::position_tagged
	{
		integer_literal sound_id;
		boost::optional<integer_literal> volume;
	};

	struct play_alert_sound_positional_action : x3::position_tagged
	{
		integer_literal sound_id;
		boost::optional<integer_literal> volume;
	};

	struct custom_alert_sound_action : x3::position_tagged
	{
		auto& operator=(string_literal sl)
		{
			path = std::move(sl);
			return *this;
		}

		const auto& get_value() const { return path; }

		string_literal path;
	};

	struct disable_drop_sound_action : x3::position_tagged {};

	struct minimap_icon_action : x3::position_tagged
	{
		auto& operator=(icon_literal il)
		{
			icon = il;
			return *this;
		}

		const auto& get_value() const { return icon; }

		icon_literal icon;
	};

	struct play_effect_action : x3::position_tagged
	{
		suit_literal suit;
		bool is_temporary;
	};

	struct action : x3::variant<
		color_action,
		set_font_size_action,
		play_alert_sound_action,
		play_alert_sound_positional_action,
		custom_alert_sound_action,
		disable_drop_sound_action,
		minimap_icon_action,
		play_effect_action
	>, x3::position_tagged
	{
		using base_type::base_type;
		using base_type::operator=;
	};

	struct rule : x3::variant<condition, action>, x3::position_tagged
	{
		using base_type::base_type;
		using base_type::operator=;
	};

	using visibility_statement = common::visibility_statement;

	struct filter_block : x3::position_tagged
	{
		visibility_statement visibility;
		std::vector<rule> rules;
	};

	using ast_type = std::vector<filter_block>;
} // namespace rf

} // namespace fs::parser::ast

namespace fs::parser
{
	// TODO rename to position_tag_of
	template <typename T>
	boost::spirit::x3::position_tagged get_position_info(const T& ast)
	{
		static_assert(std::is_base_of_v<boost::spirit::x3::position_tagged, T>, "T must be derived from position_tagged");
		// intentional object slicing
		return static_cast<boost::spirit::x3::position_tagged>(ast);
	}
}
