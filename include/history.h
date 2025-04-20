#ifndef HISTORY_H
#define HISTORY_H

#define MAX_CMD_CHARS 512
#define MAX_HISTORY 20

extern char cmd_history[MAX_HISTORY][MAX_CMD_CHARS];
extern int history_count;
extern int history_index;

void command_history(char *commands);
void print_history_commands();
void save_history(char cmd_history[][MAX_CMD_CHARS], int history_count);
void load_history(char cmd_history[][MAX_CMD_CHARS], int *history_count);
char* execute_invocation_command(char *invoke_command[]);

#endif // HISTORY_H