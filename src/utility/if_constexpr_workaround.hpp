#pragma once

/*
 * workaround for Eclipse CDT not being able (yet) to
 * parse "if constexpr" - define a macro that evaluates to
 * nothing for Eclipse but to constexpr for the compiler
 * https://stackoverflow.com/questions/41386945
 *
 * usage:
 * if ECLIPSE_CONSTEXPR (condition)
 */
#ifdef __CDT_PARSER__
#  define ECLIPSE_CONSTEXPR
#else
#  define ECLIPSE_CONSTEXPR constexpr
#endif // __CDT_PARSER__
