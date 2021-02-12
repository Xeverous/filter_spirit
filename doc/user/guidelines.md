# filter writing guidelines

TODO some code samples are outdated
TODO rewrite/refactor this article

___

The following article aims to establish a set of good practices and conventions for writing any filter. Examples use FS filter template language but they are applicable to real filters as well.

If you have no clue how Item Filters work, read *how filters work* article first.

Items can be filtered by different **properties** (no formal use of this word from GGG side but I picked this one to avoid any name conflict). Properties are independent of each other (although some combinations do not exist, like quest items with sockets). Filters support many different properties but because *every item that goes through the filter gets the style of first matched block* **the order of blocks is crucial to ensure correct filtering**.

- What should be the order of blocks?

### general tips and tricks

- Use as many `==` (strict string matching) much as possible. This will avoid any unwanted matches due to substring clashes. Use `==`-less conditions only when you do want them to match multiple names.
- If after adding `==` to a condition you get an error that the name does not exist, check it is spelled correctly:
  - Some class names end with "s" (`Active Skill Gems`, `Quest Items`), some without (`Incubator`, `Divination Card`). Blame GGG for their inconsistency.
  - If it is a `BaseType` string, check wiki whether the name contains "The".
- If you use sounds to differentiate drops: you can reuse the same sound for multiple sets of items without losing certainty if they never drop together. Example: `Stone of Passage` drops only during Incursions and Labyrinth keys drop only in Labyrinth.

### filter core structure: class condition blocks

It is recommended to build filter's core around `Class` conditions because:

- Each item in Path of Exile has associated exactly one class. This means that if the filter has ClassAblock1, ClassAblock2, ClassAblock3, ClassBblock1, ClassBblock2, ClassCblock1 layout you can:
  - freely reorder class-block-groups for filter performance without affecting its behavior
  - take the most advantage of FS nesting feature and `Continue` blocks
- Classes are strongly related to items purpose:
  - Filtering by class allows to easily style items based on their intended usage (and often value).
  - Filters which first separate items by class require least modification in case something in the game changes or new item classes are introduced.

### items with multiple interesting properties

- Because filters stop processing blocks once a non-continue block matches, you should filter first for most specific items, then decrease requirements in further blocks. Most specific items will stop on the first block while less specific ones will traverse further untill they match potentially small amount of conditions. This often correlates with item's value.
- To avoid code duplication when you want to apply the same style for multiple items you may:
  - (A) (in FS templates) nest blocks in a parent dummy block that applies the style
  - (B) (in FS templates OR in real filters) precede actual blocks with a `Continue` block that applies the style
- Some items may have multiple small interesting properties (RGB link, 6 sockets, 20+ quality). Filter by more important / more valuable property first. See the checklist which is further down in this article.

```
Class == "Maps"
{
	SetFontSize 38 # (A)

	BlightedMap True # (B)
	{
		SetBackgroundColor ...
		Continue
	}

	MapTier >= 16
	{
		SetTextColor ...
		SetMinimapIcon ...
		...
		Show
	}

	MapTier >= 11
	{
		SetTextColor ...
		SetMinimapIcon ...
		...
		Show
	}

	MapTier >= 6
	{
		SetTextColor ...
		SetMinimapIcon ...
		...
		Show
	}

	# any map
	SetTextColor ...
	SetMinimapIcon ...
	...
	Show
}
```


### error detection

Sometimes you may want to style the last (generic) block as an error block:

```
Class == "Stackable Currency"
{
	BaseType "Scroll"
	{
		...
	}

	BaseType == "Mirror of Kalandra" "Exalted Orb"
	{
		...
	}

	# more BaseType blocks...
	# ...

	# Block for items that were not caught earlier.
	# Likely something newly introduced or something that was forgotten.
	# Give it special colors / icons / sounds etc to inform that the filter needs fixing.
	SetBackgroundColor 255 0 0 255
	SetAlertSound "error.wav"
	Show
}
```

## checklist

- Filter as many items by class as possible before you use other conditions.
- Talismans should be filtered before rares, otherwise they will blend into amulets.
- Uniques should be filtered before RGB and linked items.
- Filter 6L uniques before uniques.
- Filter white socket items after RGB items and uniques.
- Filter mod-specific rares (Incursion, Syndicate, Warband, Synthesis etc mods) before all other rares.
- Chance Orb and crafting bases: don't forget `Corrupted False`.
- Don't forget to disable leveling gear once a certain character level was reached - for example, you can display any item with `LinkedSockets 4` but only if `AreaLevel <= 67`.
- Don't forget the very last block in the filter that has no conditions (either use it as an error block or to hide everything remaining).

Things you have to decide:

- 6s 5L - at the start of a league might be worth few chaos, later vendor for jewellers. You may want to change the behavior depending on strictness:
  - treat these as 5L (filter in order: 6L, 5L, 6s) because they can be worth at the start of a league and later ... a quite rare chance of losing a 6s or just getting the highlight wrong.
  - treat these as 6s (filter in order: 6L, 6s, 5L) - just remember to pickup all 6s at the start of the league
  - make a separate block for them (filter in oder: 6L, 5L 6s, 5L, 6s)
- Hammers that are RGB:
  - filter for RGB first (chromatic recipe prority)
  - filter for hammers first (chisel recipe priority)
  - filter for RGB hammers first (if you are pedantic)
