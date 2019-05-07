# Filter Spirit

Advanced item filter generator for Path of Exile that uses it's own [DSL](https://en.wikipedia.org/wiki/Domain-specific_language) and online item price APIs. Basically a separate pseudo-programming language that lets you write item filters using item price data from poe.ninja or poe.watch available at generation time. Create multiple filter variants and refresh whenever you want to always be up to date with market prices.

The project is under construction, message [/u/Xeverous](https://old.reddit.com/user/Xeverous/) on reddit or Xeverous#2151 on Discord if you are interested in it's early development or have any questions/suggestions/whatever. You can also open an issue.

## documentation and tutorial

Browse `doc` directory. Files are in Markdown so you can read them online on GitHub.

Note that the tool is in its early development so things may chage. Suggestions welcome.

## overview

Core features:

- static typing safety
- generation-time error checking
- language elements dedicated to elimination of code duplication:
  - ability to define constants: `const red = RGB(255, 0, 0)`
  - statements work top-down like in regular filters but they can be nested to inherit conditions and override actions
- dynamic rules: `BaseType $divination(10, 100)` pulls data from API to list div cards worth 10-100c at generation time, refresh whenever you want - your filter is always up-to-date with market prices
- support for filter variants (eg color-blind, strict, uber-strict, Animate Weapon support etc) - generate multiple filters with different flavours from a single source file

There are 2 intended usage scenarios:

- Someone writes a filter using Filer Spirit templates and publishes generated variants that are ready to be immediately loaded by the game client.
- Someone writes a filter template and shares it to other people who have downloaded Filter Spirit - they can:
  - (re)generate actual filter any time, making it always up to date with market prices
  - edit the config to tweak generated variants
  - edit the source to change styles (eg colors, sounds)
  - edit the source to change filter structure

## example code

There is a full example filter template source in the repository although the syntax is a subject to change.

```
const color_white  = RGB(255, 255, 255, 255)
const color_hammer = RGB(162,  85,   0) # (default opacity)

BaseType ["Gavel", "Stone Hammer", "Rock Breaker"] {
	SetTextColor color_white
	SetBackgroundColor color_hammer
	# above BaseType conditon will be inherited by all nested blocks
	# above actions will be inherited and can be overriden by nested blocks

	Rarity normal {
		Show # show normal hammers
	}

	Rarity magic
	Quality >= 12 {
		Show # show magic hammers with 12+ quality
	}

	Rarity rare
	Quality >= 16 {
		Show # show rare hammers with 16+ quality
	}

	# hide any other hammers
	# actually discouraged as you might lose RGB recipe material for later blocks
	Hide
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
	BaseType $divination(100, 999999) {
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

	# if you really hate these cards
	BaseType ["Rain of Chaos", "Carrion Crow"] {
		Hide
	}
}
```

## build dependencies

- C++17 compiler (`<filesystem>` not required)
- Boost 1.70 OR older with Spirit headers updated to 1.70; this project uses:
  - spirit (using bleeding-edge X3 parser)
  - fusion
  - optional
  - variant
  - **program_options**
  - date_time
  - beast
  - asio
  - system
  - **unit_test_framework** (only if you build tests)
- nlohmann/json
- **OpenSSL** (preferably 1.1)

bolded dependencies require linking, all dependencies are exposed as targets in CMake script

## building

standard CMake based build

```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release [your_options...]
```

If you are using `make` don't forget to add `-j` (parallel jobs) to add *100% increased build speed per additional core*.

## licensing

LICENSE file in the main directory of the repository applies to any file, unless otherwise specified.

Contact me if you are interested in work on or using this project but need or require a more permissive license.
