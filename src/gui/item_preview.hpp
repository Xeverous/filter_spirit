#pragma once

#include <elements/support.hpp>
#include <elements/element.hpp>

class item_preview : public cycfi::elements::element
{
public:
	item_preview(std::string name, cycfi::elements::color text_color, cycfi::elements::color border_color, cycfi::elements::color background_color)
	: _name(std::move(name)), _text_color(text_color), _border_color(border_color), _background_color(background_color)
	{}

	cycfi::elements::view_limits limits(cycfi::elements::basic_context const& ctx) const override;
	void draw(cycfi::elements::context const& ctx) override;

private:
	std::string _name;
	int _font_size = 18;
	cycfi::elements::color _text_color;
	cycfi::elements::color _border_color;
	cycfi::elements::color _background_color;
};