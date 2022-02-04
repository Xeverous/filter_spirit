# How filters work

## preface

In order to write filters or using FS in any way, you first need to understand what an actual filter can do (and what it can not).

If you have never been editing any filter, here are the related articles from wiki (not maintained by me). They cover generals how filters work and extra details such as how to open the file and where to place it.

- https://pathofexile.gamepedia.com/Item_filter
- https://pathofexile.gamepedia.com/Item_filter_guide

GGG also made a webpage about filters - https://www.pathofexile.com/item-filter/about

If you have no good text editing program (and no idea what to use) then my (subjective) recommendation is [Notepad++](https://notepad-plus-plus.org). Some /r/pathofexile users even made syntax highlight specifications for it to make it colorize `.filter` files (otherwise I recommend to use "Shell" language coloring scheme):

- https://old.reddit.com/r/pathofexile/comments/ihz5qt/syntax_highlights_for_filter_files_notepad/
- https://old.reddit.com/r/pathofexile/comments/j4gaqv/notepad_highlight_for_filters_v23/

Filters are expected to be written using UTF-8 encoding. In Notepad++: Encoding => Convert to UTF-8.

Both wiki and GGG's webpage do not cover all (somewhat intricate) details about each rule. This article is supposed to explain them all, in depth, based on in-game experiments, reddit and information that has been collected from all patch notes since Item Filters were introduced to the game.

## data availability

Filters have no access to the following information:

- player level
- league
- item full name after identification (only base type name)
- whether the item has been allocated to you (while playing in a party)
- whether the item has multiple variants (some uniques have)
- whether the item has any drop restrictions (eg boss-specific, league-specific, legacy content)
- whether the item has alternate art
- whether the item has already been picked up and dropped again by a player
- whether the currently played character can equip the item

Additionally

- Some items can not be hidden (`Hide` will be ignored by the game). Read FAQ for more details.
- Many unique items share the same base type. So your next Inpulsa's drop is very likely to actually be Tinkerskin (both are `Sadist Garb`). This pretty much makes filtering some unique items impossible because multiple uniques share the same base (very common for belts, rings, amulets and jewels). There is no better way to workaround it other than having blocks for:
  - "potentially good drops" (base with at least 1 good unique)
  - "good drops" (base with only good uniques)
  - "bad drops" (base with only bad uniques)

## basics

Some rules (conditions or actions) accept values of different types. The table below summarizes them and lists keywords for literals.

`Suit` is referred as such to avoid name conflict (or any ambiguity within documentation) with `Color`. Since there are very few possible values, you can think of it as a suit of playing cards.

expression(s) | type | notes
--------------|------|------
`None` | NoneType | use to clear a condition or assign nothing
`Temp` | TempType | optional token for some actions
`False`, `True` | Boolean |
`123` | Integer | leading zeros accepted (`0123` is equal to `123`)
`RGB`, `3GGG`, `AA` | SocketSpec | `W` - white socket, `A` - abyss socket, `D` - resonator socket
`Normal`, `Magic`, `Rare`, `Unique` | Rarity |
`Circle`, `Diamond`, `Hexagon`, `Square`, `Star`, `Triangle`, `Cross`, `Moon`, `Raindrop`, `Kite`, `Pentagon`, `UpsideDownHouse` | Shape |
`Red`, `Green`, `Blue`, `White`, `Brown`, `Yellow`, `Cyan`, `Grey`, `Orange`, `Pink`, `Purple` | Suit |
`ShMirror`, `ShExalted`, `ShDivine`, `ShGeneral`, `ShRegal`, `ShChaos`, `ShFusing`, `ShAlchemy`, `ShVaal`, `ShBlessed` | ShaperVoiceLine
`Superior`, `Divergent`, `Anomalous`, `Phantasmal` | `QualityType` |
`Shaper`, `Elder`, `Crusader`, `Redeemer`, `Hunter`, `Warlord` | Influence |
`""`, `"abc"`, `"Leather Belt"` | String | UTF-8 encoding, line breaking characters (LF and CR) not allowed between quotes

Note: game allows unquoted string literals (e.g. `abc` instead of `"abc"`) if they do not contain whitespace characters. FS does support this, but only when reading real filters. If you write filter templates, you must always write quoted string literals.

## conditions

Note: this is not a formal grammar specification. GGG never did any and it is not really realistic to make one "from experience" and then test it. Plus, it would not be easily readable (neither very usable) so have this instead:

```
X       # X token is required
[X]     # X token is optional
X | Y   # X or Y token
X*      # 0 or more X tokens
X+      # 1 or more X tokens

# token CMP: < | > | <= | >= | = | ==
# token EQ: = | ==
# token SS: [Integer]R*G*B*W*A*D*
```

```
Rarity                [CMP] Rarity+
ItemLevel             [CMP] Integer+
DropLevel             [CMP] Integer+
Quality               [CMP] Integer+
LinkedSockets         [CMP] Integer+
Height                [CMP] Integer+
Width                 [CMP] Integer+
StackSize             [CMP] Integer+
GemLevel              [CMP] Integer+
MapTier               [CMP] Integer+
AreaLevel             [CMP] Integer+
CorruptedMods         [CMP] Integer+
EnchantmentPassiveNum [CMP] Integer+
BaseDefencePercentile [CMP] Integer+
BaseArmour            [CMP] Integer+
BaseEvasion           [CMP] Integer+
BaseEnergyShield      [CMP] Integer+
BaseWard              [CMP] Integer+

Class                  [EQ] String+
BaseType               [EQ] String+
EnchantmentPassiveNode [EQ] String+
HasInfluence           [EQ] None | Influence+

HasExplicitMod         [==] | CMPInteger String+
HasEnchantment         [==] | CMPInteger String+

GemQualityType QualityType

Sockets     [CMP] SS+
SocketGroup [CMP] SS+

AnyEnchantment   Boolean
Identified       Boolean
Corrupted        Boolean
Mirrored         Boolean
ElderItem        Boolean
ShaperItem       Boolean
FracturedItem    Boolean
SynthesisedItem  Boolean
ShapedMap        Boolean
ElderMap         Boolean
BlightedMap      Boolean
Replica          Boolean
AlternateQuality Boolean
Scourged         Boolean
UberBlightedMap  Boolean
```

Note: numeric and `Rarity` conditions can accept multiple values, but there is no practicaly point in doing so. You can cover any range with 2 lines using different comparisons, which is much better than manually specifying all possible values in 1 line. Additionally, current FS implementation does not support this and will error on more than 1 value (this is true both for real filters and filter templates).

Note: Game allows it, but currently FS does not support multiples of the same non-range condition within one block.

Wiki has some class-name related info (might be outdated):

- https://pathofexile.gamepedia.com/Item_class
- https://pathofexile.gamepedia.com/Public_stash_tab_API#frameType

### (nothing) vs `=` vs `==`

- Comparison operator is optional. Unless otherwise specified, `(nothing)` has the same meaning as `=`.
- There is no difference between `=` and `==` except for `HasInfluence`, `Sockets` and `SocketGroup`.
- `BaseType == "The Wolf"` will match only *The Wolf* card, `BaseType "The Wolf"` and `BaseType = "The Wolf"` will match all *The Wolf*, *The Wolf's Legacy* and *The Wolf's Shadow*. This is analogical for all string-based conditions. In other words, `==` forces an exact match, instead of a substring match.
- Game client reports error upon loading a filter which has `==` with names that are not complete.
- `HasInfluence` condition behaves differently with `==`:
  - If there is nothing or `=`, it will match an item **with at least one of specified influences**.
  - If there is `==`, it will only match items **with all specified influences**.
- `Sockets` and `SocketGroup` have very complex matching rules. They are explained by Rhys in [this reddit thread](https://www.reddit.com/r/pathofexile/comments/f2t4tz/inconsistencies_in_new_filter_syntaxes/) and at the bottom of this article (to not clutter this section).

### `HasInfluence`

`None` will be accepted only if it appears exactly once, not mixed with any influence names. The block will match only items which have no influence.

### `HasExplicitMod` and `HasEnchantment`

These can be used like other string array conditions, but they can also have an operator immediately followed by an integer to change behavior from "at least 1 match" to specific range of matches.

```
op - any operator of: < <= = >= >

HasExplicitMod ==  "of Haast" "of Tzteosh" "of Ephij" # item must have >= 1 mod, strict string matching
HasExplicitMod op  "of Haast" "of Tzteosh" "of Ephij" # invalid syntax
HasExplicitMod     "of Haast" "of Tzteosh" "of Ephij" # item must have >= 1 mod

HasExplicitMod ==2 "of Haast" "of Tzteosh" "of Ephij" # item must have == 2 mods, strict string matching
HasExplicitMod op2 "of Haast" "of Tzteosh" "of Ephij" # item must have op 2 mods
HasExplicitMod   2 "of Haast" "of Tzteosh" "of Ephij" # invalid syntax
```

## actions

The optional 4th value for colors is the opacity value. `0` means fully transparent, `255` fully opaque. The default opacity is `240`.

```
SetBorderColor           Integer Integer Integer [Integer]
SetTextColor             Integer Integer Integer [Integer]
SetBackgroundColor       Integer Integer Integer [Integer]

SetFontSize              Integer

# first token is built-in sound ID, second is volume
PlayAlertSound           (Integer | ShaperVoiceLine) [Integer]
PlayAlertSoundPositional (Integer | ShaperVoiceLine) [Integer]

# path can be absolute or just the file name, second token is volume
CustomAlertSound         String [Integer]
# same as above but if the file does not exist, no sound is played
CustomAlertSoundOptional String [Integer]

# Some items always play specific built-in sounds,
# even if you don't specify anything.
# This action disables such behavior.
DisableDropSound
# ...and this action enables it back.
EnableDropSound

# As above, but these only have an effect
# if any alert sound has been specified
EnableDropSoundIfAlertSound
DisableDropSoundIfAlertSound

SetMinimapIcon           Integer Suit Shape

PlayEffect               Suit [Temp]
```

Examples:

```
SetTextColor 240 200 150
SetBackgroundColor 0 0 0 255
CustomAlertSound "pop.wav"
PlayAlertSound 1 300
SetBeam Green                # permanent beam
SetBeam Green Temp           # temporary beam
SetMinimapIcon 0 Blue Square
```

Notes:

- `SetFontSize` will accept values in range 1-45 but depending on game's language settings they might be clamped to smaller range. English version clamps to 18-42.

## continuation

As explained in [this GGG's post](https://www.pathofexile.com/forum/view-thread/2771031), you can place `Continue` at the end of the block to cause items to pass through it (with applied style) instead of stopping on first match. If an item matches a `Hide` block that `Continue`s, then later matches a `Show` block, it will use the most recently matched `Show` or `Hide` flag, and thus show. If an item matches and `Continue`s and then never matches any further blocks, it will also show or hide based on the most recently matched block.

The same update also introduced ability to cancel previous actions through use of `-1`, `None` and `""` values.

Examples of all disabled actions:

```
MinimapIcon -1
PlayEffect None
CustomAlertSound "None"
CustomAlertSound ""
PlayAlertSound None
PlayAlertSoundPositional None
```

Note that:

- `CustomAlertSound ""` and `CustomAlertSound "None"` are functionally equivalent.
- `CustomAlertSound None` is not valid.

## `Sockets` and `SocketGroup`

Formerly `Sockets` was a typical numeric comparison condition that just checked the amount of sockets on an item but has been extended to support colors aswell.

It still can be used exactly as it was originally intended but since it gained new functionality it sacrificed a lot of consistency and clarity for backwards compatibility and new features.

First, you should understand how sockets are stored and interpreted by the game: item's sockets are treated as a 2-dimentional array, where every subarray represents one group of sockets that are linked. Examples:

- `R-G-B A R-W` is `{{R, G, B}, {A}, {R, W}}`
- `A W W G-G-G` is `{{A}, {W}, {W}, {G, G, G}}`
- (no sockets) is `{}`

Sockets on resonators (`D`) and abyss sockets (`A`) can not ever be linked to any other sockets. Thus prime resonators are `{{D}, {D}, {D}, {D}}`.

How it works:

- Both conditions will accept multiple specifications (eg `SocketGroup < 5GGG 4RRR 6RWW`). Item has only to match one of them.
- There are no invariants between the number and color letters. Both are optional, but at least 1 thing must be present.
- `Sockets` cares about the total number of sockets/colors an item has. `SocketGroup` cares about the number of sockets/colors within each link-group separately.
  - This means that a `R-R R-R` item will be accepted by `Sockets RRR` but not by `SocketGroup RRR`.
  - `SocketGroup >= 4GG` does not catch an item `R-R-R-R G-G`. Colors must be within the link.
- soft-equal (no comparison symbol or only 1 `=` character):
  - for socket or link counts, it compares numeric values as expected
  - **for colors, it compares as if the symbol was <=**
- `<` and `>` use OR operation for subrequirements, other comparisons use AND (thus `< 5G` is equivalent to `< 5 G` but `<= 5G` is not equivalent to `<= 5 G`). **This is the reason for major inconsistency between `<` `>` and other symbols in matching.**

Examples:

```
Sockets     <  5GGG # less than 5        sockets OR  less than 3x G socket
Sockets     <= 5GGG # at most   5        sockets AND at most   3x G socket
Sockets        5GGG # exactly   5        sockets AND at least  3x G socket
Sockets     == 5GGG # exactly   5        sockets AND exactly   3x G socket
Sockets     >= 5GGG # at least  5        sockets AND at least  3x G socket
Sockets     >  5GGG # more than 5        sockets OR  more than 3x G socket
SocketGroup <  5GGG # less than 5 linked sockets OR  less than 3x G linked within any  group
SocketGroup <= 5GGG # at most   5 linked sockets AND at most   3x G linked within such group
SocketGroup    5GGG # exactly   5 linked sockets AND at least  3x G linked within such group
SocketGroup == 5GGG # exactly   5 linked sockets AND exactly   3x G linked within such group
SocketGroup >= 5GGG # at least  5 linked sockets AND at least  3x G linked within such group
SocketGroup >  5GGG # more than 5 linked sockets OR  more than 3x G linked within any  group
```

```
Sockets __ 5GGG
    5s 5L 3G   5s 5L 4G   6s 5L 2G + 1G  6s 5L 3G + 1G    6L 3G     6L 4G
<      -          -             -              -            -         -
<=     +          -             -              -            -         -
       +          +             -              -            -         -
==     +          -             -              -            -         -
>=     +          +             +              +            +         +
>      -          +             +              +            +         +

SocketGroup __ 5GGG
    5s 5L 3G   5s 5L 4G   6s 5L 2G + 1G  6s 5L 3G + 1G    6L 3G     6L 4G
<      -          -             +              +            -         -
<=     +          -             +              +            -         -
       +          +             -              +            -         -
==     +          -             -              +            -         -
>=     +          +             -              +            +         +
>      -          +             -              -            +         +
```

```
SocketGroup _____
         B-B R R   R-R B B
>  1R       +         +
>  2R       -         +
>= 1R       +         +
>= 2R       -         +
```

```
Sockets     < 4RG - an item with less than 4 sockets OR less than 1x         R socket OR 1x less than 1x         G socket
SocketGroup < 4RG - an item with less than a 4-link  OR less than 1x gropued R socket OR 1x less than 1x grouped G socket

B-B-B   G-G-G-G   R-R-R-R   R-G-B-B   R-G B-B   B-B-B-B G R
  +        +         +         -         -           -
  +        +         +         -         +           +


Sockets     > 4RG - an item with more than 4 sockets OR more than 1x         R socket OR more than 1x         G socket
SocketGroup > 4RG - an item with more than a 4-link  OR more than 1x grouped R socket OR more than 1x grouped G socket

B-B-B-B-B   R-R   R R   G-G   G G   R-G   R-G-B-B   R-G-B-B B
    +        +     +     +     +     -       -          +
    +        +     -     +     -     -       -          -
```

Notable things:

- For some items and conditions, the item can match `<` but not `<=` version of the condition.
- For some items and conditions, the item can match `>` but not `>=` version of the condition.

Examples:

```
R-R-R-G
       matches Sockets     >  5RR
does not match Sockets     >= 5RR
       matches SocketGroup >  5RR
does not match SocketGroup >= 5RR

R-R-R-R-G-B
       matches Sockets     <  5GGG
does not match Sockets     <= 5GGG
       matches SocketGroup <  5GGG
does not match SocketGroup <= 5GGG
```

- Items with no sockets get special treatment - formerly they would be treated as items with 0 link-groups but instead they are treated as if they had 1 link-group that contains 0 sockets. This treatment affects behavior of `SocketGroup` to allow socketless items to match some comparisons (because they have a group that the logic can work on). Examples:

```
(socketless item)

does not match Sockets     <  0
       matches Sockets     <  1
       matches Sockets     <  1RGB
       matches Sockets     <   RGB
       matches Sockets     <= 1
       matches Sockets     <= 1RGB
       matches Sockets     <=  RGB
       matches SocketGroup <  1
       matches SocketGroup <  1RGB
       matches SocketGroup <   RGB
       matches SocketGroup <= 1
       matches SocketGroup <= 1RGB
       matches SocketGroup <=  RGB
does not match SocketGroup <  0
       matches SocketGroup <= 0
       matches SocketGroup    0
       matches SocketGroup == 0
does not match Sockets     >  0
does not match Sockets     >  0R
does not match SocketGroup >  0
does not match SocketGroup >  0R
       matches Sockets     >= 0
does not match Sockets     >= 0R
       matches SocketGroup >= 0
does not match SocketGroup >= 0R
```
