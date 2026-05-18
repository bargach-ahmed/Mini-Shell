#include "orbitshell.h"

/* Global builtins array */
t_builtin g_builtins[] = {
    {"cd", shell_cd},
    {"env", shell_env},
    {"exit", shell_exit},
    {NULL, NULL}
};

/**
 * print_prompt - Prints the aesthetic shell prompt
 */
void print_prompt(void)
{
    char cwd[1024];

    if (Getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s%sOrbitShell%s %s%s%s%s$ ",
               COLOR_BOLD, COLOR_CYAN, COLOR_RESET,
               COLOR_BOLD, COLOR_MAGENTA, cwd, COLOR_RESET);
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
    char **tokens = Malloc(bufsize * sizeof(char *));
    char *token;

    token = strtok(line, TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            char **new_tokens;
            bufsize += TOK_BUFSIZE;
            new_tokens = Realloc(tokens, bufsize * sizeof(char *));
            tokens = new_tokens;
        }

        token = strtok(NULL, TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

/**
 * execute_process - Forks a child process and executes the command
 * @args: Array of arguments where args[0] is the command
 * Return: 1 to continue execution, 0 to terminate
 */
int execute_process(char **args)
{
    int i;
    const char *curr_builtin;
    pid_t pid;
    int status;

    if (args[0] == NULL) {
        /* An empty command was entered */
        return 1;
    }

    /* 1) Check for built-in commands */
    i = 0;
    while ((curr_builtin = g_builtins[i].builtin_name)) {
        if (strcmp(args[0], curr_builtin) == 0) {
            return (g_builtins[i].foo)(args);
        }
        i++;
    }

    /* 2) Fork a process for external commands */
    pid = Fork();
    if (pid == 0) {
        /* Child process */
        Execvp(args[0], args);
    } else {
        /* Parent process */
        Wait(&status);
    }

    return 1;
}

/**
 * shell_read_line - Reads a line from standard input using getline
 * Return: The read string
 */
char *shell_read_line(void)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    read = getline(&line, &len, stdin);
    
    /* Handle EOF (Ctrl+D) */
    if (read == -1) {
        printf("\n");
        free(line);
        orbitshell_shutdown();
        exit(EXIT_SUCCESS);
    }

    return line;
}

/**
 * main - Entry point for the minishell
 * Return: EXIT_SUCCESS
 */
int main(void)
{
    char *line;
    char **args;
    int status = 1;

    /* REPL loop */
    do {
        if (isatty(fileno(stdin))) {
            print_prompt();
        }

        line = shell_read_line();
        args = tokenize_input(line);
        status = execute_process(args);

        /* Free dynamically allocated memory */
        free(line);
        free(args);
    } while (status);

    return EXIT_SUCCESS;
}
