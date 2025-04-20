#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "shell.h"

char *original_path = NULL;

void handle_command(char *tokens[], int token_count) {
    if (strcmp(tokens[0], "exit") == 0) {
        printf("Exiting Shell...\n");
        restore_original_path();
        save_history(cmd_history, history_count);
        save_aliases(aliases, alias_count);
        exit(0);
    }
    else if (strcmp(tokens[0], "echo") == 0){
        echo(tokens,token_count);
    }
    else if (strcmp(tokens[0], "history") == 0) {
        if (token_count > 1)
            printf("Error: history takes no arguments!\n");
        else
            print_history_commands();
    }
    else if (strcmp(tokens[0], "getpath") == 0) {
        if (token_count > 1) {
            fprintf(stderr, "Error: getpath takes no arguments.\n");
        } else {
            execute_getpath();
        }
    }
    else if (strcmp(tokens[0], "setpath") == 0) {
        if (token_count == 2) {
            execute_setpath(tokens[1]);
        } else {
            fprintf(stderr, "Error: setpath requires exactly one argument.\n");
        }
    }
    else if (strcmp(tokens[0], "pwd") == 0) {
        if (token_count > 1) {
            fprintf(stderr, "Error: pwd requires no argument.\n");
        } else {
            execute_pwd();
        }
    }
    else if (strcmp(tokens[0], "cd") == 0) {
        execute_cd(tokens, token_count);
    }
    else if (tokens[0][0] == '!') {
        if (tokens[0][1] == '\0') {
            printf("arguments needed: \n"
                   " 1. !! (executes the last command)\n"
                   " 2. !-<num> (Executes the command N position behind)\n"
                   " 3. !<num> (Executes command at Nth position)\n"
                   "\n Type \"history\" to check your history of commands\n");
        } else if (history_count == 0) {
            printf("Error: No history of commands!\n");
        } else {
            char *exe_cmd = execute_invocation_command(tokens);
            if (exe_cmd != NULL) {
                char *get_History[MAX_CMD_CHARS];
                int count = Tokenization_input(get_History, exe_cmd);
                if (count > 0) {
                    execute_command(get_History);
                } else {
                    printf("Error: Can't load the history command\n");
                }
                free(exe_cmd);
            }
        }
    }
    else if (strcmp(tokens[0], "alias") == 0) {
        if (token_count == 1) {
            print_aliases();
        } else if (token_count >= 3) {
            char alias_command[MAX_CMD_CHARS] = "";
            for (int i = 2; i < token_count; i++) {
                strcat(alias_command, tokens[i]);
                if (i < token_count - 1) strcat(alias_command, " ");
            }
            add_alias(tokens[1], alias_command);
        } else {
            printf("Usage: alias <name> <command>\n");
        }
    }
    else if (strcmp(tokens[0], "unalias") == 0) {
        if (token_count == 2) {
            remove_alias(tokens[1]);
        } else {
            printf("Usage: unalias <name>\n");
        }
    }
    else {
        execute_command(tokens);
    }
}

void echo(char* commands[], int count){
    for (int i = 1; i < count; i++){
        printf("%s", commands[i]);
        if (i < count-1)
            printf(" ");
    }
    printf("\n");
}

void execute_getpath() {
    char *current_path = getenv("PATH");
    if (current_path != NULL) {
        printf("Current PATH: %s\n", current_path);
    } else {
        fprintf(stderr, "Error: Unable to retrieve the PATH environment variable.\n");
    }
}

void execute_setpath(char *new_path) {
    if (new_path == NULL || strlen(new_path) == 0) {
        fprintf(stderr, "Error: setpath requires a valid argument.\n");
        return;
    }

    if (setenv("PATH", new_path, 1) == 0) {
        printf("PATH successfully set to: %s\n", new_path);
    } else {
        perror("Error setting PATH");
    }

    char *current_path = getenv("PATH");
    if (current_path != NULL) {
        printf("Updated PATH: %s\n", current_path);
    } else {
        fprintf(stderr, "Error retrieving updated PATH.\n");
    }
}

int Tokenization_input(char* tokens[], char* user_input) {
    int token_count = 0;
    char *token = user_input;
    while (*token != '\0') {
        while (strchr(" \t<>;&|", *token)) token++;
        if (*token == '\0') break;

        if (*token == '"') {
            token++;
            char *start = token;
            while (*token != '"' && *token != '\0') token++;
            if (*token == '"') {
                *token = '\0';
                tokens[token_count++] = start;
                token++;
            } else {
                fprintf(stderr, "Error: Mismatched quotes.\n");
                return -1;
            }
        } else {
            char *start = token;
            while (!strchr(" \t<>;&|", *token) && *token != '\0') token++;
            if (*token != '\0') *token++ = '\0';
            tokens[token_count++] = start;
        }
    }
    tokens[token_count] = NULL;
    return token_count;
}

void execute_pwd() {
    char cwd[MAX_CMD_CHARS];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current directory: %s\n", cwd);
    } else {
        perror("Error getting current directory\n");
    }
}

void execute_cd(char *tokens[], int token_count) {
    if (token_count == 1) {
        char *home_dir = getenv("HOME");
        if (home_dir == NULL) {
            fprintf(stderr, "Error: HOME environment variable not set.\n");
            return;
        }
        if (chdir(home_dir) != 0) {
            perror("Error changing to HOME directory");
        }
    } else if (token_count == 2) {
        if (chdir(tokens[1]) != 0) {
            fprintf(stderr, "cd: %s ", tokens[1]);
            perror("");
        }
    } else {
        fprintf(stderr, "Error: cd takes at most one argument.\nUsage: cd <directory>\n");
    }
}

void restore_original_path() {
    if (original_path != NULL) {
        if (setenv("PATH", original_path, 1) == 0) {
            printf("PATH restored to original value: %s\n", original_path);
        } else {
            perror("Error restoring original PATH\n");
        }
        free(original_path);
    }
}

void execute_command(char *tokens[]) {
    pid_t fork_result = fork();
    if (fork_result < 0) {
        perror("Fork failed");
        exit(1);
    } else if (fork_result == 0) {
        execvp(tokens[0], tokens);
        fprintf(stderr, "'%s': Command not found\n", tokens[0]);
        exit(127);
    } else {
        int status;
        waitpid(fork_result, &status, 0);
    }
}
