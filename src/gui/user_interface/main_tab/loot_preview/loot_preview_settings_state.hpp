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

	double rarity() const { return normalize_mf_bonus(_rarity); }
	double quantity() const { return normalize_mf_bonus(_quantity); }

	static double normalize_mf_bonus(int val)
	{
		return val / 100.0;
	}

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
