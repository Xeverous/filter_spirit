## basics

Filter Spirit is written completely in C++17 - planning to move to C++20 once modules and the tooling for new include-less build system is ready.

Because the code goes pretty hard on language features, you might get problems when trying to build this project using older Microsoft Visual compiler versions. I recommend to update it to 2019 version or use Clang or GCC instead.

During development, with the help of people behind boost spirit, a [GCC bug](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=89381) has been discovered. You need GCC 9.0+ or to write [workarounds as in this issue](https://github.com/boostorg/spirit/issues/465#issuecomment-464537240) into `lib/parser/ast.hpp`.

## architecture & design

FS code uses a ton of boost so get ready to see really lots of it. I recommend to read respective boost library tutorials before trying to change any related code.

Most of the program logic is realized using [monadic](https://en.wikipedia.org/wiki/Monad_(functional_programming)) interfaces (as opposed to traditional object oriented approach using abstract classes and dynamic dispatch). In C++ this means a lot of optionals, variants, tuples and (generic) lambdas. This has important tradeoffs:

- In traditional OOP, you write types (classes) which implement behavior (virtual functions). It is easy to add a new type, but hard to add a new behavior (function) because in such case the code of all classes needs to be changed to implement new function.
- In monadic approach, you write behaviors (functions) which are implemented against types (monads: variants, tuples, optionals, etc). It is easy to add a new behavior, but hard to add a new type because in such case the code of all functions needs to be changed to support the changed monad. Some code (lambdas, templates) may adapt automatically if it is generic enough and new types satisfy some common concept.

Additionally:

- In monadic approach, variant types do not need to have anything in common.
- Since monadic interfaces use a different way of dispatching, they do not require dynamic memory allocation and use through pointer indirection.

Because the core program logic is mostly a parser+compiler, a lot of code is dedicated to work with the abstract syntax tree. Such code is very often case-specific so there is a rather low reuse of code - most functions are used only once. For this reason, monadic approach is a good choice because it is much more common to need to add a new behavior (new feature/function) than to add/modify a type (grammar change).

Some of the code is very verbose (`std::variant<T1, T2> v = f(); if (std::holds_alternative<T1>(v)) ...`) due to lack of C++ first-class support for monadic interfaces. Planning to rewrite such code by adding a wrapper with overloaded operators which work similarly to ranges-v3 or a type with an interface as mentioned in wg21.link/p0798 - that would mostly be some hybrid of `boost::outcome` and `std/boost::variant/optional/tuple`.

## dependency overview

- `boost/spirit/x3` - the heart of this project. Through abuse of C++ templates and operator overloading, the library allows to define grammars for an LL-infinite parser at compile time. This saves a ton of work in implementing the parser and debugging the whole string-find-if-else sphagetti code that is usually required to read non-regular grammars. There is [a short Wikipedia article](https://en.wikipedia.org/wiki/Spirit_Parser_Framework) on how it works. The third library edition leverages newer C++ standards to offer simpler code and compiles significantly faster. FS grammar definitions reside in `lib/parser/detail/grammar_def.hpp`.
- `boost/fusion` and the preprocessor library: mostly dependencies of Spirit.
- `boost/optional` and `boost/variant` - they are used because Spirit does not yet fully support standard library equivalents. Outside the parser and compiler, standard library is preferred.
- `boost/container` - extra data structures which mix some STL containers with different tradeoffs. Only for optimization.
- `boost/program_options` - command line parser.
- `boost/date_time` - this is rather obvious.
- `boost/unit_test` - obvious. Boost Test library might be a huge overkill when Google Test is much simpler, but since FS already uses a ton of boost there is not much point in dragging more dependencies.
- `boost/format` - string formatting.
- `boost/type_index` - pretty-printing type names with and without RTTI
- `boost/assert` - much better than the `assert` macro inherited from C.
- `nlohmann/json` - JSON parser library. Might switch to `boost/json` once it gets official approval and is integrated into official boost repository.
- OpenSSL - implementation of network cryptography. One of possible SSL implementations for libcurl.
- Better Enum library - copied into utility directory. Offers a macro to define and automate some boilerplate to avoid shortcomings with C++ enums.

Originally `boost/beast` (and its dependency - `boost/asio`) were used for networking but they have been replaced with libcurl. Beast offers a very detailed HTTP and Websocket functionality but libcurl is much simpler to use, requires much less manual work, supports proxies out-of-the-box and much more. libcurl is only a client library but FS does not need any server-side functionality. In short, beast with its very finely-grained "do your own protocol" interface is an overkill for simple download tasks.

## program structure

Directories:

- `lib` - core library code
- `cli` - command line interface executable
- `gui` - graphic user interface executable (soon)
- `test` - test runner executable

`lib` subdirectories (each subdirectory has a separate namespace):

- `network` - apart from what is obvious, also contains JSON parsing and item price data manipulation.
- `lang` - all stuff that is related to FS language (keywords, types, item definitions etc).
- `parser` - parser implementation using Boost Spirit X3. Refer to X3 documentation to learn it before trying to understand the magic that is happening there.
- `compiler` - code that takes AST generated by the parser, downloaded item price data and outputs filters or errors. Performs semantic analysis.
- `log` - logger and string utility functions. So far `boost/log` looks like a huge overkill.
- `utility` - can be used anywhere. Contains STL helpers, string helpers and Better Enum library.

Except tests, no source file may include code from other `lib` part `detail` namespace.

Internal dependency graph:

```
lang -> parser -> compiler
                           \
                            > generator
                           /
          lang -> network
```

## the code

- There are no hard formatting or naming rules used, only what always "sounds reasonable". FS strictly follows C++ Core Guidelines recommended `snake_case_naming_style` which along with C++ standard library and boost make a totally naming-consistent codebase.
- Code which is never intended to be shared in other places uses anonymous namespaces. Apart from what it speaks already, it also reduces number of exported symbols which results in faster linking of shared library objects.
