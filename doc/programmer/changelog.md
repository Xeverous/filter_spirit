# Filter Spirit changelog

## version 1.1.0 (15.04.2021)

- Ovehaulued example filter.
- Implemented support for every action and condition.
- Added new feature: `Expand $subtree`.
- Added new feature: `ShowHide $boolean` and `ShowDiscard $boolean`.
- Lots of internal improvements.
- Significant documentation improvements.

## version 1.0.1 (15.09.2021)

- Fixed startup crash, caused by `std::random_device` on MinGW toolchain.

## version 1.0.0 (15.09.2021)

- **First release of graphical interface!** Features filter generation and filter debug.
- Added support for new filter stuff from 3.13 (Echoes of the Atlas + Ritual league) update.
- Documentation overhaul.

## version 0.7.0 (18.09.2020)

- Added support for `Continue` statement. Disabling minimap/beam/sound actions by `-1` or `None` is not yet implemented.
- Implemented all 3 new conditions from Heist: `Replica`, `AlternateQuality`, `GemQualityType`
- New FS extension: `None` in string-based conditions. When given directly or through a named constant will represent an empty array of names and therefore discard the block from the output filter.
- Some internal work preparing project for loot generation/preview/debug feature.

## version 0.6.0 (18.06.2020)

- Added support for `HasInfluence None`.
- Added support for `EnchantmentPassiveNode`.
- Unified comparison operators implementation. FS should now accept every possible operator combination that real filters accept.
- Added support for Shaper voice lines. They are also supported by `SetAlertSound` extension.
- `-a` argument is now `-p`.
- **New behavior:** FS no longer requires explicit save paths in the command line, instead it automatically downloads or uses cached data if it's not too old. Control max allowed age with `-a` parameter.

## version 0.5.0 (13.03.2020)

- Implemented new `Sockets` and `SocketGroup`, `AreaLevel`, `Corrupted` and other conditions as on https://www.pathofexile.com/forum/view-thread/2771031
- Added missing `Temp` implementation for `PlayEffect`
- Added new colors (suits) and shapes for minimap icons as on https://www.pathofexile.com/forum/view-thread/2784312
- **Breaking change** - redesigned syntax of constants to be more in-line with actual filters. See documentation for examples. Also renamed some conditions/actions to make then exactly the same as in real filters.
- New feature - condition extension: `SetAlertSound` will accept both values that are normally accepted by `PlayAlertSound` and `CustomAlertSound` - this gives the convenience to use a constant that can change which sound it refers to without having to rewrite all actions that (until now) referred specifically to either built-in sound or custom sound.
- **Breaking change** - redesigned query syntax to be more intuitive and closer to actual filters syntax. This is now called `Autogen`. Autogeneration now does more verification and supports more item categories. You can also influence its output and behavior by using new `Price` condition and regular filter conditions. See documentation for examples.
- Sadly, no support for `Continue`, `None` and `-1` for minimap icons.

## version 0.4.0 (29.01.2020)

- New small features:
  - Option `-e` allows to run FS with no item price information. Useful for those who write filters without price quesries (eg. filters for SSF). If the filter contains price queries, with this option enabled no items will be returned by them.
  - Option `-i` allows to display meta-information about given item price data save.
  - Option `-c` allows to compare 2 item price data saves in regards to single-property items (that is, items for which only name matters).
  - For option `-s`, FS will now create the deepest directory in the path if it does not exist.
- FS no longer removes corrupted uniques from price data. They can now appear in results of price queries (and cause some ambiguities) but that's intended because in some rare circumstances they can actually drop (eg. a sextant mod or Corrupting Tempest).
- Updated handling of multi-influenced items. They are still not queryable (see issues) but this change removes some unwanted logs. As of now, poe.ninja does not support dual-influenced bases.
- Fixed a bug caused by the used buffer for network responses which did not have size large enough to contain some of poe.ninja's responses, resulting in "could not read the request: body limit exceeded" error.

## version 0.3.0 (11.12.2019)

- Implemented new influence types for the Conquerors of the Atlas update. See documentation for examples.
- **Breaking change** - renamed all of lowercase keywords to match the ones in actual filters (`unique` => `Unique`, `true` => `True` etc).
- Added missing `Prophecy` condition.

## version 0.2.0 (10.12.2019)

- Added missing `ElderMap` and `BlightedMap` conditions.
- Fixed a bug where "Diamon" was generated instead of "Diamond".
- Added this changelog.

## version 0.1.0 (07.12.2019)

- Initial release. Reddit post with description: https://www.reddit.com/r/pathofexile/comments/e7kfqo/filter_spirit_a_filter_compilergenerator_tool
