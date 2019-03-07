#include "compiler/actions.hpp"
#include "compiler/evaluate_as.hpp"

namespace fs::compiler
{

namespace ast = parser::ast;
namespace x3 = boost::spirit::x3;

std::optional<error::error_variant> add_action(
	const ast::action& action,
	const lang::constants_map& map,
	const itemdata::item_price_data& item_price_data,
	lang::action_set& action_set)
{
	switch (action.action_type)
	{
		case lang::action_type::set_border_color:
		{
			std::variant<lang::color, error::error_variant> color_or_error = evaluate_as<lang::color>(action.value, map, item_price_data);
			if (std::holds_alternative<error::error_variant>(color_or_error))
				return std::get<error::error_variant>(color_or_error);

			action_set.override_border_color(std::get<lang::color>(color_or_error));
			return std::nullopt;
		}
		case lang::action_type::set_text_color:
		{
			std::variant<lang::color, error::error_variant> color_or_error = evaluate_as<lang::color>(action.value, map, item_price_data);
			if (std::holds_alternative<error::error_variant>(color_or_error))
				return std::get<error::error_variant>(color_or_error);

			action_set.override_text_color(std::get<lang::color>(color_or_error));
			return std::nullopt;
		}
		case lang::action_type::set_background_color:
		{
			std::variant<lang::color, error::error_variant> color_or_error = evaluate_as<lang::color>(action.value, map, item_price_data);
			if (std::holds_alternative<error::error_variant>(color_or_error))
				return std::get<error::error_variant>(color_or_error);

			action_set.override_background_color(std::get<lang::color>(color_or_error));
			return std::nullopt;
		}
		case lang::action_type::set_font_size:
		{
			std::variant<lang::font_size, error::error_variant> font_size_or_error = evaluate_as<lang::font_size>(action.value, map, item_price_data);
			if (std::holds_alternative<error::error_variant>(font_size_or_error))
				return std::get<error::error_variant>(font_size_or_error);

			action_set.override_font_size(std::get<lang::font_size>(font_size_or_error));
			return std::nullopt;
		}
		case lang::action_type::set_alert_sound:
		{
			std::variant<lang::alert_sound, error::error_variant> alert_or_error = evaluate_as<lang::alert_sound>(action.value, map, item_price_data);
			if (std::holds_alternative<error::error_variant>(alert_or_error))
				return std::get<error::error_variant>(alert_or_error);

			action_set.override_alert_sound(std::get<lang::alert_sound>(std::move(alert_or_error)));
			return std::nullopt;
		}
		case lang::action_type::set_alert_sound_positional:
		{
			return error::positional_sound_not_supported{parser::get_position_info(action)};
		}
		case lang::action_type::disable_drop_sound:
		{
			return error::disable_drop_sound_not_supported{parser::get_position_info(action)};
		}
		case lang::action_type::set_minimap_icon:
		{
			std::variant<lang::minimap_icon, error::error_variant> icon_or_error = evaluate_as<lang::minimap_icon>(action.value, map, item_price_data);
			if (std::holds_alternative<error::error_variant>(icon_or_error))
				return std::get<error::error_variant>(icon_or_error);

			action_set.override_minimap_icon(std::get<lang::minimap_icon>(icon_or_error));
			return std::nullopt;
		}
		case lang::action_type::set_beam:
		{
			std::variant<lang::beam_effect, error::error_variant> beam_or_error = evaluate_as<lang::beam_effect>(action.value, map, item_price_data);
			if (std::holds_alternative<error::error_variant>(beam_or_error))
				return std::get<error::error_variant>(beam_or_error);

			action_set.override_beam_effect(std::get<lang::beam_effect>(beam_or_error));
			return std::nullopt;
		}
		default:
		{
			return error::internal_compiler_error_during_action_evaluation{parser::get_position_info(action)};
		}
	}
}

}
