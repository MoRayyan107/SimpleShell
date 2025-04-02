#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <limits.h>
#include "Alias.h"

#define MAX_CMD_CHARS 512 // max chars for command line
#define MAX_ARGS 50 // max amount for Tokens
#define MAX_HISTORY 20 // max amount to store history commands

void execute_getpath();
void execute_setpath(char *new_path);
void execute_pwd();
void restore_original_path();
void execute_command(char* tokens[]);
int Tokenization_input(char* tokens[], char* user_input);
char* execute_invocation_command(char* invoke_command[]);
void execute_cd(char *tokens[], int token_count);
void command_history(char *commands);
void print_history_commands();

void save_history(char cmd_history[][MAX_CMD_CHARS], int history_count);
void load_history(char cmd_history[][MAX_CMD_CHARS], int *history_count);
void trim(char *str);

// variable that stores the original path
char *original_path = NULL;

// using the Alias header file
Alias aliases[MAX_ALIASES]; // Array storing alias name and command
int alias_count = 0; // keeping track of aliases

// 2D array to store the commands entered by the user
char cmd_history[MAX_HISTORY][MAX_CMD_CHARS];
int history_count = 0; // tacks the count of commands stored in the array
int history_index = 0; // track for circular array loop

/**
 * MAIN SHELL START
 */
int main() {
    char user_input[MAX_CMD_CHARS];

    // Saves original PATH
    original_path = getenv("PATH");
    if (original_path != NULL) {
        original_path = strdup(original_path);
    } else {
        fprintf(stderr, "Error: unable to retrieve the PATH environment.\n");
        exit(EXIT_FAILURE);
    }

    // Set the current directory to HOME
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

        // Check if input exceeds max characters
        if (user_input[strlen(user_input) - 1] != '\n') {
            fprintf(stderr, "Error: Maximum characters reached (%d capacity)\n", MAX_CMD_CHARS);
            while (getchar() != '\n');
            continue;
        }

        user_input[strcspn(user_input, "\n")] = 0; // Remove newline
        if (strlen(user_input) == 0) continue;    // Ignore empty input

        command_history(user_input);

        // Tokenize user input
        char *tokens[MAX_ARGS];
        int token_count = Tokenization_input(tokens, user_input);
        tokens[token_count] = NULL; // Null-terminate tokens array

        // Ensure commands can run
        if (token_count == 0) {
            fprintf(stderr, "Error: No tokens found.\n");
        } else {
            // Resolve alias
            char *resolved_command = resolve_alias(tokens[0]);
            if (strcmp(resolved_command, tokens[0]) != 0) {
                token_count = Tokenization_input(tokens, resolved_command);
                tokens[token_count] = NULL; // Null-terminate after re-tokenizing
            }

            // Internal command handling
            if (strcmp(tokens[0], "exit") == 0) {
                printf("Exiting Shell...\n");
                restore_original_path();
                save_history(cmd_history, history_count);
                save_aliases(aliases, alias_count);
                exit(0);
            }
            // Check for 'history' command
            else if (strcmp(tokens[0], "history") == 0){
                if (token_count > 1)
                    printf("Error: history take no arguments!\n");
                else
                    print_history_commands();
            }
            // Check for 'getpath' command
            else if (strcmp(tokens[0], "getpath") == 0) {
                if (token_count > 1) {
                    fprintf(stderr, "Error: getpath takes no arguments.\n");
                } else {
                    execute_getpath();
                }
            }
            // Check for 'setpath' command
            else if (strcmp(tokens[0], "setpath") == 0) {
                if (token_count == 2) { // Ensure setpath has exactly one argument
                    execute_setpath(tokens[1]);
                } else {
                    fprintf(stderr, "Error: setpath requires exactly one argument.\n");
                }
            }
            // Check for 'pwd' command
            else if (strcmp(tokens[0], "pwd") == 0){
                if (token_count > 1){ // checks if it has no arguments
                    fprintf(stderr, "Error: pwd requires no argument.\n");
                } else {
                    execute_pwd();
                }
            }
            // Check for 'cd' command
            else if(strcmp(tokens[0], "cd") == 0){ // if user called cd
                execute_cd(tokens, token_count);
            }
            // Check for invoke command '!!, !<num>, !-<num>'
            else if(tokens[0][0] == '!') {
                if (tokens[0][0] == '!' && tokens[0][1] == '\0') {
                    printf("arguments needed: \n"
                           " 1. !! (executes the last command)"
                           "\n 2. !-<num> (Executes the command N position behind)"
                           "\n 3. !<num> (Executes command at Nth position)\n"
                           "\n Type \"history\" to check your history of commands\n");

                }
                else if (history_count == 0) {// if the history count is 0
                    printf("Error: No history of commands!\n");
                }
                else if (history_count > 0) {
                    char *exe_cmd = execute_invocation_command(tokens); // get the invocation command
                    if (exe_cmd != NULL) {
                        char *get_History[MAX_CMD_CHARS]; // temp variable
                        int count = Tokenization_input(get_History, exe_cmd); // get the count and tokenise
                        if (count > 0) {
                            execute_command(get_History);
                        } else {
                            printf("Error: Can't load the history command\n");
                        }
                    }
                }
            }
            // Check for 'alias' command
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
            // Check for 'unalias' command
            else if (strcmp(tokens[0], "unalias") == 0) {
                if (token_count == 2) {
                    remove_alias(tokens[1]);
                } else {
                    printf("Usage: unalias <name>\n");
                }
            }
            else {
                // Execute external command
                execute_command(tokens);
            }
            free(resolved_command); // free the resolve for no mem leaks
        }
    }
}

/**
 * This function prints out users current path
 * Checks if path is NULL
 * if path is NULL: prints an Error
 * if path is NOT NULL: prints user current path
 *
 * Dependencies:
 *   - getenv() to get users current path
 */
void execute_getpath() {
    char *current_path = getenv("PATH");
    if (current_path != NULL) {
        printf("Current PATH: %s\n", current_path);
    } else {
        fprintf(stderr, "Error: Unable to retrieve the PATH environment variable.\n");
    }
}

/**
 * This function totally set new path.
 *
 * This function validates user input, updates the system's PATH variable,
 * and provides feedback on success or failure. It includes error handling
 * for invalid inputs and system-level failures.
 *
 * How it works?
 * - Checks for NULL and empty String and prints error if so.
 * - using setenv() to overwrite the original path
 * - Gives a success message if the path is set
 * - Verifies and prints new updated path
 *
 * @param new_path user input path
 *
 * Dependencies:
 *  - setenv() to change users current path
 *  - getenv() to get users current path for verification
 *  - perror() printing error message
 */
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

    // Confirm the change by printing the updated PATH
    char *current_path = getenv("PATH");
    if (current_path != NULL) {
        printf("Updated PATH: %s\n", current_path);
    } else {
        fprintf(stderr, "Error retrieving updated PATH.\n");
    }
}

/**
 * Function to tokenize user input into individual commands or arguments
 *
 * This function breaks down user input into individual
 * tokens based on delimiters, space, tabs, special characters
 * eg -> '<' , '>' , ':' , '&' , '|'
 * the tokenised string stores in a pointer array
 *
 * @param tokens an Array of pointers to store tokenised strings.
 *               Array has a capacity of 50 elements
 * @param user_input a raw input by user
 *
 * @return the number of tokens stored in an array
 *         Returns -1 if its a mismatch quote
 */
int Tokenization_input(char* tokens[], char* user_input) {
    int token_count = 0; // keeping track of tokens
    char *token = user_input; // Pointer to traverse through the input string
    while (*token != '\0') {
        // Skip leading delimiters
        while (*token == ' ' || *token == '\t' || *token == '<' || *token == '>' || *token == ';' ||
               *token == '&' || *token == '|') {
            token++; // Move the pointer forward to skip delimiters
        }
        // Check if we've reached the end of the string after skipping delimiters
        if (*token == '\0') {
            break; // If no more tokens are left, exit the loop
        }
        // Handle quoted strings (e.g., "ls -l") as a single token
        if (*token == '"') {
            token++; // Skip the opening quote
            char *start = token; // Mark the start of the quoted string
            while (*token != '"' && *token != '\0') {
                token++; // Traverse the string until a closing quote or the end of the input
            }
            if (*token == '"') { // If a closing quote is found
                *token = '\0'; // Replace the closing quote with a null terminator
                tokens[token_count++] = start;
                token++; // Move the pointer past the closing quote
            } else {
                // Handle cases where quotes are mismatched (no closing quote found)
                fprintf(stderr, "Error: Mismatched quotes.\n");
                return -1;
            }
        } else {
            // Handle unquoted tokens
            char *start = token; // Mark the start of the unquoted token
            while (*token != ' ' && *token != '\t' && *token != '<' && *token != '>' &&
                   *token != ';' && *token != '&' && *token != '|' && *token != '\0') {
                token++; // Traverse until a delimiter or the end of the string
            }
            if (*token != '\0') {
                *token = '\0';
                token++; // Move the pointer past the delimiter
            }
            tokens[token_count++] = start;
        }
    }
    tokens[token_count] = NULL; // Null-terminate the tokens array for compatibility with execvp()

    return token_count;
}

/**
 * Function to print the current working directory
 *
 * This function give the absolute path of users directory
 * uses grtcwd() function to print standard output
 *
 * If any chance of not retrieving the current directory
 * an error message will be printed using perror()
 *
 * Dependencies:
 *    - getcwd() gets the current working directory of the user
 */
void execute_pwd() {
    char cwd[MAX_CMD_CHARS];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current directory: %s\n", cwd);
    } else {
        perror("Error getting current directory\n");
    }
}

/**
 * Function to restore the original PATH before exiting
 *
 * This function is called when exiting the shell,
 * it restores the path to original value
 *
 * This function uses setenv() function to set PATH environment variable
 * if its successful, prints a confirmation that the path is restored
 * If not, perror() executes
 *
 * at End it frees the memory to prevent memory leaks
 *
 * Dependencies:
 *   - setenv() to overwrite the current path to original path
 *   - perror() - to display error if failed to restore
 */
void restore_original_path() {
    if (original_path != NULL) {
        if (setenv("PATH", original_path, 1) == 0) {
            printf("PATH restored to original value: %s\n", original_path);
        } else {
            perror("Error restoring original PATH\n");
        }
        free(original_path); // Free the memory allocated for the original PATH
    }
}

/**
 * This function helps to execute a command using Child process
 * takes in tokenised input and forks it
 * Checks if its a valid command,
 * If yes, executes it
 * If not prints <Command_typed>: Command not found
 *
 * @param tokens takes in tokenised command
 *
 * Dependencies:
 *  - execvp() executing commands
 *  - waitpid() wait for the child process to finish
 */
void execute_command(char *tokens[]) {
    pid_t fork_result = fork(); // Create a new process
    if (fork_result < 0) {
        perror("Fork failed");
        exit(1);
    } else if (fork_result == 0) { // Child process
        execvp(tokens[0], tokens); // Execute command
        fprintf(stderr, "\'%s\': Command not found\n", tokens[0]);
        exit(127); // Exit code for "Command not found"
    } else {
        int status;
        waitpid(fork_result,&status,0);
    }
}

/**
 * This function inserts a raw user command into a Circular Array
 * @param commands takes in raw user input
 *
 * the circular array is,
 * char cmd_history[MAX_HISTORY][MAX_CMD_CHARS];
 * MAX_HISTORY -> 20 and MAX_CMD_CHARS -> 512
 *
 * This function checks a valid command length before inserting into circular array
 * If the command is an invocation (!! ,!<num> or !-<num>), it wont store in the Array
 * if user input is valid, it stores in the Array
 * and increments history count by 1
 *
 * Dependencies:
 *  - strlen() check if command entered by is empty
 */
void command_history(char* commands){
    if(strlen(commands) == 0){ // if length of command is 0
        return; // return back to the main method
    }
    // check if the command is an invocation if so dont add
    if (commands[0] == '!') {
        return;
    } else {
        // copy the user input command into cmd_history starting index 0
        strncpy(cmd_history[history_index], commands, MAX_CMD_CHARS);

        // calculate the index of the circular 2D array
        history_index = (history_index + 1) % MAX_HISTORY;

        // keep track of the count
        if (history_count < MAX_HISTORY){
            history_count++;
        }
    }
}

/**
 * This  function takes an un-tokenised input from history array
 * checks if invocation is valid, i.e no arguments
 *
 * history index number of commands stored in history
 * User inputs are in String format
 *
 * Check for '!!':
 *  calculates the index to executes latest command in the array
 *  and stores it in variable 'Index'
 *
 * Check of '!<Num>':
 *  checks if entered invoke command is valid,
 *  converts the ASCII Character to integer
 *  Checks if invoke number surpasses the history count (Ex !936 -> INVALID)
 *  And if invoke number is '0' (Ex !-0 -> INVALID)
 *  if invoke number is valid, calculate the index with N
 *  and stores it in variable 'Index'
 *
 * Check for '!-<Num>;:
 *  checks if entered invoke command is valid
 *  converts ASCII character to Integer
 *  checks if invoke number surpasses the history count (Ex !-936 -> INVALID)
 *  And if invoke number is '0' (Ex !-0 -> INVALID)
 *  if invoke number is valid, calculate the index with N
 *  and stores it in variable 'Index'
 *
 * After all these checks it then executes the command
 * thats present in the History array
 *
 * @param invoke_command takes in the tokenised command
 * @return the un-tokenised command form history array
 *
 * Dependencies:
 *  - atoi() to convert ASCII integer character to Integer
 *  - strcmp() to compare if the command invoked is a match
 */
char* execute_invocation_command(char* invoke_command[]){
    int index = 0; // calculation of index
    if (invoke_command[1] != NULL){
        printf("Error: Invocation takes no arguments!\n");
        return NULL;
    }
    if (strcmp(invoke_command[0], "!!") == 0){ // if invoke is "!!"
        index = (history_index - 1 + MAX_HISTORY) % MAX_HISTORY; // calculate the index
    }
    else if(invoke_command[0][0] == '!' && isdigit(invoke_command[0][1])){ // if invoke is "!<num>"
        int N = atoi(invoke_command[0] + 1);
        if (N <= 0 || N > history_count){ // check if N is correctly inputted by user
            printf("Error: Invalid history invoke number\n");
            return NULL;
        }
        int startIndex = (history_index - history_count + MAX_HISTORY) % MAX_HISTORY;
        index = (startIndex+(N-1)) % MAX_HISTORY; // calculate the index with N value
    }
    else if(invoke_command[0][1] == '-' && isdigit(invoke_command[0][2])){ // if invoke is "!-<num>"
        int N = atoi(invoke_command[0] + 2);
        if (N <= 0 || N > history_count){ // check if N is correctly inputted by user
            printf("Error: Invalid history invoke number\n");
            return NULL;
        }
        index = (history_index - N  + MAX_HISTORY) % MAX_HISTORY; // calculate the index with N value
    }
    else{
        printf("Error: Invalid history invoke number\n");
        return NULL;
    }

    // check if the invoking has "history" "pwd" "cd" command
    char* hist_cmd = cmd_history[index];
    char* temp_cmd[MAX_CMD_CHARS];
    int count = Tokenization_input(temp_cmd, hist_cmd);

    // resolve the command if alias
    // will return same input if its not an alias
    char* resolved_cmd = resolve_alias(temp_cmd[0]);
    printf("Executing command: \'%s\'\n", hist_cmd);

    // If alias exists, replace first word with expanded command
    if (strcmp(resolved_cmd, temp_cmd[0]) != 0) {
        char full_cmd[MAX_CMD_CHARS] = ""; // create a string that stores the alias
        strcat(full_cmd, resolved_cmd); // concat the string in full_cmd
        for (int i = 1; i < count; i++) {
            strcat(full_cmd, " "); // add space after each append
            strcat(full_cmd, temp_cmd[i]);
        }
        hist_cmd = strdup(full_cmd);  // Update hist_cmd
    }
    if (strcmp(hist_cmd, "history") == 0){ // if invoked command has "history"
        print_history_commands();
        return NULL;
    } else if(strcmp(hist_cmd, "pwd") == 0){ // if invoked command is "pwd"
        execute_pwd();
        return NULL;
    } else if(strncmp(hist_cmd, "cd",2) == 0){ // if invoke command is "cd"
        char* temp_cmd[MAX_CMD_CHARS];
        int count = Tokenization_input(temp_cmd,hist_cmd);
        if (count > 0) {
            execute_cd(temp_cmd,count);
        }
        return NULL;
    }
    free(resolved_cmd);
    return hist_cmd;
}

/**
 * handles to change directories
 *
 * @param tokens an array of strings having command and arguments
 * @param token_count number of tokens in the array
 *
 * Dependencies:
 *    - getenv() gets the users home directory
 *    - chdir() checks if directory is a valid directory or not
 */
void execute_cd(char *tokens[], int token_count) {
    if (token_count == 1) {
        // No arguments - change to HOME directory
        char *home_dir = getenv("HOME");
        if (home_dir == NULL) { // ensures that chdir only runs with a valid directory, preventing a crash/error
            fprintf(stderr, "Error: HOME environment variable not set.\n"); // if no HOME directory exists
            return;
        }
        if (chdir(home_dir) != 0) {
            perror("Error changing to HOME directory"); // clarifies any unusual behaviour when using chdir, easy to dubug
        }
    } else if (token_count == 2) {
        // One argument: change to specified directory
        if (chdir(tokens[1]) != 0) {
            fprintf(stderr,"cd: %s ", tokens[1]);
            perror("");
        }
    } else {
        // Too many arguments
        fprintf(stderr, "Error: cd takes at most one argument.\n"
                        "Usage: cd <directory>\n");
    }
}

/**
 * Printing history of commands
 *
 * This function iterates through the command history and prints each command
 * alongside with index number of each command in ascending order
 * The newer commands are added at bottom of the array.
 *
 * If history count is 0, it prints an error message Stating
 * "Error: No history of commands!!"
 *
 * When its printed, its Format is :
 *     <number>. <command>
 */
void print_history_commands() {
    if (history_count == 0) { // check if the commands entered is 0
        printf("Error: No history of commands!!");
    } else{
        // calculate the start index of the circular array
        int index_start = (history_index - history_count + MAX_HISTORY) % MAX_HISTORY;
        for (int i = 0; i < history_count; i++) { // loop through the array
            printf("%d. %s\n", i + 1, cmd_history[index_start]); // print the command with number
            index_start = (index_start+1) % MAX_HISTORY;   // increment the index by 1
        }
    }
}

/**
 * save history to a file in users directory
 *
 * This function saves the current shell command history in a file
 * in Users home directory
 * Each command is stored in this format:
 *     <command>
 *
 * @param cmd_history a 2D Array storing command history
 * @param history_count total number of commands in a history array
 *
 * Dependencies:
 *     - snprintf() construct the file path
 *     - fclose(), fprintf() and fopen() for file management
 */
void save_history(char cmd_history[][MAX_CMD_CHARS], int history_count){
    char path[PATH_MAX]; // Buffer for current working directory
    snprintf(path, sizeof(path), "%s/.hist_list", getenv("HOME"));
    FILE * file = fopen (path, "w" ); // open the file in writing mode

    if(file == NULL){
        perror("file cannot be open");
        return; // exit when can't open the file
    }

    for(int i = 0; i < history_count; i++){
        fprintf(file, " %s\n", cmd_history[i]);   // writing command
    }
    fclose(file);// close the file
}

/**
 * loads history file from users home directory
 *
 * This function loads the history commands form saved file
 * Checks if files exists,
 * if not perror() message will print "Error opening history file"
 * iterates though each line of commands, removing the newline character after each line
 *
 * @param cmd_history a 2D Array storing command history
 * @param history_count a pointer to history count
 *
 * Dependencies:
 *  - snprintf() construct the file path
 *  - fclose(), sscanf() and fopen() for file management
 *  - strcspn() to remove newline character
 *  - strcpy() to copy file commands to array of history
 */
void load_history(char cmd_history[][MAX_CMD_CHARS], int *history_count) {
    char path[PATH_MAX]; // Buffer for current working directory
    snprintf(path, sizeof(path), "%s/.hist_list", getenv("HOME")); // store a string into buffer

    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror("Error opening history file");
        return;
    }
    char line[MAX_CMD_CHARS];  // Buffer to read lines
    *history_count = 0;  // Reset history count

    while (fgets(line, sizeof(line), file) != NULL) {
        // Remove the newline character from the end of the line
        line[strcspn(line, "\n")] = 0;
        int number;
        char command[MAX_CMD_CHARS];
        if (sscanf(line, "%d %[^\n]", &number, command) == 2) {
            // Copy the command into the history array
            strncpy(cmd_history[*history_count], command, MAX_CMD_CHARS);
            (*history_count)++;
            // Stop if the array is full
            if (*history_count >= MAX_HISTORY) {
                break;
            }
        }
    }
    fclose(file);
}

/**
 * adding alias in an array
 *
 * This function add an alias and its corresponding command
 * it trims the alias name and command for trailing whitespaces
 * Checks if alias already exists, if so, it updates the command
 * If not, checks if alias array has reached its maximum value
 * If so, prints out "Error: Maximum number of aliases reached."
 * If not, it adds into the alias array
 *
 * MAX_ALIASES -> 10
 *
 * @param name to set alias name
 * @param cmd command corresponding the alias
 *
 * Dependencies:
 *     - trim() to trim trailing whitespaces
 *     - strcmp() to compare is given name matches the alias name
 *     - strcpy() to copy the alias and command into the array
 */
void add_alias(char *name, char *cmd) {
    trim(name);
    trim(cmd);
    // Check if alias already exists
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].alias, name) == 0) {
            // If alias already exists, update command
            strcpy(aliases[i].command, cmd);
            printf("Alias '%s' updated.\n", name);
            return;
        }
    }
    // If alias doesn't exist, check for space
    if (alias_count < MAX_ALIASES) {
        strcpy(aliases[alias_count].alias, name);
        strcpy(aliases[alias_count].command, cmd);
        alias_count++;
        printf("Alias '%s' added.\n", name);
    } else {
        printf("Error: Maximum number of aliases reached.\n");
    }
}

/**
 * removes alias from alias array
 *
 * This function searches for given name in alias array
 * If alias is found, removes the alias from array and shifts elements
 * If alias not found, prints error message "Error: No such alias"
 *
 * @param name alias name to be removed from array
 *
 * Dependencies:
 *     - strcmp() to compare if given name is alias name
 */
void remove_alias(char *name) {
    int found = 0; // Flag to track if alias is found
    if (alias_count > 0) {
        for (int i = 0; i < alias_count; i++) {
            if (strcmp(aliases[i].alias, name) == 0) {
                found = 1; // Mark alias as found
                // Shift aliases to remove current one
                for (int j = i; j < alias_count - 1; j++) {
                    aliases[j] = aliases[j + 1];
                }
                alias_count--;
                printf("Alias '%s' removed.\n", name);
                return;
            }
        }
        if (!found)
            printf("Error: No such alias '%s'.\n", name);
    }
}


/**
 * This prints the number of aliases stored in an array
 *
 * This function iterates though the array of aliases,
 * Each alias is printed in this format:
 *     alias_name = 'command'
 *
 * if alias count is 0, it prints "No aliases set."
 *
 * The aliases are stored in this format:
 *     aliases[i].alias -> stores alias names
 *     aliases[i].command -> stores alias corresponding command
 */
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

/**
 * Resolves the alias to its corresponding command
 *
 * @param input alias name for corresponding command
 * @return a string dynamically allocated contains the command
 *         if no alias found, return original input
 *
 * Dependencies:
 *     - strcmp(), checks if input matches the alias name
 *     - strdup() duplicating a resolve command
 */
char *resolve_alias(char *input) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].alias, input) == 0) {
            // Check if alias matches, if it does return the mapped command
            return strdup(aliases[i].command);
        }
    }
    return strdup(input);
}


/**
 * Saves all current aliases into a file in user home directory
 *
 * This function write a file storing information of aliases,
 * with file format,
 *      aliases <alias_name> <command>
 *
 * @param cmd_aliases a array of Alias containing name and command
 * @param aliases_count number of aliases to save in the file
 *
 * Closes file if alias count is equal to 0
 * adds alias to file if its a valid format
 * Will print error if it cannot access the file
 *
 * Dependencies:
 *     - fopen(),fprintf() and fclose() for file management
 *     - snprintf() to construct file in a directory
 */
void save_aliases(Alias cmd_aliases[], int aliases_count ){
    char path[PATH_MAX]; // Buffer for current working directory
    snprintf(path, sizeof(path), "%s/.aliases", getenv("HOME"));
    FILE * file = fopen (path, "w" ); // open the file in writing mode

    if(file == NULL){
        perror("file cannot be open");
        return; // exit when can't open the file
    }

    // if no aliases, i.e empty file
    if(aliases_count == 0){
        fclose(file); // close the file and dont write anything
        return;
    }

    if(aliases_count > 0) {
        for (int i = 0; i < aliases_count; i++) {
            if (strlen(cmd_aliases[i].alias) > 0 && strlen(cmd_aliases[i].command) > 0) {
                fprintf(file, "aliases %s %s\n", cmd_aliases[i].alias, cmd_aliases[i].command);
            }
        }
    }
    fclose(file);// close the file
}

/**
 * This function loads alias file and adds to alias list
 *
 * Reads the alias file from the users home directory,
 * checks if the file aliases is in correct format,
 * and adds valid aliases into alias list
 * file format:
 *        aliases <alias_name> <command>
 *
 * @param aliases_count pointer to track number of aliases
 *
 * Will print error if it cannot read the file
 *
 * Dependencies:
 *     - fopen(),fprintf() and fclose() for file management
 *     - snprintf() to construct file in a directory
 *     - sscanf() to check file format
 *     - trim() to eliminate whitespaces
 */
void load_alias(int *aliases_count){
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/.aliases", getenv("HOME")); // store a string into buffer

    FILE *file = fopen(path, "r"); // open file in read mode
    if (file == NULL) {
        perror("Error opening alias file");
        return;
    }
    char line[MAX_CMD_CHARS];  // read lines
    *aliases_count = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        char alias_name[MAX_CMD_CHARS]; // stores alias keyword
        char alias_cmd[MAX_CMD_CHARS];  // stores command line

        if (sscanf(line,"aliases %s %[^\n]",alias_name,alias_cmd) == 2) {
            if (strlen(alias_name) > 0 && strlen(alias_cmd) > 0) {
                trim(alias_name);
                trim(alias_cmd);
                add_alias(alias_name, alias_cmd);
                (*aliases_count)++;
            }
        }
    }
    fclose(file);
}

/** (Helper Function)
 * This function trims whitespaces of a given alias
 *
 * @param str takes in a string command and alias name
 */
void trim(char *str){
    if(!str) return; // check for NULL
    //trim leading
    char *start = str;
    while (*start && isspace((unsigned char)*start)){
        start++;
    }

    if (*start == '\0'){
        *str = '\0';
        return; // return empty string if string was all spaces
    }

    // trim trailing
    char *end = str + strlen(str) - 1;
    while (end > start && isspace((unsigned char)*end)){
        end--;
    }

    *(end + 1) = '\0'; // new NULL terminator
    if (start > str){ // if there were leading spaces, shift the trimmed string to the beginning
        memmove(str, start, (end - start) + 2); // +2 for the NULL terminator
    }
}