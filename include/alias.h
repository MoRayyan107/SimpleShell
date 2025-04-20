#ifndef ALIAS_H
#define ALIAS_H

#define MAX_CMD_CHARS 512
#define MAX_ALIASES 10

typedef struct {
    char alias[MAX_CMD_CHARS];
    char command[MAX_CMD_CHARS];
} Alias;

extern int alias_count;
extern Alias aliases[MAX_ALIASES];

void add_alias(char *name, char *cmd);
void remove_alias(char *name);
void print_aliases();
char *resolve_alias(char *input);
void save_aliases(Alias aliases[], int aliases_count );
void load_alias(int *aliases_count);

#endif //ALIAS_H
