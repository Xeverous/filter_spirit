# Filter Spirit tutorial

## preface

In order to write filters using FS well, you first need to understand what an actual filter can do (and what it can not).

If you have never been editing an actual filter, here are the related articles from wiki (not maintained by me):

- pathofexile.gamepedia.com/Item_filter
- pathofexile.gamepedia.com/Item_filter_guide

You don't have to understand and remember every detail - just the rough idea how it works.

Real filters have no access to following information:

- player level
- league
- zone level, although there is a strong correlation of `ItemLevel` of dropped items
- item full name after identification (only base type name)
- whether the item is mirrored
- whether the item has abyss sockets
- whether the item has been allocated to you (while playing in a party)
- whether the item has multiple variants (some uniques have)
- whether the item has any drop restrictions
- whether the item has alternate art
- whether the item has already been picked up and dropped again by a player
- whether the currently played character can equip the item

Additionally

- Some items can not be hidden (`Hide` will be ignored by the game). Read FAQ for more details.
- Many unique items share the same base type. So your next Inpulsa's drop is very likely to actually be Tinkerskin (both are `Sadist Garb`). This pretty much makes filtering some unique items impossible because multiple uniques share the same base (very common for belts, rings, amulets and jewels). There is no better way to workaround it other than having blocks for:
  - "bad drops" (base with only bad uniques)
  - "good drops" (base with only good uniques)
  - "potentially good drops" (base with at least 1 good unique)

Filter Spirit requires files in UTF-8 encoding. This is the same as for real filters. [UTF-8 everywhere.](http://utf8everywhere.org/)

## actual filter vs Filter Spirit

Filter Spirit is a tool that lets you write a filter using enchanced syntax and market information. It is important to note few key things:

- The enchanced-syntax-text-file is read by FS program. It is not modified and only used to generate an actual filter. This brings the possibility to refresh (redo the generation) which updates the base types of worthy items while maintaining the same filter structure.
- FS filters syntax is similar to the real filters only to make it easier to learn and understand how it works. It could use a completely different grammar but there would not be much point in it. New ideas welcome though.
- FS filters syntax IS NOT a superset of real filters. Copy-pasting a real filter to FS will almost always cause syntax errors. However, the convertion is pretty easy (read further).
- Because FS only generates real filters based on extra information, it can not do more than actual filters. FS exists mostly to save you time and error-prone manual work by reducing duplicated code and automating incorporation of market information.

## FS features

Core features of FS syntax are:

- Being able to name constants. No more copy-pasting the same color values - just write `color_money = RGB(213, 159, 15)` and reuse `color_money` whenever you want
- Being able to name sets of styles. Write `x = { SetFontSize 42 SetTextColor black }` and reuse in any block.
- Nesting of blocks. This is a very powerful feature allowing to significantly remove duplicate code and override styles for specific items.
- Querying item prices at generation time. `BaseType $divination(10, 100)` will query poe.ninja or poe.watch for divination cards in [10, 100) chaos range.
- Semantic analysis of filter code. FS implements a parser and compiler that checks for grammar and type errors. You can not end up with filters like `BaseType 42`.

## FS langauge

Unlike actual filters, FS skips any whitespace characters (tabs, spaces, line breaks), except when quoted (so `"Leather Belt"` is correctly parsed). This makes the language "immune" to any text formatting changes and lets you split long lines. Any ambiguities are resolved by lookahead or delimeters like `{}`.

Just like in actual filters, comments start with `#` and span until end of line.

### file structure

FS filter template consists of constant definitions which are then followed by blocks which may contain **actions**, **conditions** and nested blocks.

- **condition** - a requirement for the item to be matched. `ItemLevel > 3`, `Rarity unique` and such.
- **action** - the style applied to matched items. `SetFontSize 18`, `SetBeam blue` and such.
- Conditions and actions are mutually exclusive. There is no thing that is both at the same time.

### constants

Syntax:

```
<identifier> = <expression>
```

- Identifier is an alpha character (`a-z`, `A-Z` and `_`) optionally followed by a sequence of alphanumeric characters (alpha and `0-9`). Said differently, identifiers can not start with a digit.
- Identifiers that start with `_` are reserved (you should not use them) for future additions like predefined color names.
- Avoid identifiers which are keywords in common programming languages - don't name anything like `const`, `if`, `else`, `for`, `while`, `do`, `class`, `int` - otherwise you risk syntax errors in case FS gets some of these keywords implemented.
- Expression is another (previously defined) identifier or an expression made of built-in literals.

Examples:

```
top_currency = ["Exalted Orb", "Mirror of Kalandra", "Eternal Orb", "Albino Rhoa Feather", "Mirror Shard"]
font_max = 42
div_beam = blue
```

You can not redefine a constant:

```
# will cause a compiler error
x = 10
x = 100
```

### type system

Each value in FS language has an associated type.

When compiling the filter template, actions and conditions check that they have got values of appropriate types: something like `SetFontSize "abc"` will result in an error.

Don't worry if you can not grasp all of the following tables - this is more for reference than to remember. If something goes wrong in the generation process, you will be given a clear error.

`Suit` is named suit to avoid name conflict with `Color`. Since there are very few possible values, you can think of it as a suit of playing cards.

expression(s) | type | notes
--------------|------|------
`false`, `true` | Boolean |
`123` | Integer | leading zeros accepted
`3.14` | FloatingPoint | `.` required
`-1.2e-3` | FloatingPoint | syntax as defined by C99 or C++11 standard
`normal`, `magic`, `rare`, `unique` | Rarity |
`circle`, `diamond`, `hexagon`, `square`, `star`, `triangle` | Shape |
`red`, `green`, `blue`, `white`, `brown`, `yellow` | Suit |
`"abc"` | String | UTF-8 encoding, line breaking characters not allowed (LF and CR)
`["Leather Belt", "Sorcerer Boots"]`, `[10, 20, 30]` | Array | empty arrays are allowed

Some actions in actual filters use multiple tokens which require more complex types. These need to be created using built-in functions.

`Group` is not named `SocketGroup` to avoid name conflict with condition.

function | result type | notes
---------|-------------|------
`RGB(<Integer>, <Integer>, <Integer>)` | Color (RGB format) | can add more formats in the future
`RGB(<Integer>, <Integer>, <Integer>, <Integer>)` | Color (RGBA format: last value is opacity) | there is a planned feature to globally override opacity of all colors
`Path(<String>)` | Path | paths not validated: copied as-is to generated filter
`Level(<Integer>)` | Level |
`FontSize(<Integer>)` | FontSize |
`SoundId(<Integer>)` | SoundId |
`Volume(<Integer>)` | Volume |
`Group(<String>)` | Group | string may consist of only `R`, `G`, `B`, `W` characters
`Beam(<Suit>, <Boolean> = false)` | Beam (permanent by default) | second argument is optional
`MinimapIcon(<Integer>, <Suit>, <Shape>)` | MinimapIcon
`AlertSound(<SoundId | Integer>)` | AlertSound | built-in sound
`AlertSound(<SoundId | Integer>, <Volume | Integer>)` | AlertSound | built-in sound (wih specified volume)
`AlertSound(<SoundId | Integer>, <Boolean>)` | AlertSound | built-in sound (being positional or not)
`AlertSound(<SoundId | Integer>, <Volume | Integer>, <Boolean>)` | AlertSound | built-in sound (wih specified volume and being positional or not)
`AlertSound(<Path | String>)` | AlertSound | custom sound (note that filters do not support volume and positionality for custom sounds)

### blocks

**All constants must be defined before any actions/conditions.** Otherwise you will get parse errors.

Blocks in actual filters start with either `Show` or `Hide` while in FS these are **placed after all actions and conditions**.

**Conditions** are placed before braces and **actions** are placed inside the block. In other words, an item enters block (gets the style) if it matches **all** conditions.

<table>
<tr>
<th>FS template</th>
<th>generated filter</th>
</tr>
<tr>

<td>
<pre>
Class "Curency"
BaseType "Portal Scroll" {
	SetFontSize 30
	Show
}
</pre>
</td>

<td>
<pre>
Show
	Class "Curency"
	BaseType "Portal Scroll"
	SetFontSize 30
</pre>
</td>

</tr>
</table>


Nested blocks inherit all conditions and actions from the parent block. Actions can be overriden to customize more specific drops.

Nesting grammar can be understood this way:

```
statement = action OR block OR Show/Hide
block = conditions { statements }
```

<table>
<tr>
<th>FS template</th>
<th>generated filter</th>
</tr>
<tr>

<td>
<pre>
Class "Curency" {
	SetFontSize 36
<div></div>
	BaseType "Portal Scroll" {
		Show
	}
<div></div>
	BaseType "Orb" {
		# overwrites SetFontSize 36
		SetFontSize 42
		Show
	}
<div></div>
	Show
}
</pre>
</td>

<td>
<pre>
Show
	Class "Curency"
	BaseType "Portal Scroll"
	SetFontSize 36
<div></div>
Show
	Class "Currency"
	BaseType "Orb"
	SetFontSize 42
<div></div>
Show
	Class "Currency"
	SetFontSize 36
</pre>
</td>

</tr>
</table>

Actions must be always in the inner block. Further conditions must be after actions. In other words, you can not interleave actions and conditions in the same nesting depth.

<table>
<tr>
<th>FS template</th>
<th>output</th>
</tr>
<tr>

<td>
<pre>
Class "Gem"
SetFontSize 42 # action before {
Quality == 20
</pre>
</td>

<td>
<pre>
error something something parse failed
</pre>
</td>

</tr>
</table>

Each `Show`/`Hide` directly corresponds to a generation of a block. It collects all conditions and actions upwards giving priority to the closer ones. **If you don't place `Show`/`Hide` a block will not be generated.** If you place multiple `Show`/`Hide`, multiple blocks will be generated. This drives the typical filter structure: a lot of opening blocks at the beginning and then closing each of them with a Show/Hide statement. First blocks will match most specific items and then going down less and less specific ones.

<table>
<tr>
<th>FS template</th>
<th>generated filter</th>
</tr>
<tr>

<td>
<pre>
BaseType ["Gavel", "Stone Hammer", "Rock Breaker"] {
	SetBackgroundColor RGB(162, 85, 0)
<div></div>
	Rarity normal {
		Show
	}
<div></div>
	Rarity magic
	Quality > 12 {
		Show
	}
<div></div>
	Rarity rare
	Quality > 16 {
		Show
	}
<div></div>
	# no Show/Hide here: a block for a just hammer
	# (without quality or rarity conditions) is not generated
	# this might be useful if you would like the rest of hammers
	# to be catched later eg for a chromatic orb recipe
}
</pre>
</td>

<td>
<pre>
Show
        Rarity = Normal
        BaseType "Gavel" "Stone Hammer" "Rock Breaker"
        SetBackgroundColor 162 85 0
<div></div>
Show
        Quality > 12
        Rarity = Magic
        BaseType "Gavel" "Stone Hammer" "Rock Breaker"
        SetBackgroundColor 162 85 0
<div></div>
Show
        Quality > 16
        Rarity = Rare
        BaseType "Gavel" "Stone Hammer" "Rock Breaker"
        SetBackgroundColor 162 85 0
</pre>
</td>

</tr>
</table>

Nested blocks can only add more conditions. Specifying the same condition again is an error.

<table>
<tr>
<th>FS template</th>
<th>output</th>
</tr>
<tr>

<td>
<pre>
Class "Curency" {
	SetFontSize 30
<div></div>
	Class "Stackable Currency" {
		Show
	}
<div></div>
	Show
}
</pre>
</td>

<td>
<pre>
line 3: error: condition redefinition (the same condition can not be specified again in the same block or nested blocks)
                Class "Stackable Currency" {
                ~~~~~~~~~~~~~~~~~~~~~~~~~~
line 1: note: first defined here
        Class "Curency" {
        ~~~~~~~~~~~~~~~
</pre>
</td>

</tr>
</table>

An exception to this are conditions that have ranges. You can specify a bound again if it's a bound of a different side.

<table>
<tr>
<th>FS template</th>
<th>generated filter</th>
</tr>
<tr>

<td>
<pre>
Class "Gem"
Quality > 0 {
	# here we specify quality again but
	# since it's a different side it's OK
	Quality < 20 {
		SetFontSize 36
		Show
	}
<div></div>
	Show
}
</pre>
</td>

<td>
<pre>
Show
        Quality > 0
        Quality < 20
        Class "Gem"
        SetFontSize 36
<div></div>
Show
        Quality > 0
        Class "Gem"
</pre>
</td>

</tr>
</table>

Exact comparisons are treated as both ranges at once. So `Quality == 20` is treated like `Quality <= 20` and `Quality >= 20`. Since you can not narrow exact comparisons, anything more is an error.

<table>
<tr>
<th>FS template</th>
<th>output</th>
</tr>
<tr>

<td>
<pre>
Class "Gem"
Quality == 20 {
	# won't work, it's impossible
	Quality < 20 {
		SetFontSize 36
		Show
	}
<div></div>
	Show
}
</pre>
</td>

<td>
<pre>
line 4: error: upper bound redefinition (the same bound for comparison can not be specified again in the same block or nested blocks)
                Quality < 20 {
                ~~~~~~~~~~~~
line 2: note: first defined here
        Quality == 20 {
        ~~~~~~~~~~~~~
</pre>
</td>

</tr>
</table>

No conditions or actions are required. You can use condition-less blocks to limit scope of actions:

```
# nothing here
{
	SetFontSize 42

	# multiple blocks here
	# each will inherit font size 42
	# ...
}

# font size no longer works here
```

If you place an action before any block begins, it will apply globally to all blocks in the entire filter.

```
# If you don't like semi-transparent background
# that is by default in actual filters.
# This will add a truly opaque black background to
# every block in the filter (unless overriden).
SetBackgroundColor RGB(0, 0, 0, 255)
# Or maybe you don't like default font size
SetFontSize 40

# filter starts here...
Class "Currency" {
	# ...
}

# ...
```

## conditions

FS supports all conditions that the actual filters support. Some conditions allow values of multiple types.

- Conditions and Actions will automatically promote the value if possible so writing `SocketGroup Group("RGB")` or `SetAlertSound AlertSound("pop.wav")` is not necessary, `SocketGroup "RGB"` and `SetAlertSound "pop.wav"` is enough. **Explicit functions are only necessary when the condition/action consists of multiple values (eg color).**
- Numeric and `Rarity` conditions accept comparison operator - one of `<`, `>`, `<=`, `>=`, `=`. Operator is optional (defaults to `=`).
- String-based conditions accept optional `==` token which enables exact maching. `BaseType == "The Wolf"` will match only *The Wolf* card, `BaseType "The Wolf"` will match all *The Wolf*, *The Wolf's Legacy* and *The Wolf's Shadow*.

```
Rarity        [< | > | <= | >= | =] <Rarity>
ItemLevel     [< | > | <= | >= | =] <Level | Integer>
DropLevel     [< | > | <= | >= | =] <Level | Integer>
Quality       [< | > | <= | >= | =] <Integer>
Sockets       [< | > | <= | >= | =] <Integer>
LinkedSockets [< | > | <= | >= | =] <Integer>
Height        [< | > | <= | >= | =] <Integer>
Width         [< | > | <= | >= | =] <Integer>
StackSize     [< | > | <= | >= | =] <Integer>
GemLevel      [< | > | <= | >= | =] <Integer>
MapTier       [< | > | <= | >= | =] <Integer>

Class          [==] <String | Array>
BaseType       [==] <String | Array>
HasExplicitMod [==] <String | Array>
HasEnchantment [==] <String | Array>

SocketGroup <Group | String>

AnyEnchantment  <Boolean>
Identified      <Boolean>
Corrupted       <Boolean>
ElderItem       <Boolean>
ShaperItem      <Boolean>
FracturedItem   <Boolean>
SynthesisedItem <Boolean>
ShapedMap       <Boolean>
```

Examples:

```
Class "Currency"
BaseType ["Shard", "Splinter"]
BaseType == "The Wolf"
MapTier = 16
```

## actions

For consistency within FS, **some actions have been renamed and built-in alert sound has been merged with custom alert sounds** (which one it is is determined from the type of the argument).

All actions begin with `Set` except `PlayDefaultDropSound`.

The only types you have to create values of using functions are `Color` and `MinimapIcon` (they require multiple arguments).

```
SetBorderColor     <Color>
SetTextColor       <Color>
SetBackgroundColor <Color>

SetFontSize <Integer>

SetAlertSound <Sound | Integer | Path | String>

PlayDefaultDropSound <Boolean>

SetMinimapIcon <MinimapIcon>

SetBeam <Beam | Suit>
```

Some items always play specific built-in sounds, even if you don't specify anything. `DisableDropSound` in actual filters disables this behaviour. In FS, use `PlayDefaultDropSound` with a boolean value, eg `PlayDefaultDropSound false`.

Examples:

```
SetTextColor RGB(240, 200, 150)
SetBackgroundColor RGB(0, 0, 0, 255)
SetAlertSound "pop.wav"
SetAlertSound AlertSound(1, 300)
SetBeam green              # permanent beam
SetBeam Beam(green, false) # permanent beam, just verbose
SetBeam Beam(yellow, true) # temporary beam (the only possible syntax)
SetMinimapIcon MinimapIcon(0, blue, square)
```

## price queries

One of the core fancy features of FS.

All queries start with `$` (obviously, there is no better operator for price queries) followed by 2 function arguments: min and max price.

- Queries return an array of strings to put into `BaseType` condition.
- Minimum price is inclusive but maximum price is exclusive to facilitate range/iterator idioms. This way two ranges where one's upper bound is the same as other's lower bound do not overlap.
- A special value of `_` can be used to indicate no bound.
- You can use queries directly or assign their results to constants.

Available queries:

- `divination`
- `prophecies`
- `essences`
- `leaguestones`
- `fossils`
- `resonators`
- `scarabs`
- `incubators`
- `oils`
- `enchants`
- `uniques_eq_unambiguous`
- `uniques_eq_ambiguous`
- `uniques_flask_unambiguous`
- `uniques_flask_ambiguous`
- `uniques_jewel_unambiguous`
- `uniques_jewel_ambiguous`
- `uniques_map_unambiguous`
- `uniques_map_ambiguous`

Unique items are separated into 4 categories where each has ambiguous and unambiguous results:

- Unambiguous outputs base types for which there is only 1 unique item.
- Ambiguous outputs base types for which there is more than 1 unique item, using the highest price of possible unique drops on each base.
- These 2 never overlap in output.
- FS removes undroppable uniques (eg Harbinger and corruption-only uniques) from market information before outputting base types. This results in less ambiguities and removes items which you would not care for anyway when writing the filter.

Examples:

```
top_cards = $divination(150, _)

Class "Divination Card" {
	BaseType top_cards {
		SetAlertSound "maybe_doctor.wav"
		Show
	}

	# cards worth >= 100c and < 150c
	BaseType $divination(100, 150) {
		# ...
	}

	# cards worth >= 10c and < 100c
	BaseType $divination(10, 100) {
		# ...
	}

	BaseType $divination(0, 0.5) {
		Hide
	}

	Show
}
```

```
Rarity unique {
	SetTextColor brown

	# surely good drops
	BaseType $uniques_eq_unambiguous(100, _) {
		SetAlertSound 1
		Show
	}

	# potentially good drop
	BaseType $uniques_eq_ambiguous(100, _) {
		SetAlertSound sound_need_check
		Show
	}

	SetAlertSound sound_unique
	Show
}
```
