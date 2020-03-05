# FAQ

## generated filters

**I have changed font size but it looks the same.**

Different font sizes that have very close values may look exactly the same. This is not a bug in item filters but the result of whole pixel roundings in graphics and the fact that Path of Exile client supports custom resolutions.

**Some items are displayed even though they are catched by a `Hide` block. Is this a bug?**

`Hide` on some items is ignored (usually items of new category introduced by newest game content) (in the past: shaper and elder bases). It is intentional from GGG side - the motivation is to force new game content to be visible in case an outdated filter accidentally hides something important. The drawback is that advanced users can not hide some truly unwanted items.

Here is the GGG's statement on this: https://www.pathofexile.com/forum/view-thread/2500872 (ctrl+F "filter").

**How does partying interact with filters?**

Filters know nothing about allocation of items in a party. All drops are treated just like you were playing solo - they will have their styles, fire sound effects and create map icons etc but items you can not pick up will be greyed out. There is no control over this behavior.

## usage

**How about auto-refresh feature?**

Could be added very easily. The problem is that the game client is not aware that the filter file has been updated so you have to manually open UI options => item filter => reload.

**When loading the filter, I'm getting an error with some gibberish string. What's wrong?**

You likely saved a file with non-Unicode encoding. Convert it to UTF-8:

- Notepad++: Encoding => Covert to UTF-8
- Visual Studio Code: status bar in the bottom right corner: click encoding abbrevation.

**I have regenerated the filter but it contains different number of blocks. Is this a bug?**

No. Invalid blocks are not generated. Consider this:

```
Class "Divination Card"
Autogen cars
Price >= 100
Price < 150 {
	# ...
}
```

What if at the moment of generation there are no cards worth \[100, 150) chaos? The autogeneration would output an empty array which would lead to a generation of such block:

```
Show
	Class "Divination Card"
	BaseType
	# ...
```

This block is invalid but since there is nothing it should catch such blocks are simply not generated.

**Is this program allowed (does not break GGG's ToS)?**

Yes. this program does not interact with the game client in any way. It only produces a text file that is later read by it.

**I have a networking problem. Can you...?**

FS uses [libcurl](https://curl.haxx.se) for networking. It is possible to expose much more of [its options](https://curl.haxx.se/libcurl/c/curl_easy_setopt.html). Just inform me what is the problem and what specific you need. Note that some of the options read environmental variables so some of them may already be used this way.

## design of FS

**Why this name?**

2 main reasons:

- I wanted the thing to be searchable - being easy to find and distinguish from other tools. "Yet another filter tool" or "Filter generator/compiler" are to broad terms, so I picked something unique that intentionally has no direct meaning to avoid misinterpretation.
- The heart of the program is the LL parser made using Boost Spirit library. It (ab)uses operator overloading and expressions templates to express language syntax using plain C++ code - each operator implements 1 particular action (skip, alternate, loop until, look ahead etc) and thanks to templates you can freely combine them to form very nested types that merge all these loops and if-elses into a single parse function. [Wikipedia has a short article on it.](https://en.wikipedia.org/wiki/Spirit_Parser_Framework)

**Why this library in particular?**

There are more parser libraries available, namely: YACC (used by GDB), Bison and Lemon (used by SQLite) but they all share the same problems:

- They are not really libraries. They have some library parts but mostly they are separate programs that given grammar spec, they generate code. This complicates the build process, complicates your own code and breaks working with any IDE.
- They all generate C. So forget about automatic memory management (in the form of garbage collection or RAII), you will have to code it yourself. And forget about type safety - everything is `void*`, any wrong cast will likely result in a segfault.

**Why are `{` and `}` required? Would not indent alone be enough?**

Grammars utilizing [off-side rule](https://en.wikipedia.org/wiki/Off-side_rule) require additional state, make many context-free subgrammars context-sensitive which in turn makes implementation more complex which in turn means "probably more bugs". Such grammars are prone to silent typo/indent errors/bugs while tokens such as `{}` must always match so a typo practically always ends in a parse error.

Obviously there would be some way to deal with such problems but I prefer to have a context-free grammar rather than something that requires to write a ton of if-else to handle all corner cases.

PoE filters are already somewhat white-space sensitive as some rules (eg `BaseType`) accept an arbitrary amount of tokens and there are no clearly visible delimeters - the only thing that currently delimits such rules is the line ending. I do not want to rely more on whitespace as such grammars result in much smaller "design space" and much less concrete error messages.
