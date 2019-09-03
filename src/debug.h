#ifndef __DEBUG_H__
#define __DEBUG_H__

/** @file Debug.h
 *  @brief Define a **simple** debugging interface to use in C programs
 *
 *  @details One method of debugging your C program is to add `printf()`
 *  statements to your code. This file provides a way of including debug
 *  output, and being able to turn it on/off either at compile time or
 *  at runtime, without making **any** changes to your code.
 *  Two levels of debugging are provided. If the value `DEBUG` is
 *  defined, your debug calls are compiled into your code. Otherwise, they are
 *  removed by the optimizer. There is an additional run time check as to
 *  whether to actually print the debugging output. This is controlled by the
 *  value `debug_level`.
 *
 *  To use it with `gcc`, simply write a `printf()` call, but
 *  replace the `printf()` call by `debug()`.
 *
 *  To easily print the value of a **single** variable, use
 *  <pre>`
    vDebug("format", var); // "format" is the specifier (e.g. "%d" or "%s", etc)
 *  `</pre>
 *  <p>
 *  To use debug(), but control when it prints, use
 *  <pre>`
    lDebug(level, "format", var); // print when debug_level >= level
 *  `</pre>
 *
 *  Based on code and ideas found
 *  <a href="http://stackoverflow.com/questions/1644868/c-define-macro-for-debug-printing">
 *  here</a> and
 *  <a
 href="http://stackoverflow.com/questions/679979/how-to-make-a-variadic-macro-variable-number-of-arguments">
 *  here</a>.
 *
 *  @author Fritz Sieker
 **/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/** Initialize the variable `debug_level` depending on the value
 *  of `argv[1]`. Normally called from `main` with the program
 *  arguments. If `argv[1]` is or begins with `-debug`, the
 *  value of `debug_level` is set and `argc, argv` are
 *  modified appropriately. An entry of `-debug5` sets the level to 5.
 *  If the function is not called, the user is responsible for setting
 *  `debug_level` in other code.
 *  @param argc the number of command line arguments
 *  @param argv the array of command line arguments.
 */
void debug_init(int*, const char**);

/** Send the debug output to a file
 *  @param fileName name of file to write debug output to
 */
void debug_to_file(const char*);

/** Close the external file and reset `debug_file` to `stderr`
 */
void debug_close();

/** Control how much debug output is produced. Higher values produce more
 * output. See the use in `lDebug()`.
 */
extern int debug_level;

/** The file where debug output is written. Defaults to `stderr`.
 *  `debug_to_file()` allows output to any file.
 */
extern FILE* debug_file;

#ifdef DEBUG
#define DEBUG_ENABLED 1 // debug code available at runtime
#else
/** This macro controls whether all debugging code is optimized out of the
 *  executable, or is compiled and controlled at runtime by the
 *  `debug_level` variable. The value (0/1) depends on whether
 *  the macro `DEBUG` is defined during the compile.
 */
#define DEBUG_ENABLED 0 // all debug code optimized out
#endif

/** Print the file name, line number, function name and "HERE" */
#define HERE debug("HERE")

/** Expand a name into a string and a value
 *  @param name name of variable
 */
#define debugV(name) #name, (name)

/** Output the name and value of a single variable
 *  @param name name of the variable to print
 */
#define vDebug(fmt, name) debug("%s=(" fmt ")", debugV(name))

/** Simple alias for `lDebug()` */
#define debug(fmt, ...) lDebug(1, fmt, ##__VA_ARGS__)

/** Print this message if the variable `debug_level` is greater
 *  than or equal to the parameter.
 *  @param level the level at which this information should be printed
 *  @param fmt the formatting string (**MUST** be a literal
 */
#define lDebug(level, fmt, ...)                                                                    \
    do {                                                                                           \
        if (DEBUG_ENABLED && (debug_level >= level))                                               \
            fprintf((debug_file ? debug_file : stderr), "DEBUG %s[%d] %s() " fmt "\n", __FILE__,   \
                    __LINE__, __func__, ##__VA_ARGS__);                                            \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif
