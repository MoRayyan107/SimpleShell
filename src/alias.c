#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "alias.h"
#include "utils.h"

Alias aliases[MAX_ALIASES];
int alias_count = 0;

void add_alias(char *name, char *cmd) {
    trim(name);
    trim(cmd);

    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].alias, name) == 0) {
            strcpy(aliases[i].command, cmd);
            printf("Alias '%s' updated.\n", name);
            return;
        }
    }

    if (alias_count < MAX_ALIASES) {
        strcpy(aliases[alias_count].alias, name);
        strcpy(aliases[alias_count].command, cmd);
        alias_count++;
        printf("Alias '%s' added.\n", name);
    } else {
        printf("Error: Maximum number of aliases reached.\n");
    }
}

void remove_alias(char *name) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].alias, name) == 0) {
            for (int j = i; j < alias_count - 1; j++) {
                aliases[j] = aliases[j + 1];
            }
            alias_count--;
            printf("Alias '%s' removed.\n", name);
            return;
        }
    }
    printf("Error: No such alias '%s'.\n", name);
}

void print_aliases() {
    if (alias_count == 0) {
        printf("No aliases set.\n");
    } else {
        for (int i = 0; i < alias_count; i++) {
            if (strlen(aliases[i].alias) > 0 && strlen(aliases[i].command) > 0) {
                printf("%s = '%s'\n", aliases[i].alias, aliases[i].command);
            }
        }
    }
}

char *resolve_alias(char *input) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].alias, input) == 0) {
            return strdup(aliases[i].command);
        }
    }
    return strdup(input);
}

void save_aliases(Alias cmd_aliases[], int aliases_count) {
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/.aliases", getenv("HOME"));
    FILE *file = fopen(path, "w");

    if (!file) {
        perror("file cannot be opened");
        return;
    }

    for (int i = 0; i < aliases_count; i++) {
        if (strlen(cmd_aliases[i].alias) > 0 && strlen(cmd_aliases[i].command) > 0) {
            fprintf(file, "aliases %s %s\n", cmd_aliases[i].alias, cmd_aliases[i].command);
        }
    }

    fclose(file);
}

void load_alias(int *aliases_count) {
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/.aliases", getenv("HOME"));
    FILE *file = fopen(path, "r");

    if (!file) {
        perror("Error opening alias file");
        return;
    }

    char line[MAX_CMD_CHARS];
    *aliases_count = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        char alias_name[MAX_CMD_CHARS];
        char alias_cmd[MAX_CMD_CHARS];

        if (sscanf(line, "aliases %s %[^\n]", alias_name, alias_cmd) == 2) {
        trim(alias_name);
        trim(alias_cmd);
        add_alias(alias_name, alias_cmd);
        (*aliases_count)++;
    }
}

fclose(file);
}
