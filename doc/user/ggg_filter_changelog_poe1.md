# List of changes in actual filters made by GGG - Path of Exile 1

## Basically a list of all official forum threads and reddit posts containing filter-related information.

Most threads contain a lit of new map tiers, new mods, new item base types and new filter features.

*newest at the top*

### 3.26 (Secrets of the Atlas + Trarthus) [Item Filter Information](https://www.pathofexile.com/forum/view-thread/3788869)

- new condition: `ZanaMemory Boolean`
- new condition: `MemoryStrands [CMP] Integer`
- `TransfiguredGem Boolean` has now a new variant: `TransfiguredGem String+` (matches specific gems)
- New Currencies, Map Fragments and Transfigured Gems
- changed Divination Cards and Veiled Chaos/Exalted Orb
- Removed Runes and Memories
- [patch notes](https://www.pathofexile.com/forum/view-thread/3787013)

### 3.25 (Settlers of Kalguur) [Item Filter Information](www.pathofexile.com/forum/view-thread/3532553)

- Huge balance patch with multiple reworks (over 30k words) - [patch notes](https://www.pathofexile.com/forum/view-thread/3531661)
- note: some implicit filter changes are hidden in the patch notes (e.g. quality changes)
- note: additional helpful information on [reddit](https://www.reddit.com/r/pathofexile/comments/1e9uez6/settlers_of_kalguur_item_filter_information/leiygze/)
- [new and changed gems](https://www.pathofexile.com/forum/view-thread/3532728)

### 3.24 (Necropolis) [Item Filter Information](https://www.pathofexile.com/forum/view-thread/3497460)

- Removal of Sextants and rework of Scarabs
- [patch notes](https://www.pathofexile.com/forum/view-thread/3496784)
- [new and changed Gems](https://www.pathofexile.com/forum/view-thread/3497686)
- [all Scarabs](https://www.pathofexile.com/forum/view-thread/3497694)

### 3.23.1

- [patch notes](https://www.pathofexile.com/forum/view-thread/3480999) - Added a new `/reloaditemfilter` chat command

### 3.23 (Affliction) [Item Filter Information](https://www.pathofexile.com/forum/view-thread/3452297)

- Removal of alternate quality (and introduction of Transfigured Gems)
- New item bases and Affliction-related item classes
- Metamorph replaced with Ultimatum
- [patch notes](https://www.pathofexile.com/forum/view-thread/3451455)

### 3.22.2b (Ancestors end)[https://www.pathofexile.com/forum/view-thread/3452244]

- *Added a new `Import` command to load contents of one item filter into another.*
- *Item filters can now use `!=` in place of `!` if desired to check for negation.*
- *When specifying multiple matches for negation specifically, item filters now use "none of" instead of "any of" for the match.*
- *`CustomAlertSound` can now specify multiple files, separated by semicolons, and will play one at random.*
- *Fixed `!=` (and `!`) not working with `EnchantmentPassiveNode`, `HasExplicitMod`, `HasEnchantment` rules.*
- *Fixed a bug where hidden items could still play effects, sounds and create minimap icons.*

### 3.22 (Trial of the Ancestors) [Item Filter Information](https://www.pathofexile.com/forum/view-thread/3410483)

- New gems, cards, currency and other items
- Adjusted gem class names
- [patch notes](https://www.pathofexile.com/forum/view-thread/3409617)

### 3.21 (Crucible) [Item Filter Information](https://www.pathofexile.com/forum/view-thread/3362077)

- New gems, cards, currency and other items
- Breachstones are now a separate item class (instead of "Map Fragments")
- Some Reliquary keys have different item classes now: https://www.pathofexile.com/forum/view-thread/3324396#THU6
- [patch notes](https://www.pathofexile.com/forum/view-thread/3361403)

### 3.20 (The Forbidden Sanctum) [Item Filter Information](https://www.pathofexile.com/forum/view-thread/3324656)

- New gems, cards, currency
- New item class: Relics
- [3.20.1 Item Filter update](https://www.pathofexile.com/forum/view-thread/3331539) - Invocations
- [patch notes](https://www.pathofexile.com/forum/view-thread/3323944)

### 3.19 (Lake of Kalandra) [Item Filter Information](https://www.pathofexile.com/forum/view-thread/3294085)

- New cards, ring bases and wand bases
- Reworked harvest - pickupable lifeforce
- New map pieces and related misc items
- Removed Sentinel drones and power cores
- [patch notes](https://www.pathofexile.com/forum/view-thread/3293287)
- [balance manifesto](https://www.pathofexile.com/forum/view-thread/3292308)

### 3.18 (Sentintel) [Item Filter Information](https://www.pathofexile.com/forum/view-thread/3265282)

- New awakened exceptional gems, cards, quest items, currency and sentinels (no new features in filters)
- [patch notes](https://www.pathofexile.com/forum/view-thread/3264788)
- [announcement post](https://www.pathofexile.com/forum/view-thread/3264698)
- [list of teasers](https://www.pathofexile.com/forum/view-thread/3265256)

### 03.03.2022 - [Undocumented Change](https://old.reddit.com/r/pathofexile/comments/t5eaa0/filters_are_broken/)

- The change broke one `EnchantmentPassiveNode` name: `"Damage with Two Handed Melee Weapons"` was changed to `"Damage with Two Handed Weapons"`
- Yes, this means that the fix is to just remove `Melee`. Pun intended.

### 3.17.2

- [patch notes](https://www.pathofexile.com/forum/view-thread/3251063)
- *Fixed a bug where temporary beam effects for item filters were not so temporary if they continued from a permanent beam.*

### 3.17 (Archnemesis + Siege of the Atlas) [Item Filter Information](https://www.pathofexile.com/forum/view-thread/3230194)

- New conditions: `HasSearingExarchImplicit`, `HasSearingExarchImplicit` and `ArchnemesisMod`
- [patch notes](https://www.pathofexile.com/forum/view-thread/3229187)
- [gem information](https://www.pathofexile.com/forum/view-thread/3229646)
- [announcement post](https://www.pathofexile.com/forum/view-thread/3229084)
- [list of teasers](https://www.pathofexile.com/forum/view-thread/3228003)

### 19.01.2022 - [HasExplicitMod bugs found](https://old.reddit.com/r/pathofexile/comments/s7bk7s/filter_condition_hasexplicitmod_seems_buggy/)

- `HasExplicitMod` was behaving incorrectly (in reverse) with operators `>`, `<`, and `<=`

### 3.16.0b

- [patch notes](https://www.pathofexile.com/forum/view-thread/3193737)
- *Fixed a bug where Item Filters would incorrectly assign Weapons and Jewellery a BaseDefencePercentile value of 0 rather than 100.*

### 3.16 (Scourge) - [Item Filter Information](https://www.pathofexile.com/forum/view-thread/3187476)

- New conditions: `BaseDefencePercentile`, `BaseArmour`, `BaseEvasion`, `BaseEnergyShield`, `BaseWard`, `Scourged`, `UberBlightedMap`
- [patch notes](https://www.pathofexile.com/forum/view-thread/3186390)
- [recent topics](https://www.pathofexile.com/forum/view-thread/3174380)
- [gem information](https://www.pathofexile.com/forum/view-thread/3187742)
- [atlas information](https://www.pathofexile.com/forum/view-thread/3187176)
- [list of teasers](https://www.pathofexile.com/forum/view-thread/3184585)
- [ascendancy changes](https://www.pathofexile.com/forum/view-thread/3187468)

### 3.15.0d

- [patch notes](https://www.pathofexile.com/forum/view-thread/3152771)
- *Rapier One Hand Swords are once again classified as Thrusting One Hand Swords. Their Weapon Range is again 14 (instead of 11). Please update your item filters!*

### 3.15 (Expedition) - [Item Filter Information](https://www.pathofexile.com/forum/view-thread/3147480)

- [patch notes](https://www.pathofexile.com/forum/view-thread/3147479)
- [balance manifesto](https://www.pathofexile.com/forum/view-thread/3147157)
- [gem information](https://www.pathofexile.com/forum/view-thread/3147751)
- New actions: `CustomAlertSoundOptional`, `DisableDropSoundIfAlertSound` and `EnableDropSoundIfAlertSound` (described in other documentation)
- Removal of item class `"Critical Utility Flasks"` (now Diamond Flasks are `"Utility Flasks"`)
- Class names have been standarized to use plural forms

### 3.14.1c (or maybe earlier)

- undocumented change: filters stopped accepting `""` and `"Microtransactions"` item classes

### 3.14.0b - invisible items fix

- [patch notes](https://www.pathofexile.com/forum/view-thread/3085367)
- [bug report](https://www.pathofexile.com/forum/view-thread/3084331)
- [reddit thread](https://www.reddit.com/r/pathofexile/comments/mtcsw6/significant_bug_within_item_filters_invisible/)
- *Fixed a bug where Item Filters could set item labels to be completely transparent when using certain `Continue` statements.*

### 3.14 (Ultimatum) - [Item Filter Information](https://www.pathofexile.com/forum/view-thread/3079980)

- No new filter features. Only name changes within existing item classes.
- [patch notes](https://www.pathofexile.com/forum/view-thread/3079973) (explode mod killed)
- [balance manifesto](https://www.pathofexile.com/forum/view-thread/3079632) (It's All Nerfs)
- [Harvest manifesto](https://www.pathofexile.com/forum/view-thread/3069670)

### 3.13 (Ritual + Echoes of the Atlas) - [Item Filter Information](https://www.pathofexile.com/forum/view-thread/3009221)

- [patch notes](https://www.pathofexile.com/forum/view-thread/3009157)
- New condition: `EnchantmentPassiveNum`.
- `CustomAlertSound` now also supports volume.
- Fixes to some syntax bugs and more small improvements.
- New webpage describing filters: https://www.pathofexile.com/item-filter/about

### 3.12 (Heist) - [Item Filter Information](https://www.pathofexile.com/forum/view-thread/2935826)

- [patch notes](https://www.pathofexile.com/forum/view-thread/2935777)
- Added `AlternateQuality True` parameter that allows you to filter for Alternate Quality Gems.
- Added `Replica True` parameter that allows you to filter for Replica Unique Items.
- (edit after posting) You can now filter for specific quality types (`GemQualityType`).

### 3.11.1 (Harvest bugfixes)

- [patch notes](https://www.pathofexile.com/forum/view-thread/2893980) - new seed types.
- [reddit comments](https://old.reddit.com/r/pathofexile/comments/hk5c3n/3111_patch_notes/fwqo2yp/?context=3) about newly introduced items.

### 3.11 (Harvest) - [Item Filter information](https://www.pathofexile.com/forum/view-thread/2873743)

- [patch notes](https://www.pathofexile.com/forum/view-thread/2873739) - Herald nerfs and other changes
- Added `HasInfluence None` parameter.
- Added `EnchantmentPassiveNode` parameter to filter Cluster Jewels by enchantment type.

### 3.10.2b

From the [patch notes](https://www.pathofexile.com/forum/view-thread/2867400):

- You can now filter for items without being required to use special alphanumeric characters, such as "Maelstrom Staff" rather than "Maelström Staff".

### 3.10.1

From the [patch notes](https://www.pathofexile.com/forum/view-thread/2806232):

- Added an `EnableDropSound` parameter that can be used in Item Filters.
- Fixed an issue with the `PlayAlertSound` parameter in Item Filters not handling the `Continue` command correctly.

### 3.10 (Delirium) - [Item Filter information](https://www.pathofexile.com/forum/view-thread/2784312)

- [patch notes](https://www.pathofexile.com/forum/view-thread/2784310) - new skills, cards and unique items
- [reddit post - new minimap icons and colors preview](https://old.reddit.com/r/pathofexile/comments/fhn16d/lootfilter_stuffs_demo_of_the_new_filter_icons/)

### 3.9.2f

- [Upcoming filter improvements post](https://www.pathofexile.com/forum/view-thread/2771031) - `AreaLevel`, `CorruptedMods`, `Mirrored`, `Continue`, socket condition changes and more
- [questions on reddit answered by RhysGGG](https://www.reddit.com/r/pathofexile/comments/f2t4tz/inconsistencies_in_new_filter_syntaxes)

### 3.9 (Metamorph) - [Item Filter information](https://www.pathofexile.com/forum/view-thread/2687401)

- [patch notes](https://www.pathofexile.com/forum/view-thread/2687400) - new gems, base types, uniques and cards
- [reddit post clarification](https://www.reddit.com/r/pathofexile/comments/e910dq/forum_announcements_conquerors_of_the_atlas_patch/fafxgvj/) - regarding new influence types

### 3.8.1d

From the [patch notes](https://www.pathofexile.com/forum/view-thread/2655286):

- You can now use an "Exact Match" parameter in Item Filters. Simply use == to filter for exact matches. For example, BaseType == "The Demon" will show The Demon Divination Card but will not show The Demoness.

### 3.8 (Blight) - [Item Filter information](https://www.pathofexile.com/forum/view-thread/2627576)

- [patch notes](https://www.pathofexile.com/forum/view-thread/2627574) - new gems, base types, uniques and cards

From [this reddit post](https://www.reddit.com/r/pathofexile/comments/d0isb7/lootfilter_neversinks_itemfilter_version_730/ezb1w0z/):

- The parser is now more strict about formatting. So as you can see in this very thread, some people are now getting new errors, e.g. if they have a number after a custom sound file path. Previously, that number was silently ignored; now, it raises a formatting error.
- Filters also no longer accept negative numbers for anything, since it doesn't make sense for any of the rules atm.
- Comments (marked with "#") should now work better. Previously they could cause errors in certain places because they were incorrectly interpreted as search strings/values etc. Now, they should work everywhere. If you find somewhere that comments still don't work, let me know.
- Fixed the "Your Item Filter is out of date" error message appearing too often. It's only supposed to show if the filter fails to load the very first time the game tries and the filter hasn't updated since then.

### 3.7 (Legion) - [Item Filter information](https://www.pathofexile.com/forum/view-thread/2513458)

- [patch notes](https://www.pathofexile.com/forum/view-thread/2513456) - new gems, base types, uniques and cards

### 3.6 (Synthesis) - [Item Filter information](https://www.pathofexile.com/forum/view-thread/2333649)

From the [patch notes](https://www.pathofexile.com/forum/view-thread/2333648):

- Item filters can now filter out Shaper and Elder items, if that's something you want to do for some reason.

### 3.5 (Betrayal) - [Item Filter information](https://www.pathofexile.com/forum/view-thread/2255461)

- [patch notes](https://www.pathofexile.com/forum/view-thread/2255460) - new gems, base types, uniques and cards

### 3.4 (Delve) - [Item Filter information](https://www.pathofexile.com/forum/view-thread/2201188)

- [patch notes](https://www.pathofexile.com/forum/view-thread/2201187) - new gems, base types, uniques and cards

### 3.3.0c

From the [patch notes](https://www.pathofexile.com/forum/view-thread/2167029):

- Tweaked various alert sounds in the Default Loot Filter.

### 3.3 (Incursion) - [Item Filter information](https://www.pathofexile.com/forum/view-thread/2150238)

- [patch notes](https://www.pathofexile.com/forum/view-thread/2150237) - new gems, base types, uniques and cards

### 3.2 (Bestiary) - [Item Filter information](https://www.pathofexile.com/forum/view-thread/2093501)

- [patch notes](https://www.pathofexile.com/forum/view-thread/2093500) - new gems, base types, uniques and cards

### 3.1 (War for the Atlas + Abyss) - [Item Filter information](https://www.pathofexile.com/forum/view-thread/2036673)

- [patch notes](https://www.pathofexile.com/forum/view-thread/2036284) - new gems, base types, uniques and cards

### 3.0 (Fall of Oriath + Harbinger) - [Item Filter information](https://www.pathofexile.com/forum/view-thread/1930848)

From the [patch notes](https://www.pathofexile.com/forum/view-thread/1930316):

- In item filters, you can now use PlayAlertSoundPositional rather than PlayAlertSound to play a positional version (the sound comes from where the item dropped).
- Item filters can now play a much larger array of sounds when items drop.

### 2.6.0 (Legacy) - [Item Filter information](https://www.pathofexile.com/forum/view-thread/1840246)

- [patch notes](https://www.pathofexile.com/forum/view-thread/1839765) - new gems, base types, uniques and cards

### 2.5.0 (Breach) - [Item Filter information](https://www.pathofexile.com/forum/view-thread/1774440)

From the [patch notes](https://www.pathofexile.com/forum/view-thread/1773975):

- Items filters can now see whether an item is corrupted with "corrupted true" (Yes, this code fragment contains casing mistakes).

### 2.4.2

From the [patch notes](https://www.pathofexile.com/forum/view-thread/1767835):

- There's now an item filter bundled with Path of Exile that you can select in the User Interface options.

### 2.4.0 (Atlas + Essence) - [Item Filter information](https://www.pathofexile.com/forum/view-thread/1716826)

- [patch notes](https://www.pathofexile.com/forum/view-thread/1716228) - new gems, base types, uniques and cards

### 2.3.2 (Mediallion race season)

From the [patch notes](https://www.pathofexile.com/forum/view-thread/1703209):

- Added "Identified" as a filter option for Item Filters. Valid values for this filter are "True" or "False".

### 2.3.0 (Prophecy) - [Item Filter information](https://www.pathofexile.com/forum/view-thread/1667538)

- New item base type: `Offering to the Goddess`. Its current class is `Map Fragments` (the thread now is either outdated or was never correct about offering's class)
- [patch notes](https://www.pathofexile.com/forum/view-thread/1667259) - new gems, base types, uniques and cards

### 2.2.1d

From the [patch notes](https://www.pathofexile.com/forum/view-thread/1621545):

- Fixed a bug where Labyrinth Items could be displayed incorrectly when modifying how they are shown using Item Filters.

### 2.2.0 (Ascendancy + Perandus)

- New item class: `Labyrinth Trinket`
- [patch notes](https://www.pathofexile.com/forum/view-thread/1596094) - new gems, base types, uniques and cards

### 2.1.0 (Talisman)

- Talismans - new amulet base types. **They do not have separate item class.**
- [patch notes](https://www.pathofexile.com/forum/view-thread/1489915) - new gems, base types, uniques and cards

### 2.0.1

From the [patch notes](https://www.pathofexile.com/forum/view-thread/1367504):

- Sound events triggered by item filters can now have a maximum volume of 300 (up from 100).

### 2.0.0b

From the [patch notes](http://www.pathofexile.com/forum/view-thread/1306473):

- Fixed a bug with Item Filters where some item types would not load if they were included in a type that was already loaded. For example, "Thrusting" when "Swords" was already loaded.

### 2.0 (The Awakening + Tempest + Warbans) - www.pathofexile.com/itemfilters

From the [patch notes](http://www.pathofexile.com/forum/view-thread/1304441):

- Item filters were introduced to the game.
- See http://www.pathofexile.com/forum/view-thread/1235695 for more information

Initial filter code guide linked by GGG: https://www.pathofexile.com/forum/view-thread/1260664
