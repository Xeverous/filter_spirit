# coding

Everything related to the project's source code

## formatting

There are no hard formatting requirements - just write code that looks good to read. Undecided? Read more code or ask in the PR.

## includes

### include order:

- local (using "" instead of <>) - used only in CLI executable and tests (might be changed to <> to be more consistent)
- `<fs/gui/*>`
- `<fs/*>`
- `<Magnum/*>`
- `<Corrade/*>`
- `<imgui*>`
- `<SDL_*>` (generally you should not need to)
- `<nlohmann/*>`
- `<boost/*>`
- `<*>` (C++ standard library)

Why this order? Because it goes from most-related-local dependency to most-widely-used-global-dependency. Such order is very good at "detecting" (through build errors and warnings) broken header guards and missing includes.

### header guards

This project uses `#pragma once`. I know it is not technically standard but it is "standard in practice". Even niche compilers support it and none of major compilers issue a warning about this extension even in pedantic mode. As a result, the code is shorter and while I had never problems with this pragma, I have had multiple cases of build failures due to incorrectly copy/pasted/edited traditional header guards.

### modules

Right now too soon to write/use anything. Will edit the project to use them once C++20 support is well enough.

### code

General:

- Follow C++ Core Guidelines. The project uses recommended `snake_case_style` only. Some external libraries (mostly ones used for GUI) use different styles but even they sometimes use the recommended style because the holy standard enforces this in certain concepts and language features.
- Do not use `BOOST_ASSERT`/`BOOST_ASSERT_MSG` directly. Use `FS_ASSERT`/`FS_ASSERT_MSG` instead. It wraps it and adds a layer of customization, implemented in CMake.
- Report any new cases of `FS_ASSERT`/`FS_ASSERT_MSG` in `noexcept` functions. These are problematic because with specific build configuration asserts will throw.
- `src/fs/gui/windows` is not Windows-specific code. Files there are virtual windows in the application draw area.
- Some types may seem quite simple that they could be rule-of-zero structs but instead they have private constructor and public static methods to create them. This is intended and exists to impose invariants.
- No internal code should ever do backwards compiler/parser flow. That is, if you parse a token and the token results in some `lang` object, it should never be "converted back" to any parser type "just because it would be easier to match a function". Always refactor to avoid duplicate logic. This is because duplicate logic:
  - May duplicate warnings, errors and even bugs.
  - It is not technically duplicate code, but duplicate logic is the major flaw of duplicate code.
- Do not write any platform-specific code without proper preprocessor. Desktop native version is currently aimed at 64-bit Windows 7+ and the planned web version is aimed at Emscripten which acts as a 32-bit Unix system.

UI:

- Don't use hardcoded sizes, use multiplies of `ImGui::GetFontSize()` or `ImGui::GetFrameHeight()` instead.
- Do not write `std::snprintf(buf, buf_sz, c_str)`. Do `std::snprintf(buf, buf_sz, "%s", c_str)` instead. In first case, if str contains `%` it is a security exploit.
- Use `std::reference_wrapper` as member variable when a member pointer would never be allowed to be null.

### other stuff

Read respectibe Boost library tutorials before modifying or even reading the code! This is especially true for Boost Spirit X3, which is very unique how it expects code to be structured.

[Boost Spirit X3 CppCon overview + tutorial](https://www.youtube.com/watch?v=xSBWklPLRvw) (60min)

Example commits you might want to check:

- `440e8b99f1cd4704999363a03e6bddebcd25e952`: support for `Expand`
- `4aaa4a7500b0d8bdf668ca6e1b717fccdbeac3a1`: support for `ShowHide` and `ShowDiscard`
- `d8ba8066b7f052fb3841803630bafa9389cde053`: support for new comparisons in `HasExplicitMod` and `HasEnchantment`
- `4786b89cf04e6fbf89ea5c205f92ce8ec85ce879`: support for `EnchantmentPassiveNum`
- `9f6ec9a244996939da867c2c312f3e1bc79fe1ca`: support for `Replica` and `AlternateQuality`
- `7ea011a65a719663bf853c5343c3ab22802e07c0`: support for `EnableDropSound`

AST code recommendations:

- if the AST type is a variant type (utilizing `X | Y | Z` grammars) - inherit from `x3::variant` (required by Spirit X3)
- if the AST type is a container type (utilizing `X+` or `*X` grammars) - inherit from the container
- if the AST type has 1- data member: write `operator=` that accepts data member type and a member `get_value()` (this is required for AST debug)
- if the AST type has 2+ data data members: adapt it using `BOOST_FUSION_ADAPT_STRUCT`
