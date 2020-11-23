#pragma once

#include <imgui.h>

#include <utility>

namespace fs::gui::aux {

/*
 * wrappers for managing Dear ImGui state
 */

template <typename Impl, typename T>
struct scoped_override_base
{
	using value_type = typename Impl::value_type;

	scoped_override_base(value_type val)
	{
		Impl::push(val);
	}
};

template <typename Impl>
struct scoped_override_base<Impl, void>
{
	using value_type = typename Impl::value_type;

	scoped_override_base()
	{
		Impl::push();
	}
};

template <typename Impl>
class [[nodiscard]] scoped_override : public scoped_override_base<Impl, typename Impl::value_type>
{
public:
	using value_type = typename Impl::value_type;
	using base_type = scoped_override_base<Impl, value_type>;

	using base_type::base_type;

	~scoped_override()
	{
		if (_cleanup_required)
			Impl::pop();
	}

	scoped_override(const scoped_override&) = delete;

	scoped_override(scoped_override&& other)
	: _cleanup_required(std::exchange(other._cleanup_required, false))
	{
	}

	scoped_override& operator=(scoped_override other)
	{
		std::swap(_cleanup_required, other._cleanup_required);
		return *this;
	}

private:
	bool _cleanup_required = true;
};

struct scoped_pointer_id_impl
{
	using value_type = const void*;

	static void push(value_type id)
	{
		ImGui::PushID(id);
	}

	static void pop()
	{
		ImGui::PopID();
	}
};

using scoped_pointer_id = scoped_override<scoped_pointer_id_impl>;

struct scoped_group_impl
{
	using value_type = void;

	static void push()
	{
		ImGui::BeginGroup();
	}

	static void pop()
	{
		ImGui::EndGroup();
	}
};

using scoped_group = scoped_override<scoped_group_impl>;

struct scoped_tooltip_impl
{
	using value_type = void;

	static void push()
	{
		ImGui::BeginTooltip();
	}

	static void pop()
	{
		ImGui::EndTooltip();
	}
};

using scoped_tooltip = scoped_override<scoped_tooltip_impl>;

struct scoped_font_override_impl
{
	using value_type = ImFont*;

	static void push(value_type font)
	{
		ImGui::PushFont(font);
	}

	static void pop()
	{
		ImGui::PopFont();
	}
};

using scoped_font_override = scoped_override<scoped_font_override_impl>;

struct scoped_text_color_override_impl
{
	using value_type = ImU32;

	static void push(value_type color)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, color);
	}

	static void pop()
	{
		ImGui::PopStyleColor();
	}
};

using scoped_text_color_override = scoped_override<scoped_text_color_override_impl>;

}
