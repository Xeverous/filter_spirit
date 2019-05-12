#include "fs/compiler/detail/actions.hpp"
#include "fs/compiler/detail/evaluate_as.hpp"

#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>

#include <utility>

namespace ast = fs::parser::ast;
namespace x3 = boost::spirit::x3;

namespace
{

using namespace fs;
using namespace fs::compiler;

[[nodiscard]]
std::optional<error::error_variant> add_unary_action(
	const ast::unary_action& action,
	const lang::constants_map& map,
	const lang::item_price_data& item_price_data,
	lang::action_set& action_set)
{
	switch (action.action_type)
	{
		case lang::unary_action_type::set_border_color:
		{
			std::variant<lang::color, error::error_variant> color_or_error = detail::evaluate_as<lang::color>(action.value, map, item_price_data);
			if (std::holds_alternative<error::error_variant>(color_or_error))
				return std::get<error::error_variant>(std::move(color_or_error));

			action_set.override_border_color(std::get<lang::color>(color_or_error));
			return std::nullopt;
		}
		case lang::unary_action_type::set_text_color:
		{
			std::variant<lang::color, error::error_variant> color_or_error = detail::evaluate_as<lang::color>(action.value, map, item_price_data);
			if (std::holds_alternative<error::error_variant>(color_or_error))
				return std::get<error::error_variant>(std::move(color_or_error));

			action_set.override_text_color(std::get<lang::color>(color_or_error));
			return std::nullopt;
		}
		case lang::unary_action_type::set_background_color:
		{
			std::variant<lang::color, error::error_variant> color_or_error = detail::evaluate_as<lang::color>(action.value, map, item_price_data);
			if (std::holds_alternative<error::error_variant>(color_or_error))
				return std::get<error::error_variant>(std::move(color_or_error));

			action_set.override_background_color(std::get<lang::color>(color_or_error));
			return std::nullopt;
		}
		case lang::unary_action_type::set_font_size:
		{
			std::variant<lang::font_size, error::error_variant> font_size_or_error = detail::evaluate_as<lang::font_size>(action.value, map, item_price_data);
			if (std::holds_alternative<error::error_variant>(font_size_or_error))
				return std::get<error::error_variant>(std::move(font_size_or_error));

			action_set.override_font_size(std::get<lang::font_size>(font_size_or_error));
			return std::nullopt;
		}
		case lang::unary_action_type::set_alert_sound:
		{
			std::variant<lang::alert_sound, error::error_variant> alert_or_error = detail::evaluate_as<lang::alert_sound>(action.value, map, item_price_data);
			if (std::holds_alternative<error::error_variant>(alert_or_error))
				return std::get<error::error_variant>(std::move(alert_or_error));

			action_set.override_alert_sound(std::get<lang::alert_sound>(std::move(alert_or_error)));
			return std::nullopt;
		}
		case lang::unary_action_type::set_minimap_icon:
		{
			std::variant<lang::minimap_icon, error::error_variant> icon_or_error = detail::evaluate_as<lang::minimap_icon>(action.value, map, item_price_data);
			if (std::holds_alternative<error::error_variant>(icon_or_error))
				return std::get<error::error_variant>(std::move(icon_or_error));

			action_set.override_minimap_icon(std::get<lang::minimap_icon>(icon_or_error));
			return std::nullopt;
		}
		case lang::unary_action_type::set_beam:
		{
			std::variant<lang::beam_effect, error::error_variant> beam_or_error = detail::evaluate_as<lang::beam_effect>(action.value, map, item_price_data);
			if (std::holds_alternative<error::error_variant>(beam_or_error))
				return std::get<error::error_variant>(std::move(beam_or_error));

			action_set.override_beam_effect(std::get<lang::beam_effect>(beam_or_error));
			return std::nullopt;
		}
		default:
		{
			return error::internal_compiler_error_during_action_evaluation{parser::get_position_info(action)};
		}
	}
}

[[nodiscard]]
std::optional<error::error_variant> add_nullary_action(
	const ast::nullary_action& action,
	lang::action_set& action_set)
{
	switch (action.action_type)
	{
		case lang::nullary_action_type::enable_drop_sound:
		{
			action_set.enable_drop_sound();
			return std::nullopt;
		}
		case lang::nullary_action_type::disable_drop_sound:
		{
			action_set.disable_drop_sound();
			return std::nullopt;
		}
		default:
		{
			return error::internal_compiler_error_during_action_evaluation{parser::get_position_info(action)};
		}
	}
}

} // namespace

namespace fs::compiler::detail
{

std::optional<error::error_variant> add_action(
	const ast::action& action,
	const lang::constants_map& map,
	const lang::item_price_data& item_price_data,
	lang::action_set& action_set)
{
	return action.apply_visitor(x3::make_lambda_visitor<std::optional<error::error_variant>>(
		[&](const ast::nullary_action& nullary_action)
		{
			return add_nullary_action(nullary_action, action_set);
		},
		[&](const ast::unary_action& unary_action)
		{
			return add_unary_action(unary_action, map, item_price_data, action_set);
		}));
}

}
