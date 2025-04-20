#include <string.h>
#include <ctype.h>
#include "utils.h"

void trim(char *str) {
    if (!str) return;

    // Trim leading whitespace
    char *start = str;
    while (*start && isspace((unsigned char)*start)) start++;

    if (*start == '\0') {
        *str = '\0';
        return;
    }

    // Trim trailing whitespace
    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;

    *(end + 1) = '\0';

    if (start > str) {
        memmove(str, start, end - start + 2);
    }
}