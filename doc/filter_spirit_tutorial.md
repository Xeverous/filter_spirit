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
- item full name after identification (only base type name)
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

Filter Spirit is a tool that lets you write a filter using enhanced syntax and market information. It is important to note few key things:

- The enhanced-syntax-filter-template-file is read by FS program. It is not modified and only used to generate an actual filter. This brings the possibility to refresh (redo the generation) which updates the base types of worthy items while maintaining the same filter structure.
- FS filters syntax is similar to the real filters only to make it easier to learn and understand how it works. It could use a completely different grammar but there would not be much point in it. Ideas for new extensions welcome.
- FS filters syntax IS NOT a superset of real filters. Copy-pasting a real filter to FS will almost always cause syntax errors. However, the convertion is pretty easy (read further).
- Because FS only generates real filters based on extra information, it can not do more than actual filters. FS exists mostly to save you time and error-prone manual work by reducing duplicated code and automating incorporation of market information.

## FS features

Core features of FS syntax are:

- Being able to name constants. No more copy-pasting the same color values - just write `$color_money = 213 159 15` and reuse `$color_money` whenever you want
- Being able to name sets of styles. Write `$x = { SetFontSize 42 SetTextColor $black }` and reuse in any block.
- Nesting of blocks. This is a very powerful feature allowing to significantly remove duplicate code and override styles for specific items.
- Querying item prices at generation time. `Autogen cards Price >= 100` will query poe.ninja or poe.watch for divination cards worth 100 or more chaos.
- Semantic analysis of filter code. FS implements a parser and compiler that checks for grammar and type errors. You can not end up with filters like `BaseType 42`.

## FS langauge

Similarly to actual filters, FS skips any whitespace characters (tabs, spaces, line breaks), except when quoted (so `"Leather Belt"` is correctly parsed). This makes the language "immune" to any text formatting changes and lets you split long lines (except in the case below). Any ambiguities are resolved by lookahead or delimeters like `{}`.

The only requirement is that you place 1 condition/action per line. Some of them accept an arbitrary amount of tokens (eg `BaseType` accepts 1 or more strings) so an end-of-line is expected to tell the parser where is the end of the rule.

Just like in actual filters, comments start with `#` and span until end of line.

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

Don't worry if you can not grasp all of the following tables - this is more for reference than to remember. If something goes wrong in the generation process, you will be given a clear error.

`Suit` is named suit to avoid name conflict with `Color`. Since there are very few possible values, you can think of it as a suit of playing cards.

expression(s) | type | notes
--------------|------|------
`None` | NoneType | use to clear a condition or assign nothing
`Temp` | TempType | optional token for some actions
`False`, `True` | Boolean |
`123` | Integer | leading zeros accepted (`0123` is equal to `123`)
`3.14` | Fractional | `.` character is required
`-1.2e-3` | FloatingPoint | scientific notation syntax as defined by C99 or C++11 standard
`RGB`, `3GGG`, `AA` | SocketSpec | `W` - white socket, `A` - abyss socket, `D` - resonator socket
`Normal`, `Magic`, `Rare`, `Unique` | Rarity |
`Circle`, `Diamond`, `Hexagon`, `Square`, `Star`, `Triangle`, `Cross`, `Moon`, `Raindrop`, `Kite`, `Pentagon`, `UpsideDownHouse` | Shape |
`Red`, `Green`, `Blue`, `White`, `Brown`, `Yellow`, `Cyan`, `Grey`, `Orange`, `Pink`, `Purple` | Suit |
`ShMirror`, `ShExalted`, `ShDivine`, `ShGeneral`, `ShRegal`, `ShChaos`, `ShFusing`, `ShAlchemy`, `ShVaal`, `ShBlessed` | ShaperVoiceLine
`Shaper`, `Elder`, `Crusader`, `Redeemer`, `Hunter`, `Warlord` | Influence |
`"abc"` | String | UTF-8 encoding, line breaking characters (LF and CR) not allowed between quotes

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
Class "Currency" {
	# ...
}

# ...
```

## conditions

FS supports all conditions that the actual filters support. Some conditions allow values of multiple types.

- Place 1 condition per line.
- Numeric and `Rarity` conditions accept comparison operator - one of `<`, `>`, `<=`, `>=`, `=`. Operator is optional (defaults to `=`).
- String-based conditions accept optional `==` token which enables exact maching. `BaseType == "The Wolf"` will match only *The Wolf* card, `BaseType "The Wolf"` will match all *The Wolf*, *The Wolf's Legacy* and *The Wolf's Shadow*.
- `HasInfluence` condition behaves differently with `==`:
  - If `==` is not present, it will match an item **with at least one of specified influences**.
  - If `==` is present, it will only match items **with all specified influences**.

Grammar overview (not a normative documentation as actual EBNF grammar specifications are much harder to read):

```
X       # X token is required
[X]     # X token is optional
X | Y   # X or Y token
X...    # 0 or more X tokens
X X...  # 1 or more X tokens

# token CMP: < | > | <= | >= | =
# token SS: [Integer] R...G...B...W...A...D...
```

```

Rarity        [CMP] Rarity
ItemLevel     [CMP] Integer
DropLevel     [CMP] Integer
Quality       [CMP] Integer
LinkedSockets [CMP] Integer
Height        [CMP] Integer
Width         [CMP] Integer
StackSize     [CMP] Integer
GemLevel      [CMP] Integer
MapTier       [CMP] Integer
AreaLevel     [CMP] Integer
CorruptedMods [CMP] Integer

Class          [==] String String...
BaseType       [==] String String...
HasExplicitMod [==] String String...
HasEnchantment [==] String String...
Prophecy       [==] String String...
HasInfluence   [==] Influence Influence...

Sockets     [CMP | ==] SS SS...
SocketGroup [CMP | ==] SS SS...

AnyEnchantment  Boolean
Identified      Boolean
Corrupted       Boolean
Mirrored        Boolean
ElderItem       Boolean
ShaperItem      Boolean
FracturedItem   Boolean
SynthesisedItem Boolean
ShapedMap       Boolean
ElderMap        Boolean
BlightedMap     Boolean

# FS autogeneration extensions - explained further down this article
Autogen         Identifier
Price           [CMP] Integer
```

Examples:

```
Class "Currency"
BaseType "Shard" "Splinter"
BaseType == "The Wolf"
MapTier = 16
Sockets 6
Sockets >= 4RR
SocketGroup >= 1 2 3R AA 3D 5GBW
```

## actions

The optional 4th value for colors is the opacity value. `0` means fully transparent, `255` fully opaque. The default opacity is unknown but it is somewhat transparent.

```
SetBorderColor           Integer Integer Integer [Integer]
SetTextColor             Integer Integer Integer [Integer]
SetBackgroundColor       Integer Integer Integer [Integer]

SetFontSize              Integer

# first token is built-in sound ID, second is volume
PlayAlertSound           (Integer | ShaperVoiceLine) [Integer]
PlayAlertSoundPositional (Integer | ShaperVoiceLine) [Integer]

# path can be absolute or just the file name
# note: custom sounds do not support specifying volume
CustomAlertSound         String

# Some items always play specific built-in sounds,
# even if you don't specify anything.
# This action disables such behavior.
DisableDropSound

SetMinimapIcon           Integer Suit Shape

PlayEffect               Suit [Temp]

# FS extension - applies predefined group of actions
Set                      $identifier

# FS extension - this action accepts both PlayAlertSound
# and CustomAlertSound tokens. Useful when you want to use
# constants because the constant can be freely changed
# what sound variant it refers to.
SetAlertSound            String | ((Integer | ShaperVoiceLine) [Integer])
```

Examples:

```
SetTextColor 240 200 150
SetBackgroundColor 0 0 0 255
CustomAlertSound "pop.wav"
PlayAlertSound 1 $volume
SetBeam Green                # permanent beam
SetBeam Green Temp           # temporary beam
SetMinimapIcon 0 Blue Square
```

### compound actions

FS also supports compound actions. You can assign multiple actions to 1 identifier and use the `Set` keyword to apply all.

Like with every other actions, later (and deeper nested) actions override previous ones on their depth.

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
$comp1 = {
	SetBorderColor $x
	SetTextColor $x
}
<div></div>
$comp2 = {
	SetBorderColor $y
	SetTextColor $y
	SetFontSize 42
}
<div></div>
SetFontSize 36
Set $comp1
<div></div>
Rarity rare {
	Set $comp2
<div></div>
	Quality 20 {
		SetBackgroundColor 50 50 50
		Set $comp1
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

## autogeneration

One of the core fancy features of FS.

Autogeneration places strings into `BaseType` or similar conditions (in case of prophecies it populates `Prophecy` condition). The respective condition must be empty when you use autogeneration and the related `Class` condition must either not exist or match autogenerated item class. Some autogenerations place additional requirements - for `uniques_*` the `Rarity` condition must either not exist or allow `Unique`.

- All queries only return high-confidence items. Low-confidence items are treated as if they did not exist in the price data.
- You can limit what items appear using `Price` condition

Available queries:

- `cards`
- `prophecies`
- `essences`
- `fossils`
- `resonators`
- `scarabs`
- `incubators`
- `oils`
- `catalysts`
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
- FS removes undroppable uniques (eg Harbinger and vendor-only uniques) from market information before outputting base types. This results in less ambiguities and removes items which you would not care for anyway when writing the filter. Corruption-only uniques are not removed because 1) poe.watch and poe.ninja differ in their reports or searches 2) maps with Corrupting Tempest and some sextant mods can drop corrupted items

Examples:

```
Class "Divination Card"
{
	Autogen cards
	{
		Price > 150
		{
			SetAlertSound "maybe_doctor.wav"
			Show
		}

		Price <  150
		Price >= 100
		{
			# ...
		}

		Price <  100
		Price >= 10
		{
			# ...
		}

		Price < 0.5
		{
			Hide
		}

	}

	# any remaining cards, not catched above
	Show
}
```

```
Rarity Unique
{
	SetTextColor $brown

	# surely good drops
	Autogen uniques_eq_unambiguous
	Price > 100
	{
		SetAlertSound 1
		Show
	}

	# potentially good drops
	Autogen uniques_eq_ambiguous
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
