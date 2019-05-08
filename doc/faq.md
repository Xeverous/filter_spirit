# FAQ

## usage

**I have changed font size but it looks the same.**

Different font sizes that have very close values may look exactly the same. This is not a bug in item filters but the result of whole pixel roundings in graphics and the fact that Path of Exile client supports custom resolutions.

**Some items are displayed even though they are catched by a `Hide` block. Is this a bug?**

`Hide` on some items is ignored (usually items of new category introduced by newest game content) (in the past: shaper and elder bases). It is intentional from GGG side - the motivation is to force new game content to be visible in case an outdated filter accidentally hides something important. The drawback is that advanced users can not hide some truly unwanted items.

**How about auto-refresh feature?**

Could be added very easily. The problem is that the game client is not aware that the filter file has been updated so you have to manually open UI options => item filter => reload.

**How do I filter cards from X price up, with no limit?**

`$divination(X, 9999999)` should be enough. Just don't go past `2147483648`.

There is an idea to support `(X, _)` but since the current workaround is very easy it's a low priority feature.

**When loading the filter, I'm getting an error with some gibberish string. What's wrong?**

You likely saved a file with non-Unicode encoding. Convert it to UTF-8:

- Notepad++: Encoding => Covert to UTF-8
- Visual Studio Code: status bar in the bottom right corner: click encoding abbrevation.

**I have regenerated the filter but it contains different number of blocks. Is this a bug?**

No. Invalid blocks are not generated. Consider this:

```
Class "Divination Card"
BaseType $divination(100, 150) {
	# ...
}
```

What if at the moment of generation there are no cards worth \[100, 150) chaos? The query would return an empty array which would lead to a generation of such block:

```
Show
	Class "Divination Card"
	BaseType
	# ...
```

This block is invalid but since there is nothing it should catch such blocks are simply not generated.

**Is this program allowed (does not break GGG's ToS)?**

Yes. this program does not interact with the game client in any way. It only produces a text file that is later read by it.

## design of FS

**Why this name?**

2 main reasons:

- I wanted the thing to be searchable - being easy to find and distinguish from other tools. "Yet another filter tool" or "Filter generator/compiler" are to broad terms, so I picked something unique that intentionally has no direct meaning to avoid misinterpretation.
- The heart of the program is the LL parser made using Boost Spirit library. It (ab)uses operator overloading and expressions templates to express language syntax using plain C++ code - each operator implements 1 particular action (skip, alternate, loop until, look ahead etc) and thanks to templates you can freely combine them to form very nested types that merge all these loops and if-elses into a single parse function. [Wikipedia has a short article on it.](https://en.wikipedia.org/wiki/Spirit_Parser_Framework)

**Why this library in particular?**

There are more parser libraries available, namely: YACC (used by GDB), Bison and Lemon (used by SQLite) but they all share the same problems:

- They are not really libraries. They have some library parts but mostly they are separate programs that given grammar spec, they generate code. This complicates the build process, complicates your own code and breaks working with any IDE.
- They all generate C. So forget about automatic memory management (in the form of garbage collection or RAII), you will have to code it yourself. And forget about type safety - everything is `void*`, any wrong cast will likely result in a segfault.

**Why `==` and not `=` like in the original filter?**

`=` from filter language has been replaced by `==` for consistency (this change should be obvious for anyone who ever programmed in any language). `=` is used for defining constants, `==` for comparisons. This makes filter easier to parse. You can still skip `==`, just like with original filter language: `Quality 20` will have exactly the same meaning as `Quality == 20`.

**Why arrays have `[elem, ...]` syntax? In actual filters you just place more strings in the same line.**

Again, parsing problems, this time even more complex.

`const <identifier> = <expr>` is much easier to implement because the parser does not have to check if there are multiple expressions. If there is an array it is delimited by `[]` and is treated as one expression with multiple subexpressions separated by `,`.

```
101 102 103 # a color or an array of integers?
```

Obviously there would be some way to deal with such problems but I prefer to have a context-free grammar rather than something that requires to write a ton of if-else to handle all corner cases.

Using line breaks as a delimiter and/or indent does not appeal to me because:

- Mistaken indentation has a very high chance to be parsed successfully, whereas an unmatched bracket is always a clear error.
- If whitespace affects parsing, code can not be freely formatted.
  - If you want to allow some relaxed formatting (eg ignoring line breaks when a bracked is not closed) that will be a lot of pain to handle all cases and a place for lots of potential bugs.
- Grammars utilizing [off-side rule](https://en.wikipedia.org/wiki/Off-side_rule) require additional state, make many subgrammars context-sensitive which makes implementation more complex.
