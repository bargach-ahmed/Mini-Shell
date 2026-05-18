#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define TOK_DELIM " \t\r\n\a"
#define TOK_BUFSIZE 64

extern char **environ;

/* Aesthetic constants */
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_WHITE   "\x1b[37m"
#define COLOR_BOLD    "\x1b[1m"
#define COLOR_RESET   "\x1b[0m"

/**
 * print_prompt - Prints the aesthetic shell prompt
 */
void print_prompt(void)
{
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s%sAntigravityShell%s %s%s%s%s$ ",
               COLOR_BOLD, COLOR_CYAN, COLOR_RESET,
               COLOR_BOLD, COLOR_MAGENTA, cwd, COLOR_RESET);
    } else {
        perror("getcwd");
    }
}

/**
 * tokenize_input - Splits a string into an array of tokens
 * @line: The string to tokenize
 * Return: A dynamically allocated array of pointers to tokens
 */
char **tokenize_input(char *line)
{
    int bufsize = TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;

    if (!tokens) {
        fprintf(stderr, "minishell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            char **new_tokens;
            bufsize += TOK_BUFSIZE;
            new_tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!new_tokens) {
                fprintf(stderr, "minishell: allocation error\n");
                free(tokens);
                exit(EXIT_FAILURE);
            }
            tokens = new_tokens;
        }

        token = strtok(NULL, TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

/**
 * shell_cd - Built-in command to change directory
 * @args: Array of arguments, args[1] is the target directory
 * Return: Always returns 1 to continue execution
 */
int shell_cd(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "minishell: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("minishell");
        }
    }
    return 1;
}

/**
 * shell_env - Built-in command to print environment variables
 * @args: Array of arguments (unused)
 * Return: Always returns 1 to continue execution
 */
int shell_env(char **args)
{
    int i;
    (void)args; /* suppress unused variable warning */
    for (i = 0; environ[i] != NULL; i++) {
        printf("%s\n", environ[i]);
    }
    return 1;
}

/**
 * shell_exit - Built-in command to exit the shell
 * @args: Array of arguments (unused)
 * Return: Always returns 0 to terminate execution loop
 */
int shell_exit(char **args)
{
    (void)args;
    return 0;
}

/**
 * execute_process - Forks a child process and executes the command
 * @args: Array of arguments where args[0] is the command
 * Return: 1 to continue execution, 0 to terminate
 */
int execute_process(char **args)
{
    pid_t pid, wpid;
    int status;

    if (args[0] == NULL) {
        /* An empty command was entered */
        return 1;
    }

    /* Check for built-in commands */
    if (strcmp(args[0], "cd") == 0) {
        return shell_cd(args);
    } else if (strcmp(args[0], "env") == 0) {
        return shell_env(args);
    } else if (strcmp(args[0], "exit") == 0) {
        return shell_exit(args);
    }

    /* Fork a process for external commands */
    pid = fork();
    if (pid == 0) {
        /* Child process */
        if (execvp(args[0], args) == -1) {
            perror("minishell");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        /* Error forking */
        perror("minishell");
    } else {
        /* Parent process */
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
            if (wpid == -1) {
                perror("waitpid");
                break;
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

/**
 * shell_loop - The main REPL loop of the shell
 */
void shell_loop(void)
{
    char *line = NULL;
    char **args;
    int status = 1;
    size_t len = 0;
    ssize_t read;

    do {
        print_prompt();

        /* Use getline to read dynamic input */
        read = getline(&line, &len, stdin);
        
        /* Handle EOF (Ctrl+D) */
        if (read == -1) {
            printf("\n");
            break;
        }

        args = tokenize_input(line);
        status = execute_process(args);

        /* Free dynamically allocated memory for tokens */
        free(args);
    } while (status);

    /* Free dynamically allocated memory for line buffer */
    free(line);
}

/**
 * main - Entry point for the minishell
 * Return: EXIT_SUCCESS
 */
int main(void)
{
    /* Run command loop */
    shell_loop();

    return EXIT_SUCCESS;
}
