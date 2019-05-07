# filter writing tutorial

The following article aims to establish a set of good practices and conventions for writing a spirit filter (and in general, any filter).

- If you have no clue how Filter Spirit works, read "how it works" article first.
- If you want detailed spec what can be done in the Filter Spirit language, read "language documentation" article

## filter writing guidelines

Items can be filtered by different **properties** (no formal use of this word from GGG side but I picked this one to avoid any name conflict). Properties are independent of each other (although some combinations do not exist, like quest items with sockets). Filters support many different properties but because *every item that goes through the filter gets the style of first matched block* **the order of blocks is crucial to ensure correct filtering**.

This naturally arises the questions:

- How strictly (in the sense of amount of conditions, not item value) we should filter stuff?
  - strict (very detailed checking): we can apply different styles to very narrow set of items, but risk that some items may be not catched
  - generic: apply the same style to a wider set of items, this has the benefit that we likely do not need to modify / need less modifications to accomodate the filter for new items introduced in the future
- What should be the order of blocks?
- What if an item has multiple "interesting" properties?

### generic vs strict filtering

Because filter rules are first-match, we can do both without losing any benefits and without attracting drawbacks.

```
Class "Maps" {
	MapTier >= 16 {
		# apply styles in this block to any T16+ map
		SetTextColor ...
		SetMinimapIcon ...
		...
		Show
	}

	ShapedMap true {
		MapTier >= 11 {
			# red, shaped maps
			Set...
			Show
		}

		# apply styles in this block to any shaped map
		SetTextColor ...
		SetMinimapIcon ...
		...
		Show
	}

	# apply styles in this block to any map (just map)
	SetTextColor ...
	SetMinimapIcon ...
	...
	Show
}
```

We simply **filter with details first and later with a very broad condition** - here the last `Show` requires an item only to be just a map.

Sometimes you may want to style the last (generic) block as an error block:

```
Class "Currency" {
	BaseType "Scroll" {
                ...
        }

        BaseType ["Mirror of Kalandra", "Exalted Orb"] {
                ...
        }

        # more BaseType blocks...
        # ...


        # something is currency, but was not catched earlier
        # likely some newly introduced currency or something that was forgotten
        # give it some very loud colors / icons / sounds so we are notified
        # that the filter needs fixing
        SetBackgroundColor RGB(255, 0, 0)
        SetAlertSound "error.wav"
        Show
}
```

### filter core structure: class condition blocks

Each item in Path of Exile has an associated **class** (exactly 1). Some classes are subclasses of others ("One Hand Maces" is a subclass of "One Hand" which is a subclass of "One").

It is recommended to build a filter around classes because:

- They are unique (no item has multiple associated classes) - this gives freedom for the order of blocks because no item can be misclassified (pun intended) due to having multiple classes. Freedom of order allows to optimize the filter - the more items are catched earlier the better - there have been some reports from people using very large filters that they can impact (slow down) UI so much it is noticeable for humans (even without highlight key on).
- Classes are strongly related to items purpose
  - filtering by class allows to easily style items based on their intended usage (and often value)
  - new items may be introduced in the future - no action is required to modify the filter if in every class you leave a final show block with no additional conditions
- Classes have no name clashes. Actual filter uses very naive approach of checking whether given class is a substring of filtered item class. For classes, there is no problem because all are unique strings. This is not true for base types: for example, the full name of `The Wolf` card is a substring of the `The Wolf's Shadow` card. Filtering for `The Wolf` catches both cards. To workaround, filter longer names (superset strings) first.

### non-class properties

Some items are valueable (or just "interesting") not because of their class but of other properties (eg links, colors). Correct placement of these blocks is hard because:

- For some items, having a rare non-class propety is often meaningless or has practically no impact on item value (eg top-tier unique with RGB links).
- Some non-class properties can be interesting by items of very varying classes. They may have very varying impact on item value.
- We would like to collect RGB items but some of them are more valuable (eg 6-socket RGB items). Multiple interesting properties can interleave.

So, before filtering by colors and links, filter out all items that can have these but also have more important properties.

See the checklist which is further down in this article.

### price-based filtering

Filter Spirit allows to use market item prices when generating the filter. This is one of core features of FS, but please bear in mind that:

- You need to select the correct league when downloading the prices. If you play on multiple leagues, generate a filter for each.
- Some items have very scarce supply and may have no price information. Be prepared for it - **don't write div card block with only price-based nested blocks**.
- Not all items have stable and/or reliable market prices. This is especially true at the league start. You may want to refresh the filter as often as per hour.
- While some items have a very clear market value, it is impossible to filter them. Unique jewels are a primary example - since the filter only sees item base type, it is impossible to determine which unique it is.

### tips and tricks

- There is no generic "Equipment" class that would allow to easily catch chaos/regal recipe scrap-metal etc. However `Rarity` condition is as good because it not only catches only items with specified rarity, it also limits catched items to only these that can have `Rarity` property - a block with `Rarity >= normal` condition will never catch any currency, because currency has no such property.
- You can reuse the same sound for multiple sets of items without losing certainty if they never drop together. Example: stones drop only during incursions and trinkets drop only in lab.

## checklist

- Talismans should be filtered before rares, otherwise they will blend into amulets.
- Uniques should be filtered before RGB and linked items.
- Filter 6L uniques before uniques.
- Filter white socket items after RGB items and uniques.
- Chance/crafting bases: don't forget `Corrupted false`.
- Don't forget to disable leveling gear once a certain character level was reached - for example, you can display any item with `LinkedSockets 4` but only if they have `ItemLevel < 50`. If you reach higher-lvl zones they will simply have too high ilvl to appear.
- Don't forget the very last block in the filter that has no conditions (either use it as an error block or to hide everything remaining).
- If you place multiple price blocks, go with highest prices first - in case of name clashes you will get false alarms which is better than hidden worthy items.

Things you have to decide:

- 6s 5L - at the start of a league might be worth few chaos, later vendor for jewellers. You may want to:
  - treat these as 5L (filter in order: 6L, 5L, 6s) because they can be worth at the start of a league and later ... a quite rare chance of losing a 6s or just getting the highlight wrong.
  - make a separate block for them (filter in oder: 6L, 5L 6s, 5L, 6s)
- Hammers that are RGB:
  - filter for RGB first (chromatic recipe prority)
  - filter for hammers first (chisel recipe priority)
  - filter for RGB hammers first (if you are pedantic)

## corner cases

Basically a list of name clashes.

- `Sai` (a dagger base) also catches
  - Saint's Hauberk
  - Saintly Chainmail
  - Mo**sai**c Kite Shield

Solution: add `Class "Dagger"`. None of other items are daggers.

- `The Wolf` divination card is a substring of the `The Wolf's Shadow` card.

Solution: filter the one with longer name first.

# filter template

The following code is the recommended order of blocks in a filter.

For each item set, the strictest possible rules are provided - you can use fewer rules (in most cases, sole `BaseType` is enough) but having strict rules results in better error handling. You can also use strict rules to split blocks, eg to separate 1-3 socket resonators and 4 socket resonators.

You can use this file as a starting point if writing spirit filter from scratch.

Notes:

- This is a recommendation, constructive criticizm welcomed.
- The following template is not the only one possible.
- The following template is a generic mapping filter - if you are very into racing/leveling or private leagues you may have a very different structure.
- The following template is targeted at middle-experienced player. If you measure your clear speed in maps per minute and DPS in Shapers per second you will likely have a very different view on what should be picked up.

## currency

```
# NOTE: you can also use "Stackable Currency" for a more narrow results
# NOTE: Fossils are stackable as far as filter is concerned (known bug)
# NOTE: you can also use "Delve Socketable Currency" (currently only resonators)

# if you use any of narrow classes listed above, make sure to also
# use a generic "Currency" block to filter remaining currency items

Class "Currency" {
	# add favourite orb blocks here
	# ...

	# fossils
	BaseType "Fossil" {
		Show
	}

	# resonators
	BaseType "Resonator" {
		Sockets 4 {
			Show
		}

		Show
	}

	# Perandus coins
	BaseType "Perandus Coin" {
		Show
	}

	# prophecies
	BaseType "Prophecy" {
		Show
	}

	# Breach splinters
	BaseType "Splinter" {
		Show
	}

	# Breach blessings
	BaseType "Blessing" {
		Show
	}

	# Incursion vials
	BaseType "Vial of" {
		Show
	}

	# essences - corruption only
	BaseType [
		"Essence of Hysteria",
		"Essence of Insanity",
		"Essence of Horror",
		"Essence of Delirium"] {
		Show
	}

	# essences - top 2 tiers
	BaseType ["Deafening Essence of", "Shrieking Essence of"] {
		Show
	}

	# essences - anything else
	BaseType "Essence" {
		Show
	}

	# shards
	BaseType "Shard" {
		Show
	}

	# recomemnded place for an error block - any other currency will be catched here
	SetTextColor error
	SetAlertSound "error.wav"
	Show
}
```

## gems

```
# Note: you can also use these classes, which are more narrow:
# "Active Skill Gems"
# "Support Skill Gems"

Class "Gems" {
	# drop only gems
	BaseType ["Empower", "Enhance", "Enlighten", "Portal"] {
		Show
	}

	# vaal gems
	BaseType "Vaal" {
		Show
	}

	# quality or any other filterting you would like
	# here: shwow gems with quality and hide any remaining gems
	Quality > 0 {
		Show
	}

	Hide
}
```

## hammers for chisel recipe

```
Class "One Hand Maces" &&
BaseType ["Gavel", "Stone Hammer", "Rock Breaker"] {
	Rarity normal {
		Show
	}

	Rarity magic && Quality > 12 {
		Show
	}

	Rarity rare && Quality > 16 {
		Show
	}
}
```

## harbinger pieces

```
Class "Piece" {
	Show
}
```

## uniques

Note: for a detailed example of using price-based filters, read the source code of example filter in the repository.

```
Rarity unique {
	# Tabula Rasa and Skin of the Loyal
	# and also a very rare case that a unique drops 6L'd - Delve
	# and Synthesis was reported to drop some already linked items
	LinkedSockets 6 {
		Show
	}

	# price-based filters
	# ...

	# anything else
	Show
}
```

## vendor items

```
LinkedSockets 6 {
	Show
}

LinkedSockets 5 {
	Show
}

Sockets 6 {
	Show
}
```

## Incursion items

```
Class "Incursion Item" &&
BaseType ["Stone of Passage", "Flashpowder Keg"] {
	Show
}
```

## fragments

```
Class "Map Fragments" {
	# Pantheon stuff
	BaseType "Divine Vessel" {
		Show
	}

	# scarabs
	# note: there are price queries for scarabs
	BaseType "Scarab" {
		Show
	}

	# normal Atziri
	BaseType "Sacrifice" {
		Show
	}

	# uber Atziri
	BaseType "Mortal" {
		Show
	}

	# Shaper guardian fragments
	BaseType [
		"Fragment of the Phoenix",
		"Fragment of the Minotaur",
		"Fragment of the Chimera",
		"Fragment of the Hydra"] {
		Show
	}
}
```

## leaguestones

```
Class "Leaguestones" {
	Show
}
```

## miscellaneous

```
Class "Misc Map Items" {
	# breachstones
	BaseType "Breachstone" {
		Show
	}

	# reliquary keys
	# Ancient Reliquary Key - dropped during Legacy league
	# Timeworn Reliquary Key - dropped during Delve league
	BaseType "Reliquary Key" {
		Show
	}

	# Pale Court keys
	BaseType ["Eber's Key", "Yriel's Key", "Inya's Key", "Volkuur's Key"] {
		Show
	}
}
```

## Labyrinth stuff

```
# NOTE: you can use Class "Labirynth" if you want to
# filter all of lab items with a single block

Class "Labyrinth Map Item" &&
BaseType "Offering to the Goddess" {
	Show
}

Class "Labyrinth Item" {
	BaseType "Treasure Key" {
		Show
	}

	# NOTE: Izaro speaks upon Golden and Silver key drops
	# so generally you do not want to add alert sounds here
	BaseType "Golden Key" {
		Show
	}

	BaseType "Silver Key" {
		Show
	}

	# any future lab item
	Show
}

Class "Labyrinth Trinket" {
	Show
}
```

## other classes

```
Class "Quest Items" {
	Show
}

# no idea what this is actually catching
Class "Hideout Doodads" {
	Show
}

# as above, no idea
# maybe items with skin transfer?
Class "Microtransactions" {
	Show
}

# NOTE: the following class was working previously but has been removed
# TODO verify this
# Class "Pantheon Soul" {
	# Show
# }

# The game has this class to refer to skills types that can be
# used on Facebreaker and Doryani's Fist but AFAIK no items
# with such class ever appeared in the game
Class "Unarmed" {
	Show
}

# NOTE: fishing rods are a separate class
# so far there is only 1 item that fits here
# and it's BaseType name is ... also "Fishing Rod"
Class "Fishing Rod" {
	BaseType "Fishing Rod" {
		Show
	}

	# right now: nothing, just in case we get new
	# fishing rod base types in the future
	Show
}
```

## divination cards

```
Class "Divination Card" {
        # low-value cards that you always want to pick up
        BaseType ["The Gambler", "The Void"] {
                ...
                Show
        }

        # whatever, these cards are immediate drops
        StackSize 1 {
                ...
                Show
        }

        # price-based blocks here
        BaseType $divination(100, 999999) {
                ...
                Show
        }

        BaseType $divination(10, 100) {
                ...
                Show
        }

        BaseType $divination(1, 10) {
                ...
                Show
        }

        # dont want garbage like Carrion Crow
        BaseType $divination(0, 1) {
                ...
                Hide
        }

        # cards with unknown value
        ...
        Show
}
```

TODO this is unfinished, missing some items

## remaining items

```
# place this at the end of the filter
# it will catch anything that was not catched or explicitly hidden earlier
# if something appears there it will mean that the filter does not cover
# all items in the game
SetAlertSound "error.wav"
Show
```
