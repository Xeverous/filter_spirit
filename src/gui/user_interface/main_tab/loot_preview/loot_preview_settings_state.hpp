#pragma once

#include "user_interface/generic/slider.hpp"

#include <elements/element/element.hpp>

class loot_preview_settings_state
{
public:
	loot_preview_settings_state();

	std::shared_ptr<cycfi::elements::element> ui()
	{
		return _root_element;
	}

	int rarity() const { return _rarity; }
	int quantity() const { return _quantity; }

private:
	void make_ui();
	void rarity_changed(double val);
	void quantity_changed(double val);

	slider _slider_rarity;
	slider _slider_quantity;
	int _rarity;
	int _quantity;

	std::shared_ptr<cycfi::elements::element> _root_element;
	std::shared_ptr<cycfi::elements::basic_label> _label_rarity;
	std::shared_ptr<cycfi::elements::basic_label> _label_quantity;
};
