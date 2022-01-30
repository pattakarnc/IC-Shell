#include <stdio.h> 
#include <stdlib.h>
#include <string.h> 
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define	MAX_CMD_CHAR 256

char last_cmd[MAX_CMD_CHAR];    // a string holder for last user's input
char cmd[MAX_CMD_CHAR];         // a string holder for user's input
pid_t pid;

void assign_last_cmd(char* input) {
    input[strcspn(input, "\n")] = 0;         // removing trailing new line from input 
    strcpy(last_cmd, input);                 // assign it to last_cmd
}

// check if the given string is empty or contains only white space
int is_empty(char* str) {
    while (*str) {
        if (!isspace((unsigned char)*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

void exit_with_status(char* status) {
    if (is_empty(status)) {
        printf("Please specify the exit code!\n");
    }
    else {
        printf("bye");
        exit(atoi(status));
    }
}

// source: https://www.geeksforgeeks.org/c-program-to-trim-leading-white-spaces-from-string/
char* trim_leading_spaces(char* str) {
    static char str1[MAX_CMD_CHAR];
    int count = 0, j, k;
    while (str[count] == ' ') {
        count++;
    }
    for (j = count, k = 0;
        str[j] != '\0'; j++, k++) {
        str1[k] = str[j];
    }

    str1[k] = '\0';
    return str1;
}

// source: https://codeforwin.org/2016/04/c-program-to-trim-trailing-white-space-characters-in-string.html
char* trim_trailing_spaces(char* str) {
    int index = -1;
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n') {
            index= i;
        }
        i++;
    }
    str[index + 1] = '\0';
    return str;
}

char** get_cmd_and_args(char* input) {
    char* cleanInput = trim_leading_spaces(input);
    char** cmdAndArgs = malloc(3 * sizeof(char*));
    for (int i = 0; i < 3; i++) {
        cmdAndArgs[i] = malloc(MAX_CMD_CHAR * sizeof(char));
    } 

    char* command = strtok(cleanInput, " ");
    char* argument = strtok(NULL, "\n");

    strcpy(cmdAndArgs[0], trim_trailing_spaces(command));

    // if there is no argument in the input, assign null to the argument string
    if (argument == NULL) {
        cmdAndArgs[1] = '\0';
    }
    // else, trim the leading white space and assign it
    else {
        strcpy(cmdAndArgs[1], trim_leading_spaces(argument)); 
    }

    cmdAndArgs[2] = '\0';    // assign NULL to last element of string array
    return cmdAndArgs;
}

void cmd_handler(char* input) {
    char listOfCmd[3][5] = {"echo", "!!", "exit"};
    char** inputArgs = get_cmd_and_args(input);

    // command = echo
    if (strcmp(inputArgs[0], listOfCmd[0]) == 0) {
        assign_last_cmd(input);
        if (inputArgs[1] == NULL) {
            printf("\n");
        }
        else {
            printf("%s\n", inputArgs[1]);
        }
    } 
    
    // command = !! 
    else if (strcmp(inputArgs[0], listOfCmd[1]) == 0) {

        // if the user has already put other commands (last_cmd != NULL) 
        if (!is_empty(last_cmd)) {
            printf("%s\n", last_cmd);
            cmd_handler(last_cmd);
        }
    }

    // command = exit
    else if (strcmp(inputArgs[0], listOfCmd[2]) == 0) {
        exit_with_status(inputArgs[1]);
    }
    
    else {
        assign_last_cmd(input);
        pid = fork();

        // wait for the child process to terminate (blocking)
        if (pid > 0) {
            waitpid(pid, NULL, 0);  
        }
        else if (pid == 0) {
            int execVal = execvp(inputArgs[0], inputArgs);

            // if the user's command doesn't match with any of the command
            if (execVal == -1) {
                printf("bad command\n");
                exit(EXIT_FAILURE);
            }
        }
        else {
            printf("fork error!\n");
            exit(EXIT_FAILURE);
        }    
    }
}
 
void read_command() {
    printf("icsh $ ");

    // to prevent the system from taking an input before printing
    fflush(stdout);

    fgets(cmd, MAX_CMD_CHAR, stdin);
}

void shell_start() {
    while (1) {
        read_command();

        // if the user passes an empty command or space character
        if (is_empty(cmd)) {
            continue;
        }
        else {
            cmd_handler(cmd);
        }
    }
}

void script_mode_start(char* filename) {
    char* buffer = malloc(MAX_CMD_CHAR * sizeof(char));
    FILE* file = fopen(filename, "r");

    // read the file line by line and execute it  
    if (file) {
        while (!feof(file)) {
            memset(buffer, 0x00, MAX_CMD_CHAR);       // clean the buffer
            fscanf(file, "%[^\n]\n", buffer);         // read a line in file 
            if (is_empty(buffer)) {
                continue;
            }
            else {
                cmd_handler(buffer);           
            }
        }
    }
    else {
        printf("the given program does not exist!\n");
    }
    fclose(file);
    free(buffer);
}

int main(int argc, char *argv[]) {
    if (argv[1]) {
        script_mode_start(argv[1]);
    }
    else {
        printf("Starting IC Shell\n");
        shell_start();
    }
    return 0;
}