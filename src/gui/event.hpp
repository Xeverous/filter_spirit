#pragma once

#include <variant>
#include <queue>
#include <utility>

namespace events {

struct load_item_database {};

struct filter_template_browse_requested {};
struct filter_template_path_changed{};
struct filter_template_changed {};

struct spirit_filter_changed {};
struct real_filter_changed {};

struct league_selection_changed {};
struct api_selection_changed {};

struct refresh_available_leagues {};
struct refresh_market_data {};
struct refresh_loot_preview {};

struct price_report_changed {};

}

using event_t = std::variant<
	events::load_item_database,
	events::filter_template_browse_requested,
	events::filter_template_path_changed,
	events::filter_template_changed,
	events::spirit_filter_changed,
	events::real_filter_changed,
	events::league_selection_changed,
	events::api_selection_changed,
	events::refresh_available_leagues,
	events::refresh_market_data,
	events::refresh_loot_preview,
	events::price_report_changed
>;

class event_inserter
{
public:
	event_inserter(std::queue<event_t>& event_queue)
	: _event_queue(event_queue)
	{
	}

	void push_event(const event_t& e)
	{
		_event_queue.push(e);
	}

	void push_event(event_t&& e)
	{
		_event_queue.push(std::move(e));
	}

private:
	std::queue<event_t>& _event_queue;
};
