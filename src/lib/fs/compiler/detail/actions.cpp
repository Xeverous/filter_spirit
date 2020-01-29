#include <fs/compiler/detail/evaluate_as.hpp>
#include <fs/compiler/detail/actions.hpp>

#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>

#include <utility>

namespace ast = fs::parser::ast;
namespace x3 = boost::spirit::x3;

namespace
{

using namespace fs;
using namespace fs::compiler;

// ---- spirit_filter_add_action helper ----

[[nodiscard]] std::optional<compile_error>
spirit_filter_add_unary_action(
	const ast::sf::unary_action& action,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data,
	lang::action_set& action_set)
{
	const auto origin = parser::get_position_info(action);

	switch (action.action_type) {
		case lang::unary_action_type::set_border_color: {
			std::variant<lang::color, compile_error> color_or_error =
				detail::evaluate_as<lang::color>(action.value, symbols, item_price_data);

			if (std::holds_alternative<compile_error>(color_or_error))
				return std::get<compile_error>(std::move(color_or_error));

			action_set.set_border_color = lang::color_action{std::get<lang::color>(color_or_error), origin};
			return std::nullopt;
		}
		case lang::unary_action_type::set_text_color: {
			std::variant<lang::color, compile_error> color_or_error =
				detail::evaluate_as<lang::color>(action.value, symbols, item_price_data);

			if (std::holds_alternative<compile_error>(color_or_error))
				return std::get<compile_error>(std::move(color_or_error));

			action_set.set_text_color = lang::color_action{std::get<lang::color>(color_or_error), origin};
			return std::nullopt;
		}
		case lang::unary_action_type::set_background_color: {
			std::variant<lang::color, compile_error> color_or_error =
				detail::evaluate_as<lang::color>(action.value, symbols, item_price_data);

			if (std::holds_alternative<compile_error>(color_or_error))
				return std::get<compile_error>(std::move(color_or_error));

			action_set.set_background_color = lang::color_action{std::get<lang::color>(color_or_error), origin};
			return std::nullopt;
		}
		case lang::unary_action_type::set_font_size: {
			std::variant<lang::font_size, compile_error> font_size_or_error =
				detail::evaluate_as<lang::font_size>(action.value, symbols, item_price_data);

			if (std::holds_alternative<compile_error>(font_size_or_error))
				return std::get<compile_error>(std::move(font_size_or_error));

			action_set.set_font_size = lang::font_size_action{(std::get<lang::font_size>(font_size_or_error)), origin};
			return std::nullopt;
		}
		case lang::unary_action_type::set_alert_sound: {
			std::variant<lang::alert_sound, compile_error> alert_or_error =
				detail::evaluate_as<lang::alert_sound>(action.value, symbols, item_price_data);

			if (std::holds_alternative<compile_error>(alert_or_error))
				return std::get<compile_error>(std::move(alert_or_error));

			action_set.set_alert_sound = lang::alert_sound_action{(std::get<lang::alert_sound>(std::move(alert_or_error))), origin};
			return std::nullopt;
		}
		case lang::unary_action_type::play_default_drop_sound: {
			std::variant<lang::boolean, compile_error> bool_or_error =
				detail::evaluate_as<lang::boolean>(action.value, symbols, item_price_data);

			if (std::holds_alternative<compile_error>(bool_or_error))
				return std::get<compile_error>(std::move(bool_or_error));

			if (std::get<lang::boolean>(bool_or_error).value)
				action_set.disable_default_drop_sound = std::nullopt;
			else
				action_set.disable_default_drop_sound = lang::disable_default_drop_sound_action{origin};

			return std::nullopt;
		}
		case lang::unary_action_type::set_minimap_icon: {
			std::variant<lang::minimap_icon, compile_error> icon_or_error =
				detail::evaluate_as<lang::minimap_icon>(action.value, symbols, item_price_data);

			if (std::holds_alternative<compile_error>(icon_or_error))
				return std::get<compile_error>(std::move(icon_or_error));

			action_set.set_minimap_icon = lang::minimap_icon_action{(std::get<lang::minimap_icon>(icon_or_error)), origin};
			return std::nullopt;
		}
		case lang::unary_action_type::set_beam: {
			std::variant<lang::beam_effect, compile_error> beam_or_error =
				detail::evaluate_as<lang::beam_effect>(action.value, symbols, item_price_data);

			if (std::holds_alternative<compile_error>(beam_or_error))
				return std::get<compile_error>(std::move(beam_or_error));

			action_set.set_beam_effect = lang::beam_effect_action{(std::get<lang::beam_effect>(beam_or_error)), origin};
			return std::nullopt;
		}
	}

	return errors::internal_compiler_error{
		errors::internal_compiler_error_cause::spirit_filter_add_unary_action,
		parser::get_position_info(action)
	};
}

[[nodiscard]] std::optional<compile_error>
spirit_filter_add_compound_action(
	const ast::sf::compound_action& ca,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data,
	lang::action_set& action_set)
{
	std::variant<lang::action_set, compile_error> actions_or_error =
		detail::evaluate_as<lang::action_set>(ca.value, symbols, item_price_data);

	if (std::holds_alternative<compile_error>(actions_or_error))
		return std::get<compile_error>(std::move(actions_or_error));

	action_set.override_with(std::get<lang::action_set>(std::move(actions_or_error)));
	return std::nullopt;
}

// ---- real_filter_add_action helper ----

[[nodiscard]] std::optional<compile_error>
real_filter_add_color_action(
	const ast::rf::color_action& action,
	lang::action_set& action_set)
{
	const auto origin = parser::get_position_info(action);
	const lang::color_action act{detail::evaluate(action.color), origin};

	switch (action.action) {
		case lang::color_action_type::set_border_color: {
			if (action_set.set_border_color) {
				return errors::action_redefinition{origin, (*action_set.set_border_color).origin};
			}

			action_set.set_border_color = act;
			return std::nullopt;
		}
		case lang::color_action_type::set_text_color: {
			if (action_set.set_text_color) {
				return errors::action_redefinition{origin, (*action_set.set_text_color).origin};
			}

			action_set.set_text_color = act;
			return std::nullopt;
		}
		case lang::color_action_type::set_background_color: {
			if (action_set.set_background_color) {
				return errors::action_redefinition{origin, (*action_set.set_background_color).origin};
			}

			action_set.set_background_color = act;
			return std::nullopt;
		}
	}

	return errors::internal_compiler_error{
		errors::internal_compiler_error_cause::real_filter_add_color_action,
		origin};
}

[[nodiscard]] lang::alert_sound
real_filter_make_alert_sound(
	parser::ast::rf::integer_literal sound_id,
	boost::optional<parser::ast::rf::integer_literal> volume,
	bool positional)
{
	if (volume) {
		return lang::alert_sound{lang::built_in_alert_sound{
			lang::sound_id{detail::evaluate(sound_id)},
			lang::volume{lang::integer{detail::evaluate(*volume)}},
			lang::boolean{positional},
		}};
	}
	else {
		return lang::alert_sound{lang::built_in_alert_sound{
			lang::sound_id{detail::evaluate(sound_id)},
			lang::boolean{positional}
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
	const lang::item_price_data& item_price_data,
	lang::action_set& action_set)
{
	return action.apply_visitor(x3::make_lambda_visitor<std::optional<compile_error>>(
		[&](const ast::sf::unary_action& ua) {
			return spirit_filter_add_unary_action(ua, symbols, item_price_data, action_set);
		},
		[&](const ast::sf::compound_action& ca) {
			return spirit_filter_add_compound_action(ca, symbols, item_price_data, action_set);
		}
	));
}

std::optional<compile_error>
real_filter_add_action(
	const parser::ast::rf::action& a,
	lang::action_set& action_set)
{
	using result_type = std::optional<compile_error>;

	return a.apply_visitor(x3::make_lambda_visitor<result_type>(
		[&](const ast::rf::color_action& action) {
			return real_filter_add_color_action(action, action_set);
		},
		[&](const ast::rf::set_font_size_action& action) -> result_type {
			const auto origin = parser::get_position_info(action);

			if (action_set.set_font_size) {
				return errors::action_redefinition{origin, (*action_set.set_font_size).origin};
			}

			action_set.set_font_size = lang::font_size_action{lang::font_size{action.font_size.value}, origin};
			return std::nullopt;
		},
		[&](const ast::rf::play_alert_sound_action& action) -> result_type {
			const auto origin = parser::get_position_info(action);

			if (action_set.set_alert_sound) {
				return errors::action_redefinition{origin, (*action_set.set_alert_sound).origin};
			}

			action_set.set_alert_sound = lang::alert_sound_action{
				real_filter_make_alert_sound(action.sound_id, action.volume, false),
				origin};
			return std::nullopt;
		},
		[&](const ast::rf::play_alert_sound_positional_action& action) -> result_type {
			const auto origin = parser::get_position_info(action);

			if (action_set.set_alert_sound) {
				return errors::action_redefinition{origin, (*action_set.set_alert_sound).origin};
			}

			action_set.set_alert_sound = lang::alert_sound_action{
				real_filter_make_alert_sound(action.sound_id, action.volume, true),
				origin};
			return std::nullopt;
		},
		[&](const ast::rf::custom_alert_sound_action& action) -> result_type {
			const auto origin = parser::get_position_info(action);

			if (action_set.set_alert_sound) {
				return errors::action_redefinition{origin, (*action_set.set_alert_sound).origin};
			}

			action_set.set_alert_sound = lang::alert_sound_action{
				lang::alert_sound{lang::custom_alert_sound{lang::path{evaluate(action.path)}}},
				origin};
			return std::nullopt;
		},
		[&](const ast::rf::disable_drop_sound_action& action) -> result_type {
			const auto origin = parser::get_position_info(action);

			if (action_set.disable_default_drop_sound) {
				return errors::action_redefinition{origin, (*action_set.disable_default_drop_sound).origin};
			}

			action_set.disable_default_drop_sound = lang::disable_default_drop_sound_action{origin};
			return std::nullopt;
		},
		[&](const ast::rf::minimap_icon_action& action) -> result_type {
			const auto origin = parser::get_position_info(action);

			if (action_set.set_minimap_icon) {
				return errors::action_redefinition{origin, (*action_set.set_minimap_icon).origin};
			}

			std::variant<lang::minimap_icon, compile_error> icon_or_error = evaluate(action.icon);
			if (std::holds_alternative<compile_error>(icon_or_error))
				return std::get<compile_error>(std::move(icon_or_error));

			auto& icon = std::get<lang::minimap_icon>(icon_or_error);
			action_set.set_minimap_icon = lang::minimap_icon_action{icon, origin};
			return std::nullopt;
		},
		[&](const ast::rf::play_effect_action& action) -> result_type {
			const auto origin = parser::get_position_info(action);

			if (action_set.set_beam_effect) {
				return errors::action_redefinition{origin, (*action_set.set_beam_effect).origin};
			}

			action_set.set_beam_effect = lang::beam_effect_action{
				lang::beam_effect{evaluate(action.suit), lang::boolean{action.is_temporary}},
				origin};
			return std::nullopt;
		}
	));
}

}
