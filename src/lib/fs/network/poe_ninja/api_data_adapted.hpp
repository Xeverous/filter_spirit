#pragma once

#include <fs/network/poe_ninja/api_data.hpp>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/for_each.hpp>

#include <type_traits>

BOOST_FUSION_ADAPT_STRUCT(
	fs::network::poe_ninja::poe1::api_item_price_data,
	// general
	currency,
	fragment,
	unique_idol,
	runegraft,
	allflame_ember,
	tattoo,
	omen,
	divination_card,
	artifact,
	oil,
	incubator,
	// equipment & gems
	unique_weapon,
	unique_armour,
	unique_accessory,
	unique_flask,
	unique_jewel,
	unique_tincture,
	unique_relic,
	skill_gem,
	cluster_jewel,
	// Atlas
	map,
	blighted_map,
	blight_ravaged_map,
	unique_map,
	delirium_orb,
	invitation,
	scarab,
	// crafting
	base_type,
	fossil,
	resonator,
	beast,
	essence,
	vial)

BOOST_FUSION_ADAPT_STRUCT(
	fs::network::poe_ninja::poe2::api_item_price_data,
	// general
	currency,
	fragments,
	abyss,
	uncut_gems,
	lineage_support_gems,
	essences,
	ultimatum,
	idols,
	runes,
	ritual,
	expedition,
	delirium,
	breach)

namespace fs::network::poe_ninja {

namespace poe1 {

namespace detail {

template <typename F>
struct applier
{
	template <const char* Name, bool IsCurrency>
	void operator()(json_file<Name, IsCurrency>& object) const
	{
		f(Name, IsCurrency, object.file_content);
	}

	template <const char* Name, bool IsCurrency>
	void operator()(const json_file<Name, IsCurrency>& object) const
	{
		f(Name, IsCurrency, object.file_content);
	}

	F f;
};

} // namespace detail

template <typename F>
void api_item_price_data::for_each_file(F f)
{
	static_assert(std::is_invocable_v<F, const char*, bool, std::string&>);

	boost::fusion::for_each(*this, detail::applier<F>{f});
}

template <typename F>
void api_item_price_data::for_each_file(F f) const
{
	static_assert(std::is_invocable_v<F, const char*, bool, const std::string&>);

	boost::fusion::for_each(*this, detail::applier<F>{f});
}

} // namespace poe1

namespace poe2 {

namespace detail {

template <typename F>
struct applier
{
	template <const char* Name>
	void operator()(json_file<Name>& object) const
	{
		f(Name, object.file_content);
	}

	template <const char* Name>
	void operator()(const json_file<Name>& object) const
	{
		f(Name, object.file_content);
	}

	F f;
};

} // namespace detail

template <typename F>
void api_item_price_data::for_each_file(F f)
{
	static_assert(std::is_invocable_v<F, const char*, std::string&>);

	boost::fusion::for_each(*this, detail::applier<F>{f});
}

template <typename F>
void api_item_price_data::for_each_file(F f) const
{
	static_assert(std::is_invocable_v<F, const char*, const std::string&>);

	boost::fusion::for_each(*this, detail::applier<F>{f});
}

} // namespace poe2

}
