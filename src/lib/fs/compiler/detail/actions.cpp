#include <fs/compiler/detail/evaluate.hpp>
#include <fs/compiler/detail/actions.hpp>
#include <fs/utility/assert.hpp>

#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>

#include <utility>
#include <type_traits>

namespace ast = fs::parser::ast;
namespace x3 = boost::spirit::x3;

namespace
{

using namespace fs;
using namespace fs::compiler;

// ---- spirit filter helpers ----

[[nodiscard]] std::optional<compile_error>
spirit_filter_add_set_color_action_impl(
	lang::color_action_type action_type,
	lang::color_action action,
	lang::action_set& set)
{
	// override previous action
	switch (action_type) {
		case lang::color_action_type::set_border_color: {
			set.set_border_color = action;
			return std::nullopt;
		}
		case lang::color_action_type::set_text_color: {
			set.set_text_color = action;
			return std::nullopt;
		}
		case lang::color_action_type::set_background_color: {
			set.set_background_color = action;
			return std::nullopt;
		}
	}

	return errors::internal_compiler_error{
		errors::internal_compiler_error_cause::spirit_filter_add_set_color_action_impl,
		action.origin};
}

[[nodiscard]] std::optional<compile_error>
spirit_filter_add_set_color_action(
	const ast::sf::set_color_action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set)
{
	std::variant<lang::object, compile_error> obj_or_err = detail::evaluate_sequence(action.seq, symbols, 3, 4);
	if (std::holds_alternative<compile_error>(obj_or_err)) {
		return std::get<compile_error>(std::move(obj_or_err));
	}

	auto& obj = std::get<lang::object>(obj_or_err);

	const auto args_num = static_cast<int>(obj.values.size());
	BOOST_ASSERT(args_num == 3 || args_num == 4);

	std::variant<lang::integer, compile_error> int0_or_err = detail::get_as<lang::integer>(obj.values[0]);
	if (std::holds_alternative<compile_error>(int0_or_err))
		return std::get<compile_error>(std::move(int0_or_err));

	std::variant<lang::integer, compile_error> int1_or_err = detail::get_as<lang::integer>(obj.values[1]);
	if (std::holds_alternative<compile_error>(int1_or_err))
		return std::get<compile_error>(std::move(int1_or_err));

	std::variant<lang::integer, compile_error> int2_or_err = detail::get_as<lang::integer>(obj.values[2]);
	if (std::holds_alternative<compile_error>(int2_or_err))
		return std::get<compile_error>(std::move(int2_or_err));

	const auto r = std::get<lang::integer>(int0_or_err);
	const auto g = std::get<lang::integer>(int1_or_err);
	const auto b = std::get<lang::integer>(int2_or_err);
	std::optional<lang::integer> a;

	if (args_num == 4) {
		std::variant<lang::integer, compile_error> int3_or_err = detail::get_as<lang::integer>(obj.values[3]);
		if (std::holds_alternative<compile_error>(int3_or_err))
			return std::get<compile_error>(std::move(int3_or_err));

		a = std::get<lang::integer>(int3_or_err);
	}

	std::variant<lang::color, compile_error> col_or_err = detail::make_color(r, g, b, a);
	if (std::holds_alternative<compile_error>(col_or_err)) {
		return std::get<compile_error>(col_or_err);
	}

	auto act = lang::color_action{std::get<lang::color>(col_or_err), parser::position_tag_of(action)};
	return spirit_filter_add_set_color_action_impl(action.action_type, act, set);
}

[[nodiscard]] std::optional<compile_error>
spirit_filter_add_set_font_size_action(
	const ast::sf::set_font_size_action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set)
{
	std::variant<lang::object, compile_error> obj_or_err = detail::evaluate_sequence(action.seq, symbols, 1, 1);
	if (std::holds_alternative<compile_error>(obj_or_err)) {
		return std::get<compile_error>(std::move(obj_or_err));
	}

	auto& obj = std::get<lang::object>(obj_or_err);

	BOOST_ASSERT(obj.values.size() == 1u);

	// TODO add erorr handling for font size
	std::variant<lang::integer, compile_error> int_or_err = detail::get_as<lang::integer>(obj.values[0]);
	if (std::holds_alternative<compile_error>(int_or_err))
		return std::get<compile_error>(std::move(int_or_err));

	auto int_obj = std::get<lang::integer>(int_or_err);

	// override previous action
	set.set_font_size = lang::font_size_action{int_obj, parser::position_tag_of(action)};
	return std::nullopt;
}

[[nodiscard]] std::optional<compile_error>
spirit_filter_add_minimap_icon_action(
	const ast::sf::minimap_icon_action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set)
{
	std::variant<lang::object, compile_error> obj_or_err = detail::evaluate_sequence(action.seq, symbols, 3, 3);
	if (std::holds_alternative<compile_error>(obj_or_err)) {
		return std::get<compile_error>(std::move(obj_or_err));
	}

	auto& obj = std::get<lang::object>(obj_or_err);

	BOOST_ASSERT(obj.values.size() == 3u);

	std::variant<lang::integer, compile_error> int_or_err = detail::get_as<lang::integer>(obj.values[0]);
	if (std::holds_alternative<compile_error>(int_or_err))
		return std::get<compile_error>(std::move(int_or_err));

	std::variant<lang::suit, compile_error> suit_or_err = detail::get_as<lang::suit>(obj.values[1]);
	if (std::holds_alternative<compile_error>(suit_or_err))
		return std::get<compile_error>(std::move(suit_or_err));

	std::variant<lang::shape, compile_error> shape_or_err = detail::get_as<lang::shape>(obj.values[2]);
	if (std::holds_alternative<compile_error>(shape_or_err))
		return std::get<compile_error>(std::move(shape_or_err));

	auto size  = std::get<lang::integer>(int_or_err);
	auto suit  = std::get<lang::suit>(suit_or_err);
	auto shape = std::get<lang::shape>(shape_or_err);

	std::variant<lang::minimap_icon, compile_error> icon_or_err =
		detail::make_minimap_icon(size, suit, shape);

	if (std::holds_alternative<compile_error>(icon_or_err)) {
		return std::get<compile_error>(std::move(icon_or_err));
	}

	// override previous action
	set.minimap_icon = lang::minimap_icon_action{
		std::get<lang::minimap_icon>(icon_or_err),
		parser::position_tag_of(action)
	};
	return std::nullopt;
}

[[nodiscard]] std::optional<compile_error>
spirit_filter_add_play_effect_action(
	const ast::sf::play_effect_action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set)
{
	std::variant<lang::object, compile_error> obj_or_err = detail::evaluate_sequence(action.seq, symbols, 1, 2);
	if (std::holds_alternative<compile_error>(obj_or_err)) {
		return std::get<compile_error>(std::move(obj_or_err));
	}

	auto& obj = std::get<lang::object>(obj_or_err);

	const auto args_num = static_cast<int>(obj.values.size());
	BOOST_ASSERT(args_num == 1 || args_num == 2);

	std::variant<lang::suit, compile_error> suit_or_err = detail::get_as<lang::suit>(obj.values[0]);
	if (std::holds_alternative<compile_error>(suit_or_err))
		return std::get<compile_error>(std::move(suit_or_err));

	auto& suit = std::get<lang::suit>(suit_or_err);

	if (args_num == 2) {
		std::variant<lang::temp, compile_error> temp_or_err = detail::get_as<lang::temp>(obj.values[1]);
		if (std::holds_alternative<compile_error>(temp_or_err))
			return std::get<compile_error>(std::move(temp_or_err));

		// override previous action
		set.play_effect = lang::play_effect_action{
			lang::play_effect{suit, true},
			parser::position_tag_of(action)
		};
		return std::nullopt;
	}

	// override previous action
	set.play_effect = lang::play_effect_action{
		lang::play_effect{suit, false},
		parser::position_tag_of(action)
	};
	return std::nullopt;
}

[[nodiscard]] std::variant<lang::builtin_alert_sound, compile_error>
make_builtin_alert_sound(
	bool positional,
	const lang::object& obj)
{
	const auto args_num = static_cast<int>(obj.values.size());
	BOOST_ASSERT(args_num == 1 || args_num == 2);

	std::variant<lang::integer, compile_error> sid_or_err = detail::get_as<lang::integer>(obj.values[0]);
	if (std::holds_alternative<compile_error>(sid_or_err))
		return std::get<compile_error>(std::move(sid_or_err));

	auto sound_id = std::get<lang::integer>(sid_or_err);
	std::optional<lang::integer> volume;

	if (args_num == 2) {
		std::variant<lang::integer, compile_error> vol_or_err = detail::get_as<lang::integer>(obj.values[1]);
		if (std::holds_alternative<compile_error>(vol_or_err))
			return std::get<compile_error>(std::move(vol_or_err));

		volume = std::get<lang::integer>(vol_or_err);
	}

	std::variant<lang::builtin_alert_sound, compile_error> bas_or_err =
		detail::make_builtin_alert_sound(positional, sound_id, volume);

	if (std::holds_alternative<compile_error>(bas_or_err)) {
		return std::get<compile_error>(std::move(bas_or_err));
	}

	return std::get<lang::builtin_alert_sound>(std::move(bas_or_err));
}

[[nodiscard]] std::optional<compile_error>
spirit_filter_add_play_alert_sound_action(
	const ast::sf::play_alert_sound_action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set)
{
	std::variant<lang::object, compile_error> obj_or_err = detail::evaluate_sequence(action.seq, symbols, 1, 2);
	if (std::holds_alternative<compile_error>(obj_or_err)) {
		return std::get<compile_error>(std::move(obj_or_err));
	}

	auto& obj = std::get<lang::object>(obj_or_err);
	std::variant<lang::builtin_alert_sound, compile_error> bas_or_err = make_builtin_alert_sound(action.positional, obj);

	if (std::holds_alternative<compile_error>(bas_or_err)) {
		return std::get<compile_error>(std::move(bas_or_err));
	}

	// override previous action
	set.play_alert_sound = lang::alert_sound_action{
		lang::alert_sound{std::get<lang::builtin_alert_sound>(std::move(bas_or_err))},
		parser::position_tag_of(action)
	};
	return std::nullopt;
}

[[nodiscard]] std::variant<lang::custom_alert_sound, compile_error>
make_custom_alert_sound(
	const lang::single_object& sobj)
{
	std::variant<lang::string, compile_error> str_or_err = detail::get_as<lang::string>(sobj);
	if (std::holds_alternative<compile_error>(str_or_err))
		return std::get<compile_error>(std::move(str_or_err));

	auto& str = std::get<lang::string>(str_or_err);
	return lang::custom_alert_sound{std::move(str)};
}

[[nodiscard]] std::optional<compile_error>
spirit_filter_add_custom_alert_sound_action(
	const ast::sf::custom_alert_sound_action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set)
{
	std::variant<lang::object, compile_error> obj_or_err = detail::evaluate_sequence(action.seq, symbols, 1, 1);
	if (std::holds_alternative<compile_error>(obj_or_err)) {
		return std::get<compile_error>(std::move(obj_or_err));
	}

	auto& obj = std::get<lang::object>(obj_or_err);

	BOOST_ASSERT(obj.values.size() == 1u);

	std::variant<lang::custom_alert_sound, compile_error> alert_or_err = make_custom_alert_sound(obj.values[0]);
	if (std::holds_alternative<compile_error>(alert_or_err))
		return std::get<compile_error>(std::move(alert_or_err));

	set.play_alert_sound = lang::alert_sound_action{
		lang::alert_sound{std::get<lang::custom_alert_sound>(std::move(alert_or_err))},
		parser::position_tag_of(action)
	};
	return std::nullopt;
}

[[nodiscard]] std::optional<compile_error>
spirit_filter_add_set_alert_sound_action(
	const ast::sf::set_alert_sound_action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set)
{
	std::variant<lang::object, compile_error> obj_or_err = detail::evaluate_sequence(action.seq, symbols, 1, 2);
	if (std::holds_alternative<compile_error>(obj_or_err)) {
		return std::get<compile_error>(std::move(obj_or_err));
	}

	auto& obj = std::get<lang::object>(obj_or_err);

	const auto args_num = static_cast<int>(obj.values.size());
	BOOST_ASSERT(args_num == 1 || args_num == 2);

	// 1st: attempt custom alert sound
	// fallthrough if error occured
	if (args_num == 1) {
		std::variant<lang::custom_alert_sound, compile_error> cas_or_err = make_custom_alert_sound(obj.values[0]);
		if (std::holds_alternative<lang::custom_alert_sound>(cas_or_err)) {
			set.play_alert_sound = lang::alert_sound_action{
				lang::alert_sound{std::get<lang::custom_alert_sound>(std::move(cas_or_err))},
				parser::position_tag_of(action)
			};
			return std::nullopt;
		}
	}

	// 2nd: attempt non-positional play alert sound
	std::variant<lang::builtin_alert_sound, compile_error> bas_or_err = make_builtin_alert_sound(false, obj);
	if (std::holds_alternative<lang::builtin_alert_sound>(bas_or_err)) {
		set.play_alert_sound = lang::alert_sound_action{
			lang::alert_sound{std::get<lang::builtin_alert_sound>(std::move(bas_or_err))},
			parser::position_tag_of(action)
		};
		return std::nullopt;
	}

	// TODO improve this error - ideally it should contain both custom and built-in alert sound errors
	// such thing right now is not possible because error storing 2 errors creates a circular definition
	return errors::invalid_set_alert_sound{parser::position_tag_of(action.seq)};
}

[[nodiscard]] std::optional<compile_error>
spirit_filter_add_compound_action(
	const ast::sf::compound_action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set)
{
	std::variant<lang::object, compile_error> obj_or_err = detail::evaluate_sequence(action.seq, symbols, 1, 1);
	if (std::holds_alternative<compile_error>(obj_or_err)) {
		return std::get<compile_error>(std::move(obj_or_err));
	}

	auto& obj = std::get<lang::object>(obj_or_err);

	BOOST_ASSERT(obj.values.size() == 1u);

	std::variant<lang::action_set, compile_error> as_or_err = detail::get_as<lang::action_set>(obj.values[0]);
	if (std::holds_alternative<compile_error>(as_or_err))
		return std::get<compile_error>(std::move(as_or_err));

	auto& as = std::get<lang::action_set>(as_or_err);
	set.override_with(std::move(as));
	return std::nullopt;
}

// ---- real filter helpers ----

template <typename T>
[[nodiscard]] std::optional<compile_error>
real_filter_add_action_impl(
	T&& action,
	std::optional<T>& target)
{
	if (target) {
		return errors::action_redefinition{action.origin, (*target).origin};
	}

	target = std::move(action);
	return std::nullopt;
}

[[nodiscard]] std::optional<compile_error>
real_filter_add_color_action(
	const ast::rf::color_action& action,
	lang::action_set& set)
{
	std::variant<lang::color, compile_error> col_or_err = detail::evaluate(action.color);
	if (std::holds_alternative<compile_error>(col_or_err)) {
		return std::get<compile_error>(std::move(col_or_err));
	}

	auto act = lang::color_action{std::get<lang::color>(col_or_err), parser::position_tag_of(action)};

	switch (action.action) {
		case lang::color_action_type::set_border_color: {
			return real_filter_add_action_impl(std::move(act), set.set_border_color);
		}
		case lang::color_action_type::set_text_color: {
			return real_filter_add_action_impl(std::move(act), set.set_text_color);
		}
		case lang::color_action_type::set_background_color: {
			return real_filter_add_action_impl(std::move(act), set.set_background_color);
		}
	}

	return errors::internal_compiler_error{
		errors::internal_compiler_error_cause::real_filter_add_color_action,
		act.origin};
}

[[nodiscard]] lang::alert_sound
real_filter_make_alert_sound(
	parser::ast::rf::integer_literal sound_id,
	boost::optional<parser::ast::rf::integer_literal> volume,
	bool positional)
{
	if (volume) {
		return lang::alert_sound{lang::builtin_alert_sound{
			positional,
			detail::evaluate(sound_id),
			detail::evaluate(*volume)
		}};
	}
	else {
		return lang::alert_sound{lang::builtin_alert_sound{
			positional,
			detail::evaluate(sound_id)
		}};
	}
}

} // namespace

namespace fs::compiler::detail
{

std::optional<compile_error>
spirit_filter_add_action(
	const ast::sf::action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set)
{
	return action.apply_visitor(x3::make_lambda_visitor<std::optional<compile_error>>(
		[&](const ast::sf::set_color_action& a) {
			return spirit_filter_add_set_color_action(a, symbols, set);
		},
		[&](const ast::sf::set_font_size_action& a) {
			return spirit_filter_add_set_font_size_action(a, symbols, set);
		},
		[&](const ast::sf::minimap_icon_action& a) {
			return spirit_filter_add_minimap_icon_action(a, symbols, set);
		},
		[&](const ast::sf::play_effect_action& a) {
			return spirit_filter_add_play_effect_action(a, symbols, set);
		},
		[&](const ast::sf::play_alert_sound_action& a) {
			return spirit_filter_add_play_alert_sound_action(a, symbols, set);
		},
		[&](const ast::sf::custom_alert_sound_action& a) {
			return spirit_filter_add_custom_alert_sound_action(a, symbols, set);
		},
		[&](const ast::sf::set_alert_sound_action& a) {
			return spirit_filter_add_set_alert_sound_action(a, symbols, set);
		},
		[&](const ast::sf::disable_drop_sound_action&) {
			set.disable_drop_sound = lang::disable_drop_sound_action{parser::position_tag_of(action)};
			return std::nullopt;
		},
		[&](const ast::sf::compound_action& ca) {
			return spirit_filter_add_compound_action(ca, symbols, set);
		}
	));
}

std::optional<compile_error>
real_filter_add_action(
	const parser::ast::rf::action& a,
	lang::action_set& set)
{
	using result_type = std::optional<compile_error>;

	return a.apply_visitor(x3::make_lambda_visitor<result_type>(
		[&](const ast::rf::color_action& action) {
			return real_filter_add_color_action(action, set);
		},
		[&](const ast::rf::set_font_size_action& action) -> result_type {
			return real_filter_add_action_impl(
				lang::font_size_action{
					lang::integer{action.font_size.value},
					parser::position_tag_of(action)
				},
				set.set_font_size);
		},
		[&](const ast::rf::minimap_icon_action& action) -> result_type {
			std::variant<lang::minimap_icon, compile_error> icon_or_error = evaluate(action.icon);
			if (std::holds_alternative<compile_error>(icon_or_error))
				return std::get<compile_error>(std::move(icon_or_error));

			return real_filter_add_action_impl(
				lang::minimap_icon_action{
					std::get<lang::minimap_icon>(icon_or_error),
					parser::position_tag_of(action)},
				set.minimap_icon);
		},
		[&](const ast::rf::play_effect_action& action) -> result_type {
			return real_filter_add_action_impl(
				lang::play_effect_action{
					lang::play_effect{evaluate(action.suit), action.is_temporary},
					parser::position_tag_of(action)
				},
				set.play_effect);
		},
		[&](const ast::rf::play_alert_sound_action& action) -> result_type {
			return real_filter_add_action_impl(
				lang::alert_sound_action{
					real_filter_make_alert_sound(action.sound_id, action.volume, false),
					parser::position_tag_of(action)
				},
				set.play_alert_sound);
		},
		[&](const ast::rf::play_alert_sound_positional_action& action) -> result_type {
			return real_filter_add_action_impl(
				lang::alert_sound_action{
					real_filter_make_alert_sound(action.sound_id, action.volume, true),
					parser::position_tag_of(action)
				},
				set.play_alert_sound);
		},
		[&](const ast::rf::custom_alert_sound_action& action) -> result_type {
			return real_filter_add_action_impl(
				lang::alert_sound_action{
					lang::alert_sound{lang::custom_alert_sound{evaluate(action.path)}},
					parser::position_tag_of(action)
				},
				set.play_alert_sound);
		},
		[&](const ast::rf::disable_drop_sound_action& action) -> result_type {
			return real_filter_add_action_impl(
				lang::disable_drop_sound_action{parser::position_tag_of(action)},
				set.disable_drop_sound);
		}
	));
}

} // namespace fs::compiler::detail
