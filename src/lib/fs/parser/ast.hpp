/**
 * @file AST definitions
 *
 * @brief This file is intended for type definitions that will be used for AST
 */
#pragma once

#include <fs/lang/primitive_types.hpp>
#include <fs/lang/enum_types.hpp>

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

		int value;
	};

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

	struct string_literal : std::string, x3::position_tagged {};

	struct socket_spec_literal : x3::position_tagged
	{
		boost::optional<integer_literal> socket_count;
		identifier socket_colors;
	};

	struct rarity_literal : x3::position_tagged
	{
		rarity_literal& operator=(lang::rarity_type r)
		{
			value = r;
			return *this;
		}

		lang::rarity_type get_value() const { return value; }

		lang::rarity_type value;
	};

	struct shape_literal : x3::position_tagged
	{
		shape_literal& operator=(lang::shape_type s)
		{
			value = s;
			return *this;
		}

		lang::shape_type get_value() const { return value; }

		lang::shape_type value;
	};

	struct suit_literal : x3::position_tagged
	{
		suit_literal& operator=(lang::suit_type s)
		{
			value = s;
			return *this;
		}

		lang::suit_type get_value() const { return value; }

		lang::suit_type value;
	};

	struct influence_literal : x3::position_tagged
	{
		influence_literal& operator=(lang::influence_type i)
		{
			value = i;
			return *this;
		}

		lang::influence_type get_value() const { return value; }

		lang::influence_type value;
	};

	struct shaper_voice_line_literal : x3::position_tagged
	{
		auto& operator=(lang::shaper_voice_line_type sh)
		{
			value = sh;
			return *this;
		}

		auto get_value() const { return value; }

		lang::shaper_voice_line_type value;
	};

	struct temp_literal : x3::position_tagged
	{
		void get_value() const {}
	};

	struct none_literal : x3::position_tagged
	{
		void get_value() const {}
	};

	struct literal_expression : x3::variant<
		boolean_literal,
		rarity_literal,
		shape_literal,
		suit_literal,
		influence_literal,
		shaper_voice_line_literal,
		temp_literal,
		none_literal,
		socket_spec_literal,
		floating_point_literal,
		integer_literal,
		string_literal
	>, x3::position_tagged
	{
		using base_type::base_type;
		using base_type::operator=;
	};

	struct comparison_operator : x3::position_tagged
	{
		static comparison_operator implicit_default()
		{
			comparison_operator result;
			result.value = lang::comparison_type::equal;
			return result;
		}

		comparison_operator& operator=(lang::comparison_type ct)
		{
			value = ct;
			return *this;
		}

		lang::comparison_type get_value() const { return value; }

		lang::comparison_type value;
	};

	struct comparison_expression : x3::position_tagged
	{
		comparison_operator operator_;
		boost::optional<integer_literal> integer;
	};

	struct unknown_expression : std::string, x3::position_tagged {};

	struct continue_statement : x3::position_tagged
	{
		void get_value() const {}
	};

	struct static_visibility_statement : x3::position_tagged
	{
		static_visibility_statement operator=(lang::item_visibility_policy policy)
		{
			value = policy;
			return *this;
		}

		auto get_value() const { return value; }

		lang::item_visibility_policy value;
	};
}

// tokens that exist only in real filters
namespace rf
{
	using identifier = common::identifier;

	using boolean_literal = common::boolean_literal;
	using integer_literal = common::integer_literal;
	using floating_point_literal = common::floating_point_literal;
	using socket_spec_literal = common::socket_spec_literal;
	using suit_literal = common::suit_literal;
	using shape_literal = common::shape_literal;
	using rarity_literal = common::rarity_literal;
	using string_literal = common::string_literal;
	using influence_literal = common::influence_literal;
	using shaper_voice_line_literal = common::shaper_voice_line_literal;
	using none_literal = common::none_literal;

	// TODO test that real filters support unquoted strings
	struct string : x3::position_tagged
	{
		auto& operator=(common::string_literal str)
		{
			value = std::move(static_cast<std::string&>(str));
			return *this;
		}

		auto& operator=(common::identifier str)
		{
			value = std::move(str.value);
			return *this;
		}

		const auto& get_value() const { return value; }

		std::string value;
	};

	using literal_expression = common::literal_expression;
	struct literal_sequence : std::vector<literal_expression>, x3::position_tagged {};

	struct condition : x3::position_tagged
	{
		lang::official_condition_property property;
		common::comparison_expression comparison;
		literal_sequence seq;
	};

	struct action : x3::position_tagged
	{
		lang::official_action_property property;
		literal_sequence seq;
	};

	using continue_statement = common::continue_statement;

	struct rule : x3::variant<condition, action, continue_statement>, x3::position_tagged
	{
		using base_type::base_type;
		using base_type::operator=;
	};

	using static_visibility_statement = common::static_visibility_statement;

	struct filter_block : x3::position_tagged
	{
		static_visibility_statement visibility;
		std::vector<rule> rules;
	};

	using ast_type = std::vector<filter_block>;
} // namespace rf

// tokens that exist only in spirit filter
namespace sf
{
	// ---- whitespace ----

	// all whitespace and comments are ignored
	// no point in saving them, hence nothing here

	// ---- fundamental tokens ----

	using identifier = common::identifier;

	// "$" followed by identifier
	struct name : x3::position_tagged
	{
		auto& operator=(identifier id)
		{
			value = std::move(id);
			return *this;
		}

		const auto& get_value() const { return value; }

		identifier value;
	};

	// ---- literal types ----

	// (all in common)

	// ---- expressions ----

	struct primitive_value : x3::variant<name, common::literal_expression, common::unknown_expression>, x3::position_tagged
	{
		using base_type::base_type;
		using base_type::operator=;
	};

	struct sequence : std::vector<primitive_value>, x3::position_tagged {};

	struct statement;

	struct statement_list_expression : std::vector<statement>, x3::position_tagged {};

	struct value_expression : x3::variant<
			statement_list_expression,
			sequence
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
	* created copy/move ctor/operators - ending in very unexpected compiler errors
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
		name value_name;
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

	struct official_condition : x3::position_tagged
	{
		lang::official_condition_property property;
		common::comparison_expression comparison;
		sequence seq;
	};

	struct autogen_condition : x3::position_tagged
	{
		auto& operator=(common::string_literal autogen_name)
		{
			name = std::move(autogen_name);
			return *this;
		}

		const auto& get_value() const { return name; }

		common::string_literal name;
	};

	struct price_comparison_condition : x3::position_tagged
	{
		common::comparison_expression comparison;
		sequence seq;
	};

	struct condition : x3::variant<
			autogen_condition,
			price_comparison_condition,
			official_condition
		>, x3::position_tagged
	{
		using base_type::base_type;
		using base_type::operator=;
	};

	// ---- actions ----

	struct official_action : x3::position_tagged
	{
		lang::official_action_property property;
		sequence seq;
	};

	struct set_alert_sound_action : x3::position_tagged
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
			official_action,
			set_alert_sound_action
		>, x3::position_tagged
	{
		using base_type::base_type;
		using base_type::operator=;
	};

	// ---- filter structure ----

	struct expand_statement : x3::position_tagged
	{
		auto& operator=(sequence s)
		{
			seq = std::move(s);
			return *this;
		}

		const auto& get_value() const { return seq; }

		sequence seq;
	};

	using static_visibility_statement = common::static_visibility_statement;
	using continue_statement = common::continue_statement;

	struct dynamic_visibility_policy : x3::position_tagged
	{
		auto& operator=(lang::item_visibility_policy policy)
		{
			value = policy;
			return *this;
		}

		auto get_value() const { return value; }

		lang::item_visibility_policy value;
	};

	struct dynamic_visibility_statement : x3::position_tagged
	{
		dynamic_visibility_policy policy;
		sequence seq;
	};

	struct visibility_statement : x3::variant<
			static_visibility_statement,
			dynamic_visibility_statement
		>, x3::position_tagged
	{
		using base_type::base_type;
		using base_type::operator=;
	};

	struct behavior_statement : x3::position_tagged
	{
		visibility_statement visibility;
		boost::optional<continue_statement> continue_;
	};

	struct unknown_statement : x3::position_tagged
	{
		common::identifier name;
		common::comparison_expression comparison;
		sequence seq;
	};

	// rule_block defined earlier than statement due to circular dependency
	// note: we could use x3::forward_ast but it would have a worse memory layout
	struct rule_block : std::vector<statement>, x3::position_tagged {};

	struct statement : x3::variant<
			condition,
			action,
			expand_statement,
			behavior_statement,
			rule_block,
			unknown_statement
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

} // namespace fs::parser::ast

namespace fs::parser
{
	template <typename T>
	boost::spirit::x3::position_tagged position_tag_of(const T& ast)
	{
		static_assert(std::is_base_of_v<boost::spirit::x3::position_tagged, T>, "T must be derived from position_tagged");
		// intentional object slicing
		return static_cast<boost::spirit::x3::position_tagged>(ast);
	}
}
