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
	ctx.canvas.line_width(2.0);
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

el::color to_color(fs::lang::socket_color c)
{
	using color = fs::lang::socket_color;

	if (c == color::a)
		return el::colors::black;
	if (c == color::r)
		return el::colors::red;
	if (c == color::g)
		return el::colors::green;
	if (c == color::b)
		return el::colors::blue;
	if (c == color::w)
		return el::colors::white;

	return el::colors::white;
}

void draw_sockets(const el::context& ctx, int width, int height, const fs::lang::socket_info& sockets)
{
	// first, calculate top left corner of the socket drawing
	const auto cx = ctx.bounds.right - (width == 2 ? 15 : 8);

	const int sockets_vertically = [&]() {
		if (width == 1)
			return sockets.sockets();
		else
			return (sockets.sockets() + 1) / 2;
	}();

	const auto cy = ctx.bounds.top + (ctx.bounds.height() / 2);

	constexpr auto socket_box_size_x = 4;
	constexpr auto socket_box_size_y = 4;

	el::rect pos(cx, cy, cx + socket_box_size_x, cy + socket_box_size_y);

	// first dirty sample rendering, just go straight downwards
	for (const fs::lang::linked_sockets& group : sockets.groups) {
		for (fs::lang::socket_color c : group.sockets) {
			ctx.canvas.fill_style(to_color(c));
			ctx.canvas.stroke_rect(pos);
			pos = pos.move(0, socket_box_size_y + 3);
		}
	}
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
	draw_sockets(ctx, _width, _height, _sockets);
}
