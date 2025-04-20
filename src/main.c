#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shell.h"
#include "alias.h"
#include "history.h"

int main() {
    char user_input[MAX_CMD_CHARS];

    // Save original PATH
    original_path = getenv("PATH");
    if (original_path != NULL) {
        original_path = strdup(original_path);
    } else {
        fprintf(stderr, "Error: unable to retrieve the PATH environment.\n");
        exit(EXIT_FAILURE);
    }

    // Change to HOME directory
    char *home_dir = getenv("HOME");
    if (home_dir != NULL) {
        if (chdir(home_dir) == 0) {
            printf("Current working directory set to HOME: %s\n", home_dir);
        } else {
            perror("Error changing to HOME directory");
        }
    } else {
        fprintf(stderr, "Error: Unable to retrieve the HOME environment variable.\n");
    }

    load_history(cmd_history, &history_count);
    load_alias(&alias_count);

    while (1) {
        char Current_Directory[1024];
        if (getcwd(Current_Directory, sizeof(Current_Directory)) != NULL) {
            printf("%s~$ ", Current_Directory);
            fflush(stdout);
        }

        if (fgets(user_input, MAX_CMD_CHARS, stdin) == NULL) {
            printf("\nExiting Shell...\n");
            restore_original_path();
            save_history(cmd_history, history_count);
            save_aliases(aliases, alias_count);
            exit(0);
        }

        if (user_input[strlen(user_input) - 1] != '\n') {
            fprintf(stderr, "Error: Maximum characters reached (%d capacity)\n", MAX_CMD_CHARS);
            while (getchar() != '\n');
            continue;
        }

        user_input[strcspn(user_input, "\n")] = 0;
        if (strlen(user_input) == 0) continue;

        command_history(user_input);

        char *tokens[MAX_ARGS];
        int token_count = Tokenization_input(tokens, user_input);
        tokens[token_count] = NULL;

        if (token_count == 0) {
            fprintf(stderr, "Error: No tokens found.\n");
            continue;
        }

        char *resolved_command = resolve_alias(tokens[0]);
        if (strcmp(resolved_command, tokens[0]) != 0) {
            token_count = Tokenization_input(tokens, resolved_command);
            tokens[token_count] = NULL;
        }

        handle_command(tokens, token_count);
        free(resolved_command);
    }
    return 0;
}
