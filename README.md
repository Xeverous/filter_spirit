# Filter Spirit

Advanced item filter generator for Path of Exile that uses it's own [DSL](https://en.wikipedia.org/wiki/Domain-specific_language) and online item price APIs. Basically an enhanced filter language that lets you write item filters with more convenience and using item price data from poe.ninja or poe.watch available at generation time. Create multiple filter variants and refresh whenever you want to always be up to date with market prices.

If you ever worked with web stuff - a perfect analogy is that Filter Spirit is the same for real filters as are LESS and Sass for CSS.

## news

All 3 new conditions from Hesit league implemented. Currently FS supports everything except `EnableDropSound` (`DisableDropSound` works) and disabling minimap icons, beam effect and alert sounds (`-1`, `None`, etc) (introduced with `Continue`). Sole `Continue` is supported.

___

poe.watch is dead - only poe.ninja generation will work. Original author passed the project to someone else and the website is being revived on [stats.mrk.xyz](https://stats.mrk.xyz).

___

FS is under construction, but a command-line version is already available. Message me ([/u/Xeverous](https://old.reddit.com/user/Xeverous/) on reddit or Xeverous#2151 on Discord) if you are interested in it or have any questions/suggestions/whatever. You can also open an issue.

Read existing issues for more detailed information on current state of the work.
___

I planned to release first graphical version at the end of Delirium league or a bit earlier but you know, it's my first that large project and I'm also playing the game and working full-time. I have [heavily contributed to elements](https://github.com/cycfi/elements/issues?q=author%3AXeverous) and made some own GUI library experiments which were planned to be used for FS GUI. Obviously I could use something well-known like Qt or wxWidgets but none of them fully satisfy my requirements so I decided to contribute to a new promising project instead. Call it a bit of Shaper's perfection syndrome. All of experiments failed and elements project itself is in too early stage to make it work for what I want. Current plan is to use [Magnum](https://magnum.graphics) with [Dear Imgui](https://github.com/ocornut/imgui) which could enable both a web-based port and a native desktop application, both with very rich UI. Expect updates on this topic in the middle (or later) of Heist league.

Thanks to some users, I'm thinking of these program interface opportunities:

- Visual Studio Code plugin that uses FS in the background.
- WASM-compiled build, hosted on a static GitHub Pages website.

Please contact me if you are familiar with web-related technologies and would like to help in making these.

## overview

Core features:

- **Generation-time error checking**. FS has own parser and compiler that performs semantic analysis - it is not a blind text-copy-paste script.
- **Language features dedicated to elimination of filter code duplication**:
  - **named constants**: `$red = 255 0 0`
  - **named style groups**: `$x = { SetFontSize 42 SetTextColor $black }`
  - **nesting of filter blocks** to inherit conditions and override actions
- **querying item prices**: `Autogen cards Price > 50` pulls data from API to list div cards worth 50+ chaos at generation time. Refresh whenever you want - your filter is always up-to-date with market prices.

Planned features:

- Support for filter variants (eg color-blind, strict, uber-strict, Animate Weapon support etc) - generate multiple filters with different flavours from a single source file.
- Filter debugger & loot preview: something similar to https://bschug.github.io/poedit/poedit.html allowing to find matching blocks and preview randomly generated loot from specific game encounters.
- User-defined highlight for Notepad++ for FS syntax.
- Live edit mode - see generared code as you write.

## example code

There is a full example filter template source in the repository. You can also browse `src/test/compiler/filter_generation_tests.cpp` for even more examples.

```
$color_white  = 255 255 255 255
$color_hammer = 162  85   0 # (will use default opacity)

BaseType "Gavel" "Stone Hammer" "Rock Breaker" {
	SetTextColor $color_white
	SetBackgroundColor $color_hammer
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
	SetTextColor $color_divination

	# cards that you always want to pickup
	BaseType "The Void" "The Cartographer" "Chaotic Disposition" {
		SetAlertSound $alert_divination_stack_1
		PlayEffect $effect_divination
		Show
	}

	Autogen cards {
		Price >= 100 {
			SetTextColor $color_black
			SetBorderColor $color_divination
			SetBackgroundColor $color_divination
			SetFontSize $font_max
			SetAlertSound $alert_divination_best
			PlayEffect $effect_divination
			MinimapIcon 0 Blue Square
			Show
		}

		Price < 100
		Price >= 10 {
			SetBorderColor $color_divination
			SetFontSize $font_mid_divinaton
			SetAlertSound $alert_divination_mid
			PlayEffect $effect_divination
			MinimapIcon 1 Blue Square
			Show
		}
	}

	# if you really hate these cards
	BaseType "Rain of Chaos" "Carrion Crow" {
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

FS does not need to install anything (it's fully portable). You can download latest release and immediately use the program.

## build dependencies

- C++17 compiler
- Boost 1.70+:
  - spirit (FS uses newest X3 parser)
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
- nlohmann/json 3.0+
- **OpenSSL** 1.1+
- **libcurl** 7.17+

Bolded dependencies need to be build. Rest are header-only libraries.

## building

Modern CMake build script. All dependencies are exposed as targets. See comments inside the build recipe for more information.

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
