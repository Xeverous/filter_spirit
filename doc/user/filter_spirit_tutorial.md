# Filter Spirit tutorial

## preface

Just like real filters, Filter Spirit requires files in UTF-8 encoding. [UTF-8 everywhere.](http://utf8everywhere.org/)

This article assumes you have read `how_filters_work`. It only mentions filter extensions and differences in filter template syntax.

## actual filters vs FS filter templates

Filter Spirit is a tool that lets you write a filter using enhanced syntax and market information. It is important to note few key things:

- The enhanced-syntax-filter-template-file is read by FS program. It is not modified and only used to generate an actual filter. This brings the possibility to refresh (redo the generation) which updates the base types of worthy items while maintaining the same filter structure.
- FS filters syntax is similar to the real filters only to make it easier to learn and understand how it works. In theory, it could use a completely different grammar but in practice, there would not be much point in it. Ideas for new extensions welcome.
- FS filter templates use a declarative, not imperative language. It is not Turing-complete and therefore can not be labeled as a programming language.
- FS filters syntax IS NOT a superset of real filters. Copy-pasting a real filter to FS will almost always cause syntax errors. However, the convertion is pretty easy (you only need to move `Show` and `Hide`).
- Because FS only generates real filters based on extra information, it can not do more than actual filters. FS exists mostly to save you time and error-prone manual work by reducing duplicated code and automating incorporation of market information. It is not a game plugin or any sort of a hack, only a text file compiler/generator.

## FS features

Core features of FS syntax are:

- Being able to name constants. No more copy-pasting the same color values - just write `$color_money = 213 159 15` and reuse `$color_money` whenever you want.
- Being able to name sets of styles. Write `$x = { SetFontSize 42 SetTextColor $black }` and reuse in any block.
- Nesting of blocks. This is a very powerful feature allowing to significantly remove duplicate code and override styles for specific items.
- Querying item prices at generation time. `Autogen cards Price >= 100` will query poe.ninja or poe.watch for divination cards worth 100 or more chaos.
- Semantic analysis of filter code. FS implements a parser and compiler that checks for grammar and type errors. You can not end up with filters like `BaseType 42`.

## FS langauge

Similarly to actual filters, FS skips any whitespace characters (tabs, spaces, line breaks), except when quoted (so `"Leather Belt"` is correctly parsed). This makes the language "immune" to any text formatting changes and lets you split long lines (except in the case below). Any ambiguities are resolved by lookahead or delimeters like `{}`.

The only requirement is that you place 1 condition/action per line. Some of them accept an arbitrary amount of tokens (e.g. `BaseType` accepts 1 or more strings) so an end-of-line is expected to tell the parser where is the end of the rule.

Just like in actual filters, comments start with `#` and span until end of line. There are no restrictions where comments can appear.

### file structure

FS filter template consists of constant definitions which are then followed by blocks which may contain **actions**, **conditions** and nested blocks.

- **condition** - a requirement for the item to be matched. `ItemLevel > 3`, `Rarity Unique` and such.
- **action** - the style applied to matched items. `SetFontSize 18`, `SetBeam Blue` and such.
- Conditions and actions are mutually exclusive. There is no thing that is both at the same time.

### constants

Syntax:

```
$Identifier = Expression...
```

- Identifier is an alpha character (`a-z`, `A-Z` and `_`) optionally followed by a sequence of alphanumeric characters (alpha and `0-9`). Said differently, identifiers can not start with a digit.
- Identifiers that start with `_` are reserved (you should not use them) for future additions like predefined color names.
- Avoid identifiers which are keywords in common programming languages - don't name anything like `const`, `if`, `else`, `for`, `while`, `do`, `class`, `int` - otherwise you risk syntax errors in case FS gets some of these keywords implemented.
- Expression is another (previously defined) identifier or an expression made of built-in literals.
- A constant can store multiple values. Since filters do not use `()`, `[]` or even `,`, an end of line marks the end of constant definition. Place 1 constant per line.

Examples:

```
$top_currency = "Exalted Orb" "Mirror of Kalandra" "Eternal Orb" "Albino Rhoa Feather" "Mirror Shard"
$font_max = 42
$div_beam = Blue
```

You can not redefine a constant:

```
# will cause a compiler error
$x = 10
$x = 100
```

Values are automatically expanded (this is also known as "flattening"):

```
$red = 255
$green = 200
$blue = 170
$my_color = $red $green $blue 255 # same as $my_color = 255 200 170 255

$opacity = 220
$color_rgb = 255 255 255
$color_rgba = $color_rgb $opacity # same as $color_rgba = 255 255 255 220
```

### type system

Each value in FS language has an associated type.

When compiling the filter template, actions and conditions check that they have got values of appropriate types: something like `SetFontSize "abc"` will result in an error.

FS extends filter literals with additional ones:

expression(s) | type | notes
--------------|------|------
`3.14` | Fractional | `.` character is required
`-1.2e-3` | Fractional | scientific notation syntax as defined by C99 or C++11 standard

Note: game allows unquoted string literals (e.g. `abc` instead of `"abc"`) if they do not contain whitespace characters. FS does support this, but only when reading real filters. In filter templates, you must always write quoted string literals.

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
Class "Currency"
BaseType "Portal Scroll" {
	SetFontSize 30
	Show
}
</pre>
</td>

<td>
<pre>
Show
	Class "Currency"
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
Class "Currency" {
	SetFontSize 36
<div></div>
	BaseType "Portal Scroll" {
		Show
	}
<div></div>
	BaseType "Orb" {
		# overrides SetFontSize 36
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
	Class "Currency"
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
BaseType "Gavel" "Stone Hammer" "Rock Breaker" {
	SetBackgroundColor 162 85 0
<div></div>
	Rarity Normal {
		Show
	}
<div></div>
	Rarity Magic
	Quality > 12 {
		Show
	}
<div></div>
	Rarity Rare
	Quality > 16 {
		Show
	}
<div></div>
	# no Show/Hide here: a block for a just hammer
	# (without quality or rarity conditions) is not generated
	# this might be useful if you would like the rest of hammers
	# to be catched later e.g. for a chromatic orb recipe
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
Class "Currency" {
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
        Class "Currency" {
        ~~~~~~~~~~~~~~~~
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

No conditions or actions are required. You can use nesting feature alone to limit scope of actions:

```
# nothing here
{
	SetFontSize 42

	# multiple blocks here
	# each will inherit font size 42
	# ...
}

# font size 42 no longer applies here
```

If you place an action before any block begins, it will apply globally to all blocks in the entire filter.

```
# If you don't like semi-transparent background
# that is by default in actual filters.
# This will add a truly opaque black background to
# every block in the filter (unless overriden).
SetBackgroundColor 0 0 0 255
# Or maybe you don't like default font size
SetFontSize 40

# filter starts here...
Class == "Stackable Currency" {
	# ...
}

# ...
```

## statements inside constants

```
# applies predefined group of statements
Expand $identifier
```

You can assign multiple statements to an identifier and use the `Expand` keyword to apply all of them, as if they were written in the place of the `Expand` statement.

Later (and deeper nested) actions override previous ones.

<table>
<tr>
<th>FS template</th>
<th>generated filter</th>
</tr>
<tr>

<td>
<pre>
$x = 1 2 3
$y = 11 22 33
<div></div>
$actions1 = {
	SetBorderColor $x
	SetTextColor $x
}
<div></div>
$actions2 = {
	SetBorderColor $y
	SetTextColor $y
	SetFontSize 42
}
<div></div>
SetFontSize 36
Expand $actions1
<div></div>
Rarity rare {
	Expand $actions2
<div></div>
	Quality 20 {
		SetBackgroundColor 50 50 50
		Expand $actions1
		SetTextColor 100 100 100
		Show
	}
<div></div>
	Show
}
<div></div>
Show
</pre>
</td>

<td>
<pre>
Show
	Quality = 20
	Rarity = Rare
	SetBorderColor 1 2 3
	SetTextColor 100 100 100
	SetBackgroundColor 50 50 50
	SetFontSize 42
<div></div>
Show
	Rarity = Rare
	SetBorderColor 11 22 33
	SetTextColor 11 22 33
	SetFontSize 42
<div></div>
Show
	SetBorderColor 1 2 3
	SetTextColor 1 2 3
	SetFontSize 36
</pre>
</td>

</tr>
</table>

It is even possible to expand entire subtrees:

<table>
<tr>
<th>FS template</th>
<th>generated filter</th>
</tr>
<tr>

<td>
<pre>
$x = 11 22 33
$y = 1 2 3
<div></div>
$actions1 = {
	SetBorderColor $x
	SetTextColor $x
}
<div></div>
$actions2 = {
	SetBorderColor $y
	Expand $actions1
	SetTextColor $y
}
<div></div>
SetFontSize 36
Expand $actions2
Rarity Rare {
	Show
}
<div></div>
Show
</pre>
</td>

<td>
<pre>
Show
	Rarity = Rare
	SetBorderColor 11 22 33
	SetTextColor 1 2 3
	SetFontSize 36
<div></div>
Show
	SetBorderColor 11 22 33
	SetTextColor 1 2 3
	SetFontSize 36
</pre>
</td>

</tr>
</table>

<table>
<tr>
<th>FS template</th>
<th>generated filter</th>
</tr>
<tr>

<td>
<pre>
$color_chromatic_small = 175 255 200
$color_chromatic_big   = 128 128 192
<div></div>
$chrome_item_logic = {
	SocketGroup RGB {
		SetBorderColor $color_chromatic_small
<div></div>
		Width 2
		Height <= 2 {
			Show
		}
<div></div>
		Width 1
		Height <= 4 {
			Show
		}
<div></div>
		SetBorderColor $color_chromatic_big
		Show
	}
<div></div>
	Show
}
<div></div>
Rarity Normal {
	SetTextColor 200 200 200
	Expand $chrome_item_logic
}
<div></div>
Rarity Magic {
	SetTextColor 136 136 255
	Expand $chrome_item_logic
}
<div></div>
Rarity Rare {
	SetTextColor 255 255 119
	Expand $chrome_item_logic
}
</pre>
</td>

<td>
<pre>
Show
	Rarity = Normal
	Height <= 2
	Width = 2
	SocketGroup = RGB
	SetBorderColor 175 255 200
	SetTextColor 200 200 200
<div></div>
Show
	Rarity = Normal
	Height <= 4
	Width = 1
	SocketGroup = RGB
	SetBorderColor 175 255 200
	SetTextColor 200 200 200
<div></div>
Show
	Rarity = Normal
	SocketGroup = RGB
	SetBorderColor 128 128 192
	SetTextColor 200 200 200
<div></div>
Show
	Rarity = Normal
	SetTextColor 200 200 200
<div></div>
Show
	Rarity = Magic
	Height <= 2
	Width = 2
	SocketGroup = RGB
	SetBorderColor 175 255 200
	SetTextColor 136 136 255
<div></div>
Show
	Rarity = Magic
	Height <= 4
	Width = 1
	SocketGroup = RGB
	SetBorderColor 175 255 200
	SetTextColor 136 136 255
<div></div>
Show
	Rarity = Magic
	SocketGroup = RGB
	SetBorderColor 128 128 192
	SetTextColor 136 136 255
<div></div>
Show
	Rarity = Magic
	SetTextColor 136 136 255
<div></div>
Show
	Rarity = Rare
	Height <= 2
	Width = 2
	SocketGroup = RGB
	SetBorderColor 175 255 200
	SetTextColor 255 255 119
<div></div>
Show
	Rarity = Rare
	Height <= 4
	Width = 1
	SocketGroup = RGB
	SetBorderColor 175 255 200
	SetTextColor 255 255 119
<div></div>
Show
	Rarity = Rare
	SocketGroup = RGB
	SetBorderColor 128 128 192
	SetTextColor 255 255 119
<div></div>
Show
	Rarity = Rare
	SetTextColor 255 255 119
</pre>
</td>

</tr>
</table>

Due to flattening implementation, direct assignment of subtrees like `$st2 = $st1` is not possible (it will error that the value on the right has incorrect type). Write `$st2 = { Expand $st1 }` instead.

## condition extensions

FS offers extra conditions that automatically fill `BaseType` condition with item names downloaded from economy APIs. More on autogenerated block further in this article.

```
# FS autogeneration extensions
Autogen         String
Price           [CMP] Integer
```

### `None` in array-based conditions

`None` in array conditions (other than `HasInfluence`) is a FS extension. They are skipped. Useful when the value comes from elsewhere - e.g. in code like `$chance_bases = None [...] BaseType $chance_bases`. Multiple variables may be used within one condition. If a block gets only `None`s it will not be generated.

```
Class                  [EQ] (None | String)+
BaseType               [EQ] (None | String)+
EnchantmentPassiveNode [EQ] (None | String)+
ArchnemesisMod         [EQ] (None | String)+

HasExplicitMod         [==] | CMPInteger (None | String)+
HasEnchantment         [==] | CMPInteger (None | String)+
```

## action extensions

```
# this action accepts both PlayAlertSound and
# CustomAlertSound tokens. Useful when you want
# to use constants because the constant can be
# freely changed what sound type it refers to.
SetAlertSound          (String | Integer | ShaperVoiceLine) [Integer]
```

## autogeneration

One of the core fancy features of FS.

Write a block just as you would normally write it, but add an `Autogen <query_name>` condition. Autogeneration will place strings into `BaseType ==` condition, based on data provided by websites such as poe.ninja or poe.watch.

- You need to select a specific league when downloading the prices. If you play on multiple leagues, generate a filter for each.
- The `Class` condition must either not exist or match autogenerated item class. Some autogenerations will also check for other conditions that would conflict with autogenerated items (such as `GemLevel` for `currency`).
- You can limit what items appear using `Price` condition. This condition accepts fractional values which represent reported item value in Chaos Orbs.
- You can limit what items appear using `BaseType` condition. The resulting `BaseType` condition will only contain items that both match specified names and other conditions.
- The filter should be prepared that some items may not be reported due to a lack of information (which is especially true at the league start) or too low confidence. This means that if you are using an autogenerated set of blocks for Divination Cards, there should be at least 1 non-autogenerated block that catches any card and highlights it as "item with unknown value / potentially valuable".
- While some items have a very clear market value, it is impossible to filter them. Unique jewels are a primary example - since the filter only sees item base type, it is impossible to determine which unique it is. In such cases you need to treat every drop as potentially valuable.

Available queries:

- `currency`
- `fragments`
- `delirium_orbs`
- `cards`
- `essences`
- `fossils`
- `resonators`
- `scarabs`
- `incubators`
- `oils`
- `vials`
- `gems`
- `bases`
- `uniques_eq_unambiguous`
- `uniques_eq_ambiguous`
- `uniques_flasks_unambiguous`
- `uniques_flasks_ambiguous`
- `uniques_jewels_unambiguous`
- `uniques_jewels_ambiguous`
- `uniques_maps_unambiguous`
- `uniques_maps_ambiguous`

Unique items are separated into 4 categories where each has ambiguous and unambiguous results:

- `eq` are all equippable items (weapons, shields, all armour parts, belts, amulets and rings)
- Unambiguous outputs base types for which there is only 1 unique item. This is intended for blocks where it is known what unique dropped.
- Ambiguous outputs base types for which there is more than 1 unique item, using the highest price of possible unique drops on each base. This is intended for blocks where it is unknown what unique dropped (FS assumes potentilly expensive one).
- Unambiguous and Ambiguous never overlap in output.
- FS removes undroppable uniques (e.g. Harbinger and vendor-only uniques) from market information before outputting base types. This results in less ambiguities and removes items which you would not care for anyway when writing the filter. Corruption-only uniques are not removed because 1) poe.watch and poe.ninja differ in their reports or searches 2) maps with Corrupting Tempest and some sextant mods can drop corrupted items.
- It is recommended that you never hide any uniques. Names in `unambiguous` autogenerations are there only because used API reported 1 unique for specific base type. There is still a possibility that another unique item exists on the same base, which can drop but is not reported on any trade API.

Some autogenerations place additional requirements:

- `gems`:
  - `GemLevel` condition must be present. The stricter it is, the better.
  - `Quality` condition must be present. The stricter it is, the better.
  - `Corrupted` condition must be present.
- `bases`:
  - `ItemLevel` condition must be present. The stricter it is, the better.
  - `HasInfluence ==` condition must be present.
  - `Corrupted` condition must either not exist or allow `False`.
  - `Mirrored` condition must either not exist or allow `False`.
  - `Rarity` condition must either not exist or allow `Normal`, `Magic` and `Rare`.
- `uniques_*`:
  - `Rarity` condition must either not exist or allow `Unique`.

Some implementation notes:

- `scarabs` also report bestiary lures (both have class `"Map Fragments"`)
- **Unique items are currently not supported**. Planned for reimplementation on a more robust algorithm, taking into consideration all aspects (base, sockets, ilvl, whether all uniques on specific base are cheap etc.).

Examples:

```
Class == "Divination Card"
{
	# cards that you always want to pickup (usually low-value cards
	# that have 1/1 stack or grant currency when completed)
	BaseType == ... {
		# ...
		Show
	}

	Autogen "cards"
	{
		Price > 150
		{
			# ...
			Show
		}

		Price <  150
		Price >= 100
		{
			# ...
			Show
		}

		Price <  100
		Price >= 10
		{
			# ...
			Show
		}

		Price <  10
		Price >= 0.5
		{
			# ...
			Show
		}

		Price < 0.5
		{
			Hide
		}

	}

	# any remaining cards that did not appear in market data
	# highlight these items as potentially valuable

	# ...
	Show
}
```

```
Rarity Unique
{
	SetTextColor $brown

	# surely good drops
	Autogen "uniques_eq_unambiguous"
	Price > 100
	{
		SetAlertSound 1
		Show
	}

	# potentially good drops
	Autogen "uniques_eq_ambiguous"
	Price > 100
	{
		SetAlertSound $sound_need_check
		Show
	}

	# remaining uniques
	SetAlertSound $sound_unique
	Show
}
```

```
Class == "Stackable Currency"
{
	SetTextColor $yellow

	# show only valuable catalysts
	Autogen "currency"
	BaseType "Catalyst"
	Price >= 0.5
	{
		Show
	}

	...
}
```

## continuation

Works same way as in real filters. `Continue` should appear below `Show`/`Hide` keywords.

<table>
<tr>
<th>FS template</th>
<th>generated filter</th>
</tr>
<tr>

<td>
<pre>
SetFontSize 30
<div></div>
DropLevel >= 10 {
	SetFontSize 35
<div></div>
	ItemLevel < 70 {
		Hide
		Continue
	}
<div></div>
	Rarity >= Rare
	ItemLevel >= 70 {
		SetFontSize 40
		Show
		Continue
	}
<div></div>
	Sockets = 6 {
		PlayEffect Blue
		Show
		Continue
	}
<div></div>
	Show
	Continue
}
<div></div>
Hide
</pre>
</td>

<td>
<pre>
Hide
	ItemLevel < 70
	DropLevel >= 10
	SetFontSize 35
	Continue
<div></div>
Show
	ItemLevel >= 70
	DropLevel >= 10
	Rarity >= Rare
	SetFontSize 40
	Continue
<div></div>
Show
	DropLevel >= 10
	Sockets = 6
	SetFontSize 35
	PlayEffect Blue
	Continue
<div></div>
Show
	DropLevel >= 10
	SetFontSize 35
	Continue
<div></div>
Hide
	SetFontSize 30
</pre>
</td>

</tr>
</table>

## `Show`/`Hide` extensions

### preface

Filter strictness variants. A feature a lot of players want, but it is really hard to propose a specific implementation and provide arguments why it is the best implementation. I have wondered for a long time how to implement such thing in FS and it only became apparent to me once I understood few very important things.

#### important thing 1: strictness is not a matter of subsets

It is wrong to think that a very strict filter / strict filter / normal filter are just super/subsets. Many players have different opinion on what is valuable and many players use different strategies and/or do different content. Thus, the value of an item can be very subjective.

A much better approach is to allow to mix multiple, independent settings so that you can selectively enable/disable things such as 5L, veiled items, RGB items, hammers, low-value currency, etc.

From this important thing, I knew that strictness in FS must allow to cherry-pick changes, definitely not something like "global strictness level".

#### important thing 2: there are 3 ways how to deal with an item

Initially you probably thought there are only 2 (`Show` and `Hide`). Consider such real filter code:

```
# hammers for chisel recipe
Show
	BaseType == "Gavel" "Stone Hammer" "Rock Breaker"
	# ... some styles ...

Show
	LinkedSockets 5
	# ... some styles ...

Show
	Sockets 6
	# ... some styles ...

# chromatic orb recipe
Show
	SocketGroup RGB
	# ... some styles ...
```

It seems that changing such filter to a higher strictness is only a matter of changing some `Show` to `Hide`. **But this is very wrong:**

- If block for hammers is changed to `Hide`, this means that every hammer will be hidden, no matter other properties. You might lose some RGB hammers that you would use for Chromatic Orb recipe.
- If block for 5L is changed to `Hide`, this means that every 5L will be hidden, no matter other properties. You might lose some 5L6s items that you would vendor for Jeweller's Orbs.

Many more examples are possible. In short, **it is wrong to always use `Hide` for higher strictness - many items can have multiple interesting properties and they should not be hidden if one is unwanted**.

Sometimes you really want to hide the item (e.g. `Engineer's Shard`). But in more complex situations, you want to deal with an item in the 3rd way: **remove the block from the filter**.

Thus, the 3 ways are:

- `Show`
- `Hide`
- remove the block so that the item goes further through the filter

#### important thing 3: to `Hide` or to ignore

The decision is item specific, but **for each item, there is only 1 good answer**. There are 2 scenarios:

- A very one-dimentional item (e.g. specific currency) in such case, after the filter recognized item's `BaseType` it is immediately known what is the item's value.
  - Normal filter: `Show`
  - Strict filter: `Hide`
- A multi-dimentional item (e.g. equippable gear) which can have multiple interesting properties. In such case the filter should have multiple blocks in decreasing priority order, each block:
  - Normal filter: `Show`
  - Strict filter: the block should not exist so that other blocks can catch it
  - The last block, with lowest priority (usually most generic condition) always shows or always hides.

### the feature

2 Item visibility extensions are available:

```
ShowHide    Boolean+
ShowDiscard Boolean+
```

- `ShowHide`:
  - if all supplied values are `True`, the block is generated as if it was a `Show` statement
  - otherwise, the block is generated as if it was a `Hide` statement
- `ShowDiscard`:
  - if all supplied values are `True`, the block is generated as if it was a `Show` statement
  - otherwise, the block is not generated, as if there was no statement

With this feature, you can have configuration variables that can be easily edited between `True` and `False` which will correctly handle each case.

Example:

```
$show_rares = True
$show_hammers = True

BaseType "Gavel" "Stone Hammer" "Rock Breaker" {
	SetTextColor 255 255 255
	SetBackgroundColor 162 85 0

	Rarity Normal {
		 ShowDiscard $show_hammers
	}

	Rarity Magic
	Quality > 12 {
		 ShowDiscard $show_hammers
	}

	Rarity Rare
	Quality > 16 {
		 ShowDiscard $show_hammers
	}
}

Rarity Rare {
	ShowHide $show_rares
}
```

Generated filter:

```
Show
	Rarity = Normal
	BaseType "Gavel" "Stone Hammer" "Rock Breaker"
	SetTextColor 255 255 255
	SetBackgroundColor 162 85 0

Show
	Quality > 12
	Rarity = Magic
	BaseType "Gavel" "Stone Hammer" "Rock Breaker"
	SetTextColor 255 255 255
	SetBackgroundColor 162 85 0

Show
	Quality > 16
	Rarity = Rare
	BaseType "Gavel" "Stone Hammer" "Rock Breaker"
	SetTextColor 255 255 255
	SetBackgroundColor 162 85 0

Show
	Rarity = Rare
```

Generated filter with `$show_hammers = False`:

```
Show
	Rarity = Rare
```

Example with multiple values:

```
$leveling = True
$caster = True
$leveling_caster = $leveling $caster

# both statements semantically equivalent
ShowDiscard $leveling $caster
ShowDiscard $leveling_caster
```
