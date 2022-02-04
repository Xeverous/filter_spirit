# actual filter language

The following document aims to outline hidden rules of the actual filter language that the Path of Exile game client is parsing.

[The article on wiki](https://pathofexile.gamepedia.com/Item_filter) is generally correct but there are some hidden, undocumented implementation details in game's client parser that have visible consequences but are nowhere mentioned.

Additionally, wiki (which uses game metadata) is not a good source of item class information: Offering is classified as "Labyrinth Map Item" in game files while the actual one that works in filters is "Map Fragments" (or any superset). Likely, filters use a different item class system than the game data itself.

GGG does not maintain any dedicated documentation. All information is gathered from official forum posts, annoucements, reddit and my experiments.

Multiple things have been fixed with the 3.8 update (Blight league). See [this reddit reply and its parents](https://www.reddit.com/r/pathofexile/comments/d0isb7/lootfilter_neversinks_itemfilter_version_730/ezeood1/).

## trailing comments

Generally it's adviced not to rely on implementation details. There are multiple filter tools that add trailing comments to various actions in filters to allow easy find-replace edits instead of redownloading/regenerating the filter.

Given that:

- ~~not all comments are properly parsed, some cause syntax errors other are read as eg base type names~~ **fixed in 3.8**
- there are ~~many~~ few cases in which invalid filters are accepted and you end up with something that does not work

...~~I'm not really going to support traling comments in generated filters, at most make it optional~~ I might add similar feature to Filter Spirit at some point.

## gotchas

These are not bugs, but may be viewed as such if understood incorrectly.

- Resonators fall into `"Delve Stackable Socketable Currency"`, `"Stackable Socketable Currency"`, `"Socketable Currency"` and `"Currency"` class conditions but not into `"Stackable Currency"`. This may seem inconsistent with how other supersets of item classes are handled - other supersets either do work (`"Axes"` vs `"Two Hand Axes"`) or error when loading (`"Two Axes"`). This is because some strings may be substrings of another class name - if unsure about class name validity use `==` and the game client will error if it is not a proper full class name.
- There is no `"Currency"` class. All semingly such items are classified exactly as `"Stackable Currency"`, even if they do not stack in game (e.g. now removed Prophecies). Note that `Class "Currency"` works because it uses substring-based matching by default - `Class == "Currency"` will fail.

Other:

- Quest items must be filtered with an explicit item class (`Class "Quest Items"`). This is intended, not a bug.
- Disabled Prophecies cannot drop and cannot be filtered. Also intended. As of 3.17 (Siege of the Atlas) prophecy content has been removed.

## bugs

No currently know bugs.

- ~~Game can issue "Your Item Filter is out of date" instead of "Failed to load Item Filter" in numerous cases of syntax errors.~~ **fixed in 3.8**
- ~~Fossils are classified as `Stackable Currency`. They do not actually stack in game.~~ **fixed in 3.7.4**: fossils now stack
- ~~Resonators can have 1-4 sockets and Item Filter detects that correctly. Jeweller's orb errors on resonators reporting that they have no sockets. Everyone knows why it is that but ... new players gonna be mad with such contradictory descriptions.~~ **Fixed in 3.8**: the error upon using Jeweller's Orb on a resonator is now: "Item has fixed Sockets".
- ~~`#` starts a comment untill end of line. `#` is not accepted in some contexts:~~ **fixed in 3.8**
- (not my list, might be outdated): https://github.com/xhul-github/xhul-filter/blob/master/misc.txt

```
# as of 3.8, nothing from this example code occurs

Show
	# similar thing happens for any condition
	# "#" is basically tried to be parsed as some number/keyword/string
	Class     "One"  # error: no item classes matching "#"
	ItemLevel 80     # error: integer "#" not recognised

	# actions:
	SetTextColor       240 200 150     # this comment works
	SetTextColor       240 200 150 255 # this comment works
	SetBorderColor     240 200 150     # this comment works
	SetBorderColor     240 200 150 255 # this comment works
	SetBackgroundColor 255   0   0     # this comment works
	SetBackgroundColor 255   0   0 255 # this comment works
	SetFontSize        42          # error: to many arguments
	MinimapIcon        0  Red Star # error: to many arguments
	PlayEffect         Yellow      # error: to many arguments
	DisableDropSound               # this comment works
	PlayAlertSound           1     # this comment works
	PlayAlertSound           1 300 # this comment works
	PlayAlertSoundPositional 1     # this comment works
	PlayAlertSoundPositional 1 300 # this comment works
	CustomAlertSound "airhorn.wav" # this comment works
```

- ~~generally, in many places text that should cause syntax errors is accepted because characters are skipped in some contexts. Optional `+` or `-` character is accepted for numbers (btw, negative quality would buff Blood of the Karui). Almost sure GGG uses string-to-int functions like `strtol()` with base = 10 which stop on first non-digit character. Null terminated strings were a huge mistake, but that's a completely different topic.~~ **fixed in 3.8**
- ~~custom alert sounds have a really buggy implementation (looks like very naive approach of searching for `"`):~~ **fixed in 3.8**:

```
# this is accepted and you may have an illusion that the volume works
# but everything after second " is completely ignored
# custom sounds have no volume support
# since 3.8: error: Incorrect format: CustomAlertSound [filepath]
	CustomAlertSound "pop.wav" 300

# as above, everything after closing " is skipped until end of line
# since 3.8: error: Incorrect format: CustomAlertSound [filepath]
	CustomAlertSound "pop.wav"GGG Fix filters PLZ

# expected: syntax error: missing path
# actual: plays pop on item drop like it was not a comment
# since 3.8: error: Incorrect format: CustomAlertSound [filepath]
	CustomAlertSound # "pop.wav"

# worked in the past, should fail after 3.13
	CustomAlertSound WHERE IS MY HH???"pop.wav"
```

- ~~negative numbers are accepted but `Quality > -3` does not catch anything (do they parse signed numbers to unsigned integer type?)~~ **fixed in 3.8**

## undocumented features

For some conditions, it is possible to specify multiple values (see quality examples below). I do not remember this feature being ever mentioned.

```
Show
	# will match an item with ilvl 10 or 20 or 30
	ItemLevel 10 20 30
	# will match an item with quality < 15 or < 10 or < 5
	Quality < 5 10 15
	# syntax error (> is not an integer)
	Quality < 7 > 11
	# <3.8: accepted, but doesn't actually work
	#  3.8: error: Integer "-3" may not be negative
	Quality > -3
```

expression             | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11
-----------------------|---|---|---|---|---|---|---|---|---|---|----|----
Quality < 5 7 9 11     | + | + | + | + | + | + | + | + | + | + | +  |
Quality < 11 9 7 5     | + | + | + | + | + | + | + | + | + | + | +  |
Quality < 7 11 7 9     | + | + | + | + | + | + | + | + | + | + | +  |
Quality < 4 8          | + | + | + | + | + | + | + | + |   |   |    |
Quality < 8 4          | + | + | + | + | + | + | + | + |   |   |    |
Quality 0 2 4 6 8      | + |   | + |   | + |   | + |   | + |   |    |
Quality +011           |   |   |   |   |   |   |   |   |   |   |    | +
Quality <= 0xB3X 4#pop | + | + | + | + | + |   |   |   |   |   |    |

## other observations

- In the past `SocketGroup` accepted only RGBW letters, however, when you ctrl+C an item in game abyss sockets were denoted with A and fossil sockets (on resonators) were denoted with D. Since patch 3.9.2f, filters also support abyss sockets and sockets on resonators.
- Filters were/are never meant to be backwards-compatible. Any future game patch can break existing filters. This has already happened in the past.
- You can specify `CustomAlertSound` and (`PlayAlertSound` and/or `PlayAlertSoundPositional`) at the same time
  - if custom alert is last, custom sound is played
  - ~~if custom alert is not last, nothing is played~~ **3.8**: now last alert sound is played
- You can repeat the same action, the last one wins except the situation in the point above.
- `""` is a valid string. It is consistent with other (non-empty) strings: rules that have empty `BaseType` strings will match all items because any item name also contains an empty string.
- It is possible to specify font size that is outside allowed range (\[18, 42\]). If so happens, the value is clamped in this range. No such thing happens for volume (must be in range \[0, 300\]).
- Game client assumes that the filter file is in unicode. [BOM](https://en.wikipedia.org/wiki/Byte_order_mark) is accepted but not required. ASNI encoding breaks where it differs from UTF-8 such as `ö` in `Maelström Staff`. If you get an error that some gibberish string could not be parsed convert file's encoding to UTF-8.
