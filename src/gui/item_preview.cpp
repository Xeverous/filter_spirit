#include "item_preview.hpp"

#include <elements/support.hpp>
#include <elements/element.hpp>

namespace el = cycfi::elements;

namespace {

void draw_background(const el::context& ctx, el::color color)
{
	ctx.canvas.fill_style(color);
	ctx.canvas.fill_rect(ctx.bounds);
}

void draw_border(const el::context& ctx, el::color color)
{
	ctx.canvas.line_width(1.0);
	ctx.canvas.stroke_style(color);
	ctx.canvas.stroke_rect(ctx.bounds);
}

void draw_text(const el::context& ctx, el::color color, int font_size, const char* c_str)
{
	ctx.canvas.fill_style(color);
	ctx.canvas.font(el::get_theme().label_font, font_size);

	const auto cx = ctx.bounds.left;
	const auto cy = ctx.bounds.top + (ctx.bounds.height() / 2);

	ctx.canvas.text_align(el::canvas::left);
	ctx.canvas.fill_text({ cx, cy }, c_str);
}

}

el::view_limits item_preview::limits(const cycfi::elements::basic_context& ctx) const
{
	auto size = el::measure_text(ctx.canvas, _name.c_str(), el::get_theme().label_font, _font_size);
	return { { size.x + 10, size.y + 10 }, { size.x + 10, size.y + 10 } };
}

void item_preview::draw(const el::context& ctx)
{
	draw_background(ctx, _background_color);
	draw_border(ctx, _border_color);
	draw_text(ctx, _text_color, _font_size, _name.c_str());
}
