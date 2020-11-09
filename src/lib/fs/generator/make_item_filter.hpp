#pragma once

#include <fs/lang/item_filter.hpp>
#include <fs/lang/market/item_price_data.hpp>

namespace fs::generator
{

/**
 * @brief Transform spirit filter template into an real filter representation.
 * Basically filter_template + item_price_data => real_filter_representation
 *
 * @details This function should never fail. Will throw on invalid inputs.
 *
 * @param filter_template spirit filter (which contains various extensions)
 * @param item_price_data data to use in autogeneration extension
 * @return
 */
[[nodiscard]] lang::item_filter
make_item_filter(
	const lang::spirit_item_filter& filter_template,
	const lang::market::item_price_data& item_price_data);

}
