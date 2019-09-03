#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"

int debug_level = 0;

FILE* debug_file = 0;

static const char* prefix = "-debug";


/** This routine will initialize the variables from the arguments passed to
 * `main()`. If the **first** argument is, or starts with `-debug` the
 * class is initialized and the first parameter is removed from the `args`.
 * The first parameter may have one of three forms:
 *
 * 1. `-debug` - turn on debugging at level 1
 * 2. `-debugValue` - set debugging level to `Value` (e.g. `-debug5`)
 * 3. `-debugValue@fileName` set debugging level to `Value` and send
 *     debugging output to the file `fileName`. If you use this option, the
 *     file must b closed using `debugClose()`.
 *
 * On return, `argc` and `argv[]` may be modified.
 * @param argc - the number of parameters passed to `main`
 * @param args - the array of arguments passed to `main`
 */

void
debug_init(int* argc, const char* argv[])
{
    debug_file = stderr;

    if (*argc > 1) {
        const char* arg1 = argv[1];
        size_t len = strlen(prefix);

        if (strncmp(arg1, prefix, len) == 0) {
            debug_level = 1;

            char* ats = strchr(arg1, '@');

            if (ats) {
                *ats = '\0';
                debug_to_file(ats + 1);
            }

            if (strlen(arg1) > len) debug_level = atoi(arg1 + len);

            (*argc)--; // decrement number of arguments

            for (int i = 1; i < *argc; i++) // remove first argument
                argv[i] = argv[i + 1];
        }
    }
}

/** Send debugging output to a file.
 *  @param fileName name of file to send output to
 */

void
debug_to_file(const char* fileName)
{
    debug_close();

    FILE* f = fopen(fileName, "w"); // "w+" ?

    if (f) debug_file = f;
}

/** Close the output file if it was set in `toFile()` */

void
debug_close(void)
{
    if (debug_file && (debug_file != stderr)) {
        fclose(debug_file);
        debug_file = stderr;
    }
}
