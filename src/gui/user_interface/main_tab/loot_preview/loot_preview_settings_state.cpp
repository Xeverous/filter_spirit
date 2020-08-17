#include "user_interface/main_tab/loot_preview/loot_preview_settings_state.hpp"

#include <fs/utility/math.hpp>

#include <elements/element.hpp>

#include <cmath>

namespace el = cycfi::elements;

namespace {

constexpr double min_magic_find = 0;
constexpr double max_magic_find = 500;

constexpr int initial_rarity = 100;
constexpr int initial_quantity = 100;

constexpr double normalize_magic_find_value(double value)
{
	return fs::utility::inverse_lerp(value, min_magic_find, max_magic_find);
}

constexpr double calculate_magic_find_value(double normalized)
{
	return fs::utility::lerp(normalized, min_magic_find, max_magic_find);
}

std::string magic_find_to_string(int value)
{
	return std::to_string(value) + "%";
}

void update_magic_find_state(double normalized, int& variable, el::basic_label& label)
{
	variable = static_cast<int>(std::round(calculate_magic_find_value(normalized)));
	label.set_text(magic_find_to_string(variable));
}

}

loot_preview_settings_state::loot_preview_settings_state()
: _slider_rarity(normalize_magic_find_value(initial_rarity), [this](double val) { rarity_changed(val); })
, _slider_quantity(normalize_magic_find_value(initial_quantity), [this](double val) { quantity_changed(val); })
, _rarity(initial_rarity)
, _quantity(initial_quantity)
{
	make_ui();
}

void loot_preview_settings_state::make_ui()
{
	_label_rarity = el::share(el::label(magic_find_to_string(initial_rarity)));
	_label_quantity = el::share(el::label(magic_find_to_string(initial_quantity)));

	_root_element = el::share(el::no_vstretch(el::htile(
		el::no_hstretch(el::vtile(
			el::align_center_middle(el::label("rarity")),
			el::align_center_middle(el::label("quantity"))
		)),
		el::vtile(
			el::hold(_slider_rarity.ui()),
			el::hold(_slider_quantity.ui())
		),
		el::no_hstretch(el::vtile(
			el::align_center_middle(el::hold(_label_rarity)),
			el::align_center_middle(el::hold(_label_quantity))
		))
	)));
}

void loot_preview_settings_state::rarity_changed(double val)
{
	update_magic_find_state(val, _rarity, *_label_rarity);
}

void loot_preview_settings_state::quantity_changed(double val)
{
	update_magic_find_state(val, _quantity, *_label_quantity);
}
