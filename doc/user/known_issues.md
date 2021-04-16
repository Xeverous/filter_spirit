# known issues

- Filters in the game ignore diacritics. This means that `Maelstrom Staff` will catch `Maelström Staff`. FS treats strings exactly as they are and therefore may showcase different matches in debug view because it does not implement any unicode-specific text logic (which is quite complicated).
  - Generation of filters is unaffected because strings are copied as they are written in original file.
  - If you always write item names perfectly, this issue will never happen for you.
- `Rarity` and numeric conditions that compare values can accept many values, for example: `Quality > 2 3 4 5`. Current FS implementation does not support this and will error if there is more than 1 value.
-  Game allows it, but currently FS does not support multiples of the same non-range condition within one block.