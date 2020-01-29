# Filter Spirit changelog

## upcoming version

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
