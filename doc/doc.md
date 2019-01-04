# doc

The following file describes all of the syntax used in Filter Spirit. It should be a primary source of knowledge for writing filter sources and a background for developing (I like the approach documentation first, implementation second).

## terms

Filter Sprit uses the following terms accross the documentation. These exist mostly to avoid ambiguities and offer short names for complex mechanisms.

- **condition** - a complete requirement for the item to be matched. `ItemLevel > 3`, `Rarity Unique` and such.
- **action** - a thing that is performed upon matched items. `SetFontSize 18`, `MinimapIcon 0 Red Circle` and such.
- Conditions and actions are mutually exclusive. There is no thing that is both at the same time.

## core grammar

Filter Spirit uses [*context-sensitive grammar*](https://en.wikipedia.org/wiki/Chomsky_hierarchy#The_hierarchy) that uses whitespace to delimit tokens and ignores any redundant non-newline whitespace. The only exception is the newline character, which delimits conditions, actions and other expressions. I could make it also ignore newlines, but then there would be something needed as a delimeter - having to write `;` at the end of each line is an unnecessary noise I would like to avoid. While the C-like curcly brace syntax with semicolons is quite popular for many languages, I think Path of Exile filters are simple enough to be able to avoid such thing and just have the rule of 1 statement per line. I aim to be as close to the original item filter syntax as possible.

The language type system is strong, which means that the tool will error upon incompatible types:

```
Number x = 3
Level  l = x   # builds 'Level' object from 'Number'

# [...]
ItemLevel > x  # ok, builds 'Level' object from 'Number'
ItemLevel > l  # ok
DropLevel > l  # ok
Sockets   > l  # error: expected sequence of digits or 'Number', got 'Level'
Sockets   > x  # ok
```

Strong typing prevents a lot of accidental mistakes (eg using drop alert volume as the number of required links). Use the most strict types for any condition/action.

### syntax elements

- **comment** - a text that is always ignored. Lines starting with `#` are comments.
- **operator** - a string literal that is one of `=`, `==`, `<`, `>`, `<=`, `>=`. `=` is not a **comparison operator**.
- **identifier** - an alpha character (one of `a-zA-Z_`) optionally followed by a sequence of alphanumeric characters (`a-zA-Z_0-9`) (identifiers can not start with a digit). Identifiers that start with `_` are reserved (you should not use them).

## types

Actions and conditions may consist of multiple elements. Filter Spirit denotes multiple types for better error checking.

- **Boolean** - a string literal `True` or `False`.
- **Number** - a non-empty sequence of digit characters (`0-9`).
  - **Level** - a more restricted form of Number that can be used only for `ItemLevel` and `DropLevel` conditions.
  - **SoundId** - a more restricted form of Number that can be used only for `PlayAlertSound` and `PlayAlertSoundPositional`
  - **Volume** - a more restricted form of Number that can be used only for `PlayAlertSound` and `PlayAlertSoundPositional`
- **Color** - RGB or RGBA color, written as 3 or 4 numbers. The default value of alpha channel (opacity) is ??? but can be globally overriden.
- **Suit** - a string literal `Red`, `Green`, `Blue`, `Brown`, `White` or `Yellow`. Used for beams and minimap icons. This type was not named Color to avoid ambiguities with RGB colors that are used for other actions - think of it as the 'suit' meaning from playing cards.
- **Shape** - a string literal `Circle`, `Diamond`, `Hexagon`, `Square`, `Star`, or `Triangle`.
- **Rarity** - a string literal `Normal`, `Magic`, `Rare`, or `Unique`.
- **Group** - a non-empty sequence of `R`, `G`, `B` or `W` characters.
- **String** - a sequence of characters between 2 `"` characters. May contain spaces. `""` is an empty String.

## notes

Some stuff which is worth to mention.

- Filter Spirit syntax consists only of basic ASCII (first 128 codes) but files can be UTF-8 and contain unicode characters in the comments.
- `=` from filter language has been replaced by `==` for consistency (this change should be obvious for anyone who ever programmed in any language). `=` is used for defining constants, `==` for comparisons. You can also skip `==`, just like with original filter language: `Quality 20` will have exactly the same meaning as `Quality == 20`.
- Different font sizes that have very close values may look exactly the same. This is not a bug in item filters but the result of roundings in graphics and the fact that Path of Exile client supports any custom resolution.
