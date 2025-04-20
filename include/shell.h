#ifndef SHELL_H
#define SHELL_H

#include "alias.h"
#include "history.h"

#define MAX_CMD_CHARS 512
#define MAX_ARGS 50

extern char *original_path;

void echo(char* commands[], int count);
int Tokenization_input(char* tokens[], char* user_input);
void handle_command(char *tokens[], int token_count);
void execute_command(char *tokens[]);
void execute_cd(char *tokens[], int token_count);
void execute_pwd();
void execute_getpath();
void execute_setpath(char *new_path);
void restore_original_path();

#endif // SHELL_H