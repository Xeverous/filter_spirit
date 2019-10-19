#include <fs/compiler/detail/actions.hpp>
#include <fs/compiler/detail/evaluate_as.hpp>

#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>

#include <utility>

namespace ast = fs::parser::ast;
namespace x3 = boost::spirit::x3;

namespace
{

using namespace fs;
using namespace fs::compiler;

[[nodiscard]] std::optional<compile_error>
add_unary_action(
	const ast::unary_action& action,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data,
	lang::action_set& action_set)
{
	switch (action.action_type)
	{
		case lang::unary_action_type::set_border_color:
		{
			std::variant<lang::color, compile_error> color_or_error = detail::evaluate_as<lang::color>(action.value, symbols, item_price_data);
			if (std::holds_alternative<compile_error>(color_or_error))
				return std::get<compile_error>(std::move(color_or_error));

			action_set.override_border_color(std::get<lang::color>(color_or_error));
			return std::nullopt;
		}
		case lang::unary_action_type::set_text_color:
		{
			std::variant<lang::color, compile_error> color_or_error = detail::evaluate_as<lang::color>(action.value, symbols, item_price_data);
			if (std::holds_alternative<compile_error>(color_or_error))
				return std::get<compile_error>(std::move(color_or_error));

			action_set.override_text_color(std::get<lang::color>(color_or_error));
			return std::nullopt;
		}
		case lang::unary_action_type::set_background_color:
		{
			std::variant<lang::color, compile_error> color_or_error = detail::evaluate_as<lang::color>(action.value, symbols, item_price_data);
			if (std::holds_alternative<compile_error>(color_or_error))
				return std::get<compile_error>(std::move(color_or_error));

			action_set.override_background_color(std::get<lang::color>(color_or_error));
			return std::nullopt;
		}
		case lang::unary_action_type::set_font_size:
		{
			std::variant<lang::font_size, compile_error> font_size_or_error = detail::evaluate_as<lang::font_size>(action.value, symbols, item_price_data);
			if (std::holds_alternative<compile_error>(font_size_or_error))
				return std::get<compile_error>(std::move(font_size_or_error));

			action_set.override_font_size(std::get<lang::font_size>(font_size_or_error));
			return std::nullopt;
		}
		case lang::unary_action_type::set_alert_sound:
		{
			std::variant<lang::alert_sound, compile_error> alert_or_error = detail::evaluate_as<lang::alert_sound>(action.value, symbols, item_price_data);
			if (std::holds_alternative<compile_error>(alert_or_error))
				return std::get<compile_error>(std::move(alert_or_error));

			action_set.override_alert_sound(std::get<lang::alert_sound>(std::move(alert_or_error)));
			return std::nullopt;
		}
		case lang::unary_action_type::play_default_drop_sound:
		{
			std::variant<lang::boolean, compile_error> bool_or_error = detail::evaluate_as<lang::boolean>(action.value, symbols, item_price_data);
			if (std::holds_alternative<compile_error>(bool_or_error))
				return std::get<compile_error>(std::move(bool_or_error));

			const bool enable = std::get<lang::boolean>(bool_or_error).value;
			action_set.override_play_default_drop_sound(enable);
			return std::nullopt;
		}
		case lang::unary_action_type::set_minimap_icon:
		{
			std::variant<lang::minimap_icon, compile_error> icon_or_error = detail::evaluate_as<lang::minimap_icon>(action.value, symbols, item_price_data);
			if (std::holds_alternative<compile_error>(icon_or_error))
				return std::get<compile_error>(std::move(icon_or_error));

			action_set.override_minimap_icon(std::get<lang::minimap_icon>(icon_or_error));
			return std::nullopt;
		}
		case lang::unary_action_type::set_beam:
		{
			std::variant<lang::beam_effect, compile_error> beam_or_error = detail::evaluate_as<lang::beam_effect>(action.value, symbols, item_price_data);
			if (std::holds_alternative<compile_error>(beam_or_error))
				return std::get<compile_error>(std::move(beam_or_error));

			action_set.override_beam_effect(std::get<lang::beam_effect>(beam_or_error));
			return std::nullopt;
		}
		default:
		{
			return errors::internal_compiler_error_during_action_evaluation{parser::get_position_info(action)};
		}
	}
}

} // namespace

namespace fs::compiler::detail
{

std::optional<compile_error>
add_action(
	const ast::action& action,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data,
	lang::action_set& action_set)
{
	return add_unary_action(action.action, symbols, item_price_data, action_set);
}

}
