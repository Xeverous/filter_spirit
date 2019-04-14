# Filter Spirit

Advanced item filter generator for Path of Exile that uses it's own DSL and online item price APIs. Basically a separate pseudo-programming language that lets you write item filters that pull item prices from online API and dynamically generate filter rules based on item prices and filter's code.

The project is under construction, message /u/Xeverous on reddit or Xeverous#2151 on Discord if you are interested in it's early development or have any questions/suggestions/whatever. You can also open an issue.

## short overview

Core features:

- static typing safety, error checking
- constants for colors/base-types etc and other data
statements can be nested (inherit conditions and override actions), no code duplication
- filter's source code is a template, you write BaseType `$divination(10, 100)` and it pulls data from API to list div cards worth 10-100c at generation time, refresh whenever you want
- support for filter config and variants (eg color-blind mode, strict mode, uber-strict, Animate Weapon support etc)

```
BaseType ["Gavel", "Stone Hammer", "Rock Breaker"] {
    SetTextColor color_white
    SetBackgroundColor color_hammer

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

Class "Divination Card" {
    SetTextColor color_divination

    # cards that you always want to pickup
    BaseType ["The Void", "The Cartographer", "Chaotic Disposition"] {
        SetAlertSound alert_divination_stack_1
        SetBeam beam_divination
        Show
    }

    # 100c+
    BaseType $divination(100,999999) {
        SetTextColor color_black
        SetBorderColor color_divination
        SetBackgroundColor color_divination
        SetFontSize font_max
        SetAlertSound alert_divination_best
        SetBeam beam_divination
        SetMinimapIcon MinimapIcon(0, blue, square)
        Show
    }

    # 10 - 100c
    BaseType $divination(10, 100) {
        SetBorderColor color_divination
        SetFontSize font_mid_divinaton
        SetAlertSound alert_divination_mid
        SetBeam beam_divination
        SetMinimapIcon MinimapIcon(1, blue, square)
        Show
    }

    [...]
}
```

## documentation

Under construction... the filter language and spec is not yet complete.

You can view example filter's source in the repository although the syntax is a subject to change.

## building

CMake script soon... right now there is only Eclipse project file

Dependencies:

- C++17 compiler (`<filesystem>` not required)
- Boost 1.68+
- Boost.Spirit develop branch (using and bug-report-fixing bleeding edge Spirit X3 parser)
- nlohmann/json
- OpenSSL (preferably 1.1)
