#pragma once

#include <elements/element.hpp>
#include <infra/support.hpp> // for remove_cvref_t

#include <functional>
#include <string>
#include <utility>
#include <boost/date_time/posix_time/posix_time_types.hpp>

inline std::string to_string(boost::posix_time::time_duration duration)
{
	std::string result;

	if (auto d = duration.hours() / 24; d != 0)
		result.append(std::to_string(d)).append("d ");

	if (auto h = duration.hours() % 24; h != 0)
		result.append(std::to_string(h)).append("h ");

	if (auto m = duration.minutes(); m != 0)
		result.append(std::to_string(m)).append("m ");

	if (auto s = duration.seconds(); s != 0)
		result.append(std::to_string(s)).append("s ");

	if (result.empty())
		result = "0s";
	else
		result.pop_back(); // remove trailing space

	return result;
}

// not named make_group to avoid name conflict with el::make_group, picked up by ADL
template <typename Element>
inline auto make_section(std::string title, Element&& element)
{
	using namespace cycfi::elements;
	return group(std::move(title), margin(rect{ 10, 40, 10, 10 }, std::forward<Element>(element)));
}

template <typename Subject>
cycfi::elements::proxy<cycfi::remove_cvref_t<Subject>>
make_proxy(Subject&& subject)
{
	return { std::forward<Subject>(subject) };
}

inline auto make_progress_bar()
{
	using namespace cycfi::elements;
	return progress_bar(box(colors::black), box(get_theme().indicator_color));
}

inline auto make_button(std::string text, std::function<void(bool)> on_click)
{
	auto but = cycfi::elements::button(std::move(text));
	but.on_click = std::move(on_click);
	return but;
}

inline auto make_button(std::uint32_t icon_code, std::string text, std::function<void(bool)> on_click)
{
	auto but = cycfi::elements::button(icon_code, std::move(text));
	but.on_click = std::move(on_click);
	return but;
}

inline auto make_refresh_button(std::function<void(bool)> on_click)
{
	return make_button(cycfi::elements::icons::cycle, "refresh", std::move(on_click));
}

inline double calculate_progress(std::size_t done, std::size_t size)
{
	if (size == 0)
		return 0.0; // avoid division by 0, report no progress instead

	return static_cast<double>(done) / size;
}
