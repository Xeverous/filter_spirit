# how it works

This article consists of 2 main parts:

- how actual filters work (the one that is loaded by the game client)
- how Filter Spirit enchances filter language and how it is translated to actual filter

# actual filter

In order to write spirit filters well, you first need to understand what an actual filter can do (and what it can not).

Related articles on wiki (not maintained by me):

- pathofexile.gamepedia.com/Item_filter
- pathofexile.gamepedia.com/Item_filter_guide

Please be aware that actual filters have no access / information of:

- player level
- league
- zone level, although there is a strong correlation of `ItemLevel` of dropped items
- item full name (after identification) - this pretty much makes filtering unique jewels impossible because all the filter sees are Crimson, Viridian, Cobalt and Prismatic jewels
- whether the item is mirrored (TODO `HasExplicitMod "Mirrored"` ???)
- whether the item has abyss sockets
- whether the item has been allocated to you (while playing in a party)
- whether the item has multiple variants (some uniques have)
- whether the item has alternate art

Additionally

- Some items can not be hidden (`Hide` will be ignored). Read FAQ for more details.
- Many unique items share the same base type. So `Leather Belt` at most, you will have to check it.

# FS templates

## syntax

FS enchances filter syntax with strong typing (supporting arrays and compound types) and additional operators. These are explained further in the article - first get familar with how blocks work.

## blocks

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


Nested blocks inherit all conditions and actions from the parent block. Nested blocks can override actions.

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

Actions must be always in the inner block. Further conditions must be after actions. In other words, you can not interleave actions and conditions in the same scope

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

Each `Show`/`Hide` directly corresponds to a generation of a block. **If you don't place `Show`/`Hide` a block will not be generated.**

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
<th>generated filter</th>
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
## types

Each value in FS language has an associated type.


When compiling the filter template, actions and conditions check that they have got values of appropriate types: something like `SetFontSize "abc"` will result in an error.

`Suit` is named suit to avoid name conflict with `Color`. Since there are very few possible values, you can think of it as a suit of playing cards.

expression(s) | type
--------------|-----
`false`, `true` | Boolean
`123` | Integer
`3.14` | FloatingPoint
`normal`, `magic`, `rare`, `unique` | Rarity
`circle`, `diamond`, `hexagon`, `square`, `star`, `triangle` | Shape
`red`, `green`, `blue`, `white`, `brown`, `yellow` | Suit
`"abc"` | String
`["Leather Belt", "Sorcerer Boots"]` | Array
`[10, 20, 30]` | Array

Complex types need to be created using built-in functions.

`Group` is not named `SocketGroup` to avoid name conflict with condition.

function | result type
---------|------------
`RGB(<Integer>, <Integer>, <Integer>)` | Color (RGB format)
`RGB(<Integer>, <Integer>, <Integer>, <Integer>)` | Color (RGBA format: last value is opacity)
`Path(<String>)` | Path
`Level(<Integer>)` | Level
`FontSize(<Integer>)` | FontSize
`SoundId(<Integer>)` | SoundId
`Volume(<Integer>)` | Volume
`Group(<String>)` | Group
`Beam(<Suit>, <Boolean> = false)` | Beam (permanent by default)
`MinimapIcon(<Integer>, <Suit>, <Shape>)` | MinimapIcon

TODO built-in/custom alert sound type?

## conditions

FS supports all conditions that the actual filters support. Some conditions allow values of multiple types. Conditions will automatically promote the value if possible so writing `SocketGroup Group("RGB")` is not necessary, `SocketGroup "RGB"` is enough.

Operator is one of `<`, `>`, `<=`, `>=`, `==`. (`!=` is not yet supported). Operator is optional (defaults to `==`).

```
Rarity        [operator] <Rarity>
ItemLevel     [operator] <Level>
DropLevel     [operator] <Level>
Quality       [operator] <Integer>
Sockets       [operator] <Integer>
LinkedSockets [operator] <Integer>
Height        [operator] <Integer>
Width         [operator] <Integer>
StackSize     [operator] <Integer>
GemLevel      [operator] <Integer>
MapTier       [operator] <Integer>

Class    [operator] <String | Array>
BaseType [operator] <String | Array>

SocketGroup <Group | String>

HasExplicitMod <String>
HasEnchantment <String>

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
MapTier == 16
```

## actions

For consistency with itself, **some actions have been renamed and built-in alert sound has been merged with custom alert sounds** (which one it is is determined from the type of the argument).

All actions begin with `Set` except `DisableDropSound`.

The only types you have to create values of using functions are `Color` and `MinimapIcon` (they require multiple arguments).

```
SetBorderColor     <Color>
SetTextColor       <Color>
SetBackgroundColor <Color>

SetFontSize <Integer>

SetAlertSound <Sound | Integer | Path | String>

DisableDropSound

SetMinimapIcon <MinimapIcon>

SetBeam <Beam | Suit>
```

Examples:

```
SetTextColor
```

## constants

FS adds the feature of being able to name constants.

```
const <identifier> = <expression>
```

Examples:

```
const currency_t0 = ["Exalted Orb", "Mirror of Kalandra", "Eternal Orb", "Albino Rhoa Feather", "Mirror Shard"]
const red = RGB(255, 0, 0)
const white = RGB(255, 255, 255)
const icon_currency = MinimapIcon(0, yellow, star)

# ...

Class "Currency" {
	BaseType currency_t0 {
		SetBorderColor red
		SetTextColor red
		SetBackgroundColor white
		SetMinimapIcon icon_currency
	}
}
```

**All constants must be defined before any actions/conditions.** Otherwise you will get parse errors.

## price queries

One of the core fancy features of FS.

All queries start with `$` (obviously, there is no better operator for price queries) and work like functions: they return a value of certain type.


```
        minimum price (inclusive)
               ~~~~~~~~~~~~~
$<identifier>(<FloatingPoint>, <FloatingPoint>)
  ~~~~~~~~~~                    ~~~~~~~~~~~~~
  query name               maximum price (exclusive)
```
You can use queries directly or assign their results to constants.

```
const top_cards = $divination(100, 999999)

Class "Divination Card" {
	BaseType top_cards {
		SetAlertSound "maybe_doctor.wav"
		Show
	}

	BaseType $divination(0, 0.5) {
		Hide
	}
}
```

Available queries:

name | result type | description
-----|-------------|------------
divination | `Array` (of `String`s) | divination cards
prophecies | `Array` (of `String`s) | sealed prophecies
fossils | `Array` (of `String`s) | fossils
vials | `Array` (of `String`s) | Incursion vials (drop from Omnitect)
essences | `Array` (of `String`s) | essences
scarabs | `Array` (of `String`s) | scarabs
uniques | `Array` (of `String`s) | base types of all unique items (merged set of 2 below)
uniques_unambiguous | `Array` (of `String`s) | base types of unique items that have only 1 unique
uniques_ambiguous | `Array` (of `String`s) | base types of unique items that have multiple uniques
uniques_armour | `Array` (of `String`s) | base types of all armour (body armour, helmet, gloves, boots) unique items (merged set of 2 below)
uniques_unambiguous_armour | `Array` (of `String`s) | base types of armour unique items that have only 1 unique
uniques_ambiguous_armour | `Array` (of `String`s) | base types of armour unique items that have multiple uniques
uniques_weapon | `Array` (of `String`s) | base types of all weapon unique items (merged set of 2 below)
uniques_unambiguous_weapon | `Array` (of `String`s) | base types of weapon unique items that have only 1 unique
uniques_ambiguous_weapon | `Array` (of `String`s) | base types of weapon unique items that have multiple uniques
uniques_accessory | `Array` (of `String`s) | base types of all accessory (rings, amulets, belts) unique items (merged set of 2 below)
uniques_unambiguous_accessory | `Array` (of `String`s) | base types of accessory unique items that have only 1 unique
uniques_ambiguous_accessory | `Array` (of `String`s) | base types of accessory unique items that have multiple uniques
uniques_flask | `Array` (of `String`s) | base types of all flask unique items (merged set of 2 below)
uniques_unambiguous_flask | `Array` (of `String`s) | base types of flask unique items that have only 1 unique
uniques_ambiguous_flask | `Array` (of `String`s) | base types of flask unique items that have multiple uniques
uniques_jewels | `Array` (of `String`s) | uniques_ambiguous_jewels
uniques_unambiguous_jewels | `Array` (of `String`s) | I guess nothing?
uniques_ambiguous_jewels | `Array` (of `String`s) | `["Crimson Jewel", "Cobalt Jewel", "Viridian Jewel", "Prismatic Jewel"]` unless no unique of each was put on sale (which never happens)
uniques_maps | `Array` (of `String`s) | base types of all map unique items (merged set of 2 below)
uniques_unambiguous_jewels | `Array` (of `String`s) | base types of map unique items that have only 1 unique
uniques_ambiguous_jewels | `Array` (of `String`s) | nothing? are there any maps bases with multiple uniques?

TODO shields where?

TODO do uniques really need split for armour/accessory/weapon etc? They are not used in chaos recipe.

What to do with:

- enchants (how they are even priced?)
- gems
- bases
- harbinger pieces