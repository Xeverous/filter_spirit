#pragma once

#include <Magnum/Math/Color.h>
#include <Magnum/Math/Packing.h>
#include <Magnum/Magnum.h>

#include <imgui.h>

namespace fs::gui::aux {

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

}
