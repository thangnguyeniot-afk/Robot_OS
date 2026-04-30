/* ro_assert.c — Host Assert Failure Handler */
#include <robotos/ro_assert.h>
#include <stdio.h>
#include <stdlib.h>

void ro_assert_fail(const char* file, int line, const char* msg)
{
    fprintf(stderr, "\n*** RO_ASSERT FAILED ***\n"
                    "  File: %s\n"
                    "  Line: %d\n"
                    "  Msg:  %s\n\n",
            file, line, msg);
    abort();
}
