#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "history.h"
#include "shell.h"
#include "alias.h"

char cmd_history[MAX_HISTORY][MAX_CMD_CHARS];
int history_count = 0;
int history_index = 0;

void command_history(char *commands) {
    if (strlen(commands) == 0 || commands[0] == '!') return;
    strncpy(cmd_history[history_index], commands, MAX_CMD_CHARS);
    history_index = (history_index + 1) % MAX_HISTORY;
    if (history_count < MAX_HISTORY) history_count++;
}

void print_history_commands() {
    if (history_count == 0) {
        printf("Error: No history of commands!!\n");
        return;
    }
    int index_start = (history_index - history_count + MAX_HISTORY) % MAX_HISTORY;
    for (int i = 0; i < history_count; i++) {
        printf("%d. %s\n", i + 1, cmd_history[index_start]);
        index_start = (index_start + 1) % MAX_HISTORY;
    }
}

void save_history(char cmd_history[][MAX_CMD_CHARS], int history_count) {
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/.hist_list", getenv("HOME"));
    FILE *file = fopen(path, "w");
    if (!file) {
        perror("file cannot be opened");
        return;
    }
    for (int i = 0; i < history_count; i++) {
        fprintf(file, "%s\n", cmd_history[i]);
    }
    fclose(file);
}

void load_history(char cmd_history[][MAX_CMD_CHARS], int *history_count) {
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/.hist_list", getenv("HOME"));
    FILE *file = fopen(path, "r");
    if (!file) {
        perror("Unable to open history file");
        return;
    }
    char line[MAX_CMD_CHARS];
    *history_count = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) > 0) {
            strncpy(cmd_history[*history_count], line, MAX_CMD_CHARS);
            (*history_count)++;
            if (*history_count >= MAX_HISTORY) break;
        }
    }
    fclose(file);
    history_index = *history_count % MAX_HISTORY;
}

char* execute_invocation_command(char *invoke_command[]) {
    int index = 0;
    if (invoke_command[1] != NULL) {
        printf("Error: Invocation takes no arguments!\n");
        return NULL;
    }
    if (strcmp(invoke_command[0], "!!") == 0) {
        index = (history_index - 1 + MAX_HISTORY) % MAX_HISTORY;
    } else if (invoke_command[0][0] == '!' && isdigit(invoke_command[0][1])) {
        int N = atoi(invoke_command[0] + 1);
        if (N <= 0 || N > history_count) {
            printf("Error: Invalid history invoke number\n");
            return NULL;
        }
        int startIndex = (history_index - history_count + MAX_HISTORY) % MAX_HISTORY;
        index = (startIndex + (N - 1)) % MAX_HISTORY;
    } else if (invoke_command[0][1] == '-' && isdigit(invoke_command[0][2])) {
        int N = atoi(invoke_command[0] + 2);
        if (N <= 0 || N > history_count) {
            printf("Error: Invalid history invoke number\n");
            return NULL;
        }
        index = (history_index - N + MAX_HISTORY) % MAX_HISTORY;
    } else {
        printf("Error: Invalid history invoke number\n");
        return NULL;
    }

    char *hist_cmd = cmd_history[index];
    char *temp_cmd[MAX_CMD_CHARS];
    int count = Tokenization_input(temp_cmd, hist_cmd);
    char *resolved_cmd = resolve_alias(temp_cmd[0]);
    printf("Executing command: '%s'\n", hist_cmd);

    if (strcmp(resolved_cmd, temp_cmd[0]) != 0) {
        char full_cmd[MAX_CMD_CHARS] = "";
        strcat(full_cmd, resolved_cmd);
        for (int i = 1; i < count; i++) {
            strcat(full_cmd, " ");
            strcat(full_cmd, temp_cmd[i]);
        }
        free(resolved_cmd);
        return strdup(full_cmd);
    }
    free(resolved_cmd);
    return strdup(hist_cmd);
}
