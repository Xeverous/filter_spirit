#pragma once

#include <Magnum/Math/Color.h>
#include <Magnum/Math/Packing.h>
#include <Magnum/Magnum.h>

#include <imgui.h>

#include <cstdint>
#include <utility>

namespace fs::gui {

/*
 * wrappers for converting sRGB format used in FS and ImGui displays to
 * float-based RGB used in Magnum's rendering and ImGui implementation
 */

inline Magnum::Color3 from_rgb(std::uint32_t rgb)
{
	using namespace Magnum;
	return Math::unpack<Math::Color3<Float>>(Math::Color3<UnsignedByte>{UnsignedByte(rgb >> 16), UnsignedByte(rgb >> 8), UnsignedByte(rgb)});
}

inline Magnum::Color3 from_rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b)
{
	using namespace Magnum;
	return Math::unpack<Math::Color3<Float>>(Math::Color3<UnsignedByte>{r, g, b});
}

inline Magnum::Color4 from_rgba(std::uint32_t rgba)
{
	using namespace Magnum;
	return Math::unpack<Math::Color4<Float>>(Math::Color4<UnsignedByte>{UnsignedByte(rgba >> 24), UnsignedByte(rgba >> 16), UnsignedByte(rgba >> 8), UnsignedByte(rgba)});
}

inline Magnum::Color4 from_rgba(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a)
{
	using namespace Magnum;
	return Math::unpack<Math::Color4<Float>>(Math::Color4<UnsignedByte>{r, g, b, a});
}

inline Magnum::Color3ub to_rgb(Magnum::Color3 color)
{
	return Magnum::Math::pack<Magnum::Color3ub>(color);
}

inline Magnum::Color4ub to_rgba(Magnum::Color4 color)
{
	return Magnum::Math::pack<Magnum::Color4ub>(color);
}

struct rect
{
	ImVec2 top_left;
	ImVec2 size;

	bool contains(ImVec2 point) const
	{
		return top_left.x <= point.x
			&& point.x < top_left.x + size.x
			&& top_left.y <= point.y
			&& point.y < top_left.y + size.y;
	}
};

constexpr ImU32 color_true  = IM_COL32(  0, 255,   0, 255);
constexpr ImU32 color_false = IM_COL32(255,   0,   0, 255);

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

} // namespace fs::gui
