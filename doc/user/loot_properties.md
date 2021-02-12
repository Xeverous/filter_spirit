# loot properties

This article lists properties of all kinds of loot, providing information on all possible conditions (as seen by item filters) (except `AreaLevel`, `BaseType` and `Prophecy`). Most of them are for implementation completness but for some items you might be interested to filter by more than just `Class` and `BaseType`. In general, you should filter items only by these 2 conditions, optionally with as few extra conditions as possible to increase filter stability in case something changes.

You can use this file as a guide how to match specific items - it should list everything that can be dropped in the game.

## global defaults

These are default values for conditions on items that do not state otherwise. Provided for implementation completness, not recommended to use for everything (some defaults were even bugged in the past).

Note: these apply to all items - filters always see some default value even if the item has no such property. Examples: all currency items are unidentified, all cards have `0` sockets, all non-gem items have `GemLevel` equal to `0` and so on...

```
Rarity                == Normal
ItemLevel             == 0
DropLevel             == 1
Quality               == 0
LinkedSockets         == 0
Height                == 1
Width                 == 1
StackSize             == 1
GemLevel              == 0
MapTier               == 0
CorruptedMods         == 0
EnchantmentPassiveNum == 0 # should be, but bugged as of writing this

HasInfluence           None
EnchantmentPassiveNode # (nothing)
HasExplicitMod         # (nothing)
HasEnchantment         # (nothing)

GemQualityType Superior

Sockets     == 0
SocketGroup == 0

AnyEnchantment   False
Identified       False
Corrupted        False
Mirrored         False
ElderItem        False
ShaperItem       False
FracturedItem    False
SynthesisedItem  False
ShapedMap        False
ElderMap         False
BlightedMap      False
Replica          False
AlternateQuality False
```

## currency items

This includes:

- All *Basic Currency* (including scrolls)
- All *Basic Currency* shards
- Atlas-related currency
- Conqueror and Maven Orbs
- All Splinters (Breach, Legion, Simulacrum, Crescent, Ritual)
- Essences and `Remnant of Corruption`
- Fossils
- Catalysts
- Blessings (Breach unique upgrade items)
- Oils (Blight league content)
- Delirium Orbs
- Harbinger scrolls (Harbinger unique upgrade items)
- Vials (Incursion league content)
- Nets (Bestiary league content, legacy items)
- Harvest currency (`Facetor's Lens`, `Infused Engineer's Orb`)
- Ritual currency (`Ritual Splinter`, `Ritual Vessel`) but not `Blood-filled Vessel`
- Heist currency (`Prime Regrading Lens`, `Secondary Regrading Lens`, `Tempering Orb`, `Tailoring Orb`, `Rogue's Marker`)
- itemised prophecies
- `Stacked Deck`

Notes:

- High `DropLevel` does not strictly correlate with currency value. Some low-value currencies require high drop levels (`Simple Sextant` - `68`) and some high-value currencies require low drop levels (`Exalted Orb` - `35`).
- In Harvest league it was possible to obtain specific currency (`Gemcutter's Prism`) in stack size of `0`. Item filters correctly detect this.
- Anything that can stack (outside special stash tabs) is `"Stackable Currency"`.
- Some items can not stack (`Facetor's Lens`, prophecies) but still are classified as such. This is intentional (see point below).
- There is no `"Currency"` class. All are `"Stackable Currency"`. `Class "Currency"` works because without `==` it requires only a substring match, not a 1:1 match.
- Prophecies can be caught by `Prophecy` condition which works for them the same way as `BaseType`.

```
DropLevel >= 1 # many are 1 but also many start at very high levels
StackSize >= 1 # see notes

Class == "Stackable Currency"
```

## resonators

```
Class == "Delve Stackable Socketable Currency"
Sockets >= 1
Sockets <= 4
```

## divination cards

- `StackSize` reports the current stack size. There is no way to check maximum stack size or whether a stack of cards is complete.

```
Class == "Divination Card"
```

## incubators

```
Class == "Incubator"
```

## gems

- Awakened gems can be filtered by `BaseType "Awakened"`.
- Vaal gems can be filtered by `BaseType "Vaal"`.
- There is no easy way to filter drop-only gems besides `BaseType`.
- There is no way to obtain gem's max level information.
- There is no way to obtain gem's color or requirements.

```
Class == "Active Skill Gems" "Support Skill Gems"
```

## quest items

- Quest items must be filtered by `Class` condition. Other conditions may not work. This is intentional.
- Non-tradeable watchstones are not considered quest items.
- (Grand) Heist target items are not considered quest items.

```
Class == "Quest Items"
```

## labyrinth

### keys

```
Class == "Labyrinth Item"
```

### trinkets

```
Class == "Labyrinth Trinket"
```

## incursion items

```
Class == "Incursion Item"
BaseType == "Stone of Passage" "Flashpowder Keg"
```

## harvest items

Note: these have been removed in 3.13, such item classes and items no longer exist.

```
Class == "Harvest Seed" "Seed Enhancer"
```

## leaguestones

Note: legacy content, but items still exist on permanent leagues.

```
Class == "Leaguestones"
```

## misc map items

```
Class == "Misc Map Items"
BaseType == "Ancient Reliquary Key" "Timeworn Reliquary Key" # reliquary keys from Legacy and Delve leagues
BaseType "Maven's Invitation" # tradeable invitations
```

## map fragments

This includes:

- scarabs
- (Uber) Atziri fragments
- (Uber) Elder fragments
- Shaper fragments
- Pale Court fragments
- Breachstones
- Emblems
- `The Maven's Writ`
- `Simulacrum`
- `Blood-filled Vessel`
- `Divine Vessel`
- offerings

```
Class == "Map Fragments"
```

## watchstones

This covers (all watchstones):

- non-tradeable watchstones obtained from Conqueror quests
- unique watchstones
- region-specific watchstones

```
Class == "Atlas Region Upgrade Item"
```

## heist items

### trinkets

```
Class == "Trinkets"
```

### contract targets

```
Class == "Heist Target"
```

### contracts

- There is no way to obtain any contract-specific stats (reward, job level requirements, etc)
- Contract's Area Level is equal to `ItemLevel` or `83`, whichever is lower.

```
Class == "Contract"
```

### blueprints

- There is no way to obtain any blueprint-specific stats (reveals, rewards, job level requirements, etc)
- Blueprint's Area Level is equal to `ItemLevel` or `83`, whichever is lower.

```
Clsss == "Blueprint"
```

### equipment

- There is no easy way to filter for equipment for specific jobs or job levels (although there are strong correlations with `BaseType`).

```
Class == "Heist Cloak" "Heist Brooch" "Heist Tool" "Heist Gear"
```

## metamorph parts

```
Rarity == Unique
Class == "Metamorph Sample"
ItemLevel       # follows same rules as any equippable item
Identified True # all metamorph parts are considered identified
```

## unique pieces

```
Rarity == Unique
Class == "Piece"
ItemLevel # follows same rules as any equippable item
```

## maps

- There is no way to check if a map is on its natural tier.
- Filter blighted maps by `BlightedMap True`.
- `ShapedMap` and `ElderMap` are conditions for old (removed) Atlas. Use `HasInfluence` instead.
- Most maps have their tier related to `ItemLevel` but there are exceptions. Use dedicated `MapTier` condition.
- `AnyEnchantment True` will detect any of (enchant names remain unknown):
  - applied Delirium
  - any Harvest enchant

## equippable items

- There is no dedicated condition or value that differentiates all equippable items.
- There is no way to differentiate Relic uniques from other uniques.

```
# main parts
Class == "Gloves" "Boots" "Helmets" "Body Armours"

# 1-handed
Class == "Claws" "Daggers" "Rune Daggers" "One Hand Swords" "Thrusting One Hand Swords" "One Hand Axes" "One Hand Maces" "Sceptres" "Wands"

# 2-handed
Class == "Fishing Rods" "Bows" "Staves" "Warstaves" "Two Hand Swords" "Two Hand Axes" "Two Hand Maces"

# offhand
Class == "Quivers" "Shields"

# jewellery
Class == "Amulets" "Rings" "Belts"
```

### talismans

- These do not have a separate class. This is the best way to filter them.
- There is no direct way to check talisman tier, use `BaseType` correlation instead.

```
Class == "Amulet"
BaseType "Talisman"
```

### rare items with specific mods

- Items with Incursion and Bestiary mods always drop as identified rare items.
- Warband mods appear on magic items of the correct item class that drops from the respective Warband members. These items drop unidentified.
- Veiled items always drop as `Rare` or `Unique`.

```
# incursion mods
HasExplicitMod == "Citaqualotl's" "Guatelitzi's" "Matatl's" "Tacati's" "Topotante's" "Xopec's" "of Citaqualotl" "of Guatelitzi" "of Matatl" "of Puhuarte" "of Tacati"
# bestiary mods
HasExplicitMod == "of Farrul" "of Saqawal" "of Craiceann" "of Fenumus"
# warband mods
HasExplicitMod == "Redblade" "Mutewind" "Brinerot" "Betrayer's" "Deceiver's" "Turncoat's"
# veiled mods
HasExplicitMod == "Veiled" "of the Veil"`
```

## jewels

### abyss jewels

```
Class == "Abyss Jewel"
```

### cluster jewels

- These do not have a separate class. Use `BaseType "Cluster"`, `EnchantmentPassiveNode` or `EnchantmentPassiveNum` to filter them from other jewels.

```
Class == "Jewel"
```

### basic jewels

```
Class == "Jewel"
```

### flasks

```
Class == "Critical Utility Flasks" # so far only diamond
Class == "Utility Flasks"
Class == "Life Flasks"
Class == "Mana Flasks"
Class == "Hybrid Flasks"
```
