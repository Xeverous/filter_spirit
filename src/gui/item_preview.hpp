#pragma once

#include <fs/lang/item.hpp>

#include <elements/support.hpp>
#include <elements/element.hpp>

#include <string>

class item_preview : public cycfi::elements::element
{
public:
	item_preview(
		std::string name,
		fs::lang::socket_info sockets,
		int width,
		int height,
		int font_size,
		cycfi::elements::color border_color,
		cycfi::elements::color text_color,
		cycfi::elements::color background_color)
	: _name(std::move(name))
	, _sockets(sockets)
	, _width(width)
	, _height(height)
	, _font_size(font_size)
	, _border_color(border_color)
	, _text_color(text_color)
	, _background_color(background_color)
	{}

	cycfi::elements::view_limits limits(cycfi::elements::basic_context const& ctx) const override;
	void draw(cycfi::elements::context const& ctx) override;

private:
	// item properties affecting preview
	/*
	 * width/height affects how links are drawn
	 *
	 * 1x4 item vs 2x2 item, both with 4L:
	 *
	 *   O
	 *   |    O---O
	 *   O        |
	 *   |    O---O
	 *   O
	 *   |
	 *   O
	 *
	 */
	std::string _name;
	fs::lang::socket_info _sockets;
	int _width;
	int _height;

	// item style applied by the filter
	int _font_size;
	cycfi::elements::color _border_color;
	cycfi::elements::color _text_color;
	cycfi::elements::color _background_color;
};
