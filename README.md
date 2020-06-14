# Filter Spirit

Advanced item filter generator for Path of Exile that uses it's own [DSL](https://en.wikipedia.org/wiki/Domain-specific_language) and online item price APIs. Basically a separate pseudo-programming language that lets you write item filters using item price data from poe.ninja or poe.watch available at generation time. Create multiple filter variants and refresh whenever you want to always be up to date with market prices.

The project is under construction, message [/u/Xeverous](https://old.reddit.com/user/Xeverous/) on reddit or Xeverous#2151 on Discord if you are interested in it's early development or have any questions/suggestions/whatever. You can also open an issue.

## overview

Core features:

- **Generation-time error checking**. FS has own parser and compiler that performs semantic analysis - it is not a blind text-copy-paste script.
- **Language features dedicated to elimination of filter code duplication**:
  - **named constants**: `const red = RGB(255, 0, 0)`
  - **named style groups**: `x = { SetFontSize 42 SetTextColor black }`
  - **nesting of filter blocks** to inherit conditions and override actions
- **querying item prices**: `BaseType $divination(10, 100)` pulls data from API to list div cards worth 10-100c at generation time. Refresh whenever you want - your filter is always up-to-date with market prices.

Planned features:

- Support for filter variants (eg color-blind, strict, uber-strict, Animate Weapon support etc) - generate multiple filters with different flavours from a single source file.
- Filter debugger & loot preview: something similar to https://bschug.github.io/poedit/poedit.html allowing to find matching blocks and preview randomly generated loot from specific game encounters.
- User-defined highlight for Notepad++ for FS syntax.
- Live edit mode - see generared code as you write.

## example code

There is a full example filter template source in the repository although the syntax is a subject to change.

```
color_white  = RGB(255, 255, 255, 255)
color_hammer = RGB(162,  85,   0) # (default opacity)

BaseType ["Gavel", "Stone Hammer", "Rock Breaker"] {
	SetTextColor color_white
	SetBackgroundColor color_hammer
	# above BaseType conditon will be inherited by all nested blocks
	# above actions will be inherited and can be overriden by nested blocks

	Rarity Normal {
		Show # show normal hammers
	}

	Rarity Magic
	Quality >= 12 {
		Show # show magic hammers with 12+ quality
	}

	Rarity Rare
	Quality >= 16 {
		Show # show rare hammers with 16+ quality
	}

	# hide any other hammers (actually discouraged as you might lose
	# RGB recipe material for later blocks - better don't write this Hide)
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
	BaseType $divination(100, _) {
		SetTextColor color_black
		SetBorderColor color_divination
		SetBackgroundColor color_divination
		SetFontSize font_max
		SetAlertSound alert_divination_best
		SetBeam beam_divination
		SetMinimapIcon MinimapIcon(0, Blue, Square)
		Show
	}

	# 10 - 100c
	BaseType $divination(10, 100) {
		SetBorderColor color_divination
		SetFontSize font_mid_divinaton
		SetAlertSound alert_divination_mid
		SetBeam beam_divination
		SetMinimapIcon MinimapIcon(1, Blue, Square)
		Show
	}

	# if you really hate these cards
	BaseType ["Rain of Chaos", "Carrion Crow"] {
		Hide
	}

	# any other cards
	Show
}
```

## documentation and tutorial

Browse `doc` directory. Files are in Markdown so you can read them online on GitHub.

Note that the tool is in its early development so things may chage. Suggestions welcome.

## program interface

Currently only a command line executable. Graphic interface in development - see pinned issue.

In Windows, you can quickly open command line in desired directory by typing "cmd" in the file explorer's path - see https://stackoverflow.com/a/10135218/4818802. Do this in FS directory (where `filter_spirit_cli.exe` is) and you can instantly use the program, eg `filter_spirit_cli --help`.

## runtime dependencies

FS does not need to install anything. You can download latest release and immediately use the program.

## build dependencies

- C++17 compiler
- Boost 1.70 OR older with Spirit headers updated to 1.70; this project uses:
  - spirit (newest X3 library)
  - fusion
  - preprocessor
  - optional
  - variant
  - **program_options**
  - date_time
  - beast
  - asio
  - system
  - **unit_test_framework** (only if you build tests)
- nlohmann/json
- **OpenSSL** (preferably 1.1+)

Bolded dependencies require linking, all dependencies are exposed as targets in CMake script.

## building

modern CMake build script

```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release [your_options...]
make -j
```

If you are using `make` don't forget to add `-j` (parallel jobs) to add *100% increased build speed per additional core*.

## licensing

LICENSE file in the main directory of the repository applies to any file, unless otherwise specified.

Contact me if you are interested in work on or using this project but got any concerns in regards to usage or licensing.
