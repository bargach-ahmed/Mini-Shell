#include "orbitshell.h"

/* Global builtins array */
t_builtin g_builtins[] = {
    {"cd", shell_cd},
    {"env", shell_env},
    {"exit", shell_exit},
    {NULL, NULL}
};

typedef struct s_redir
{
    char *input;
    char *output;
    int append;
} t_redir;

static volatile sig_atomic_t g_interactive_mode = 0;
static volatile sig_atomic_t g_foreground_running = 0;
static volatile sig_atomic_t g_skip_next_prompt = 0;

static int is_delim_char(char c);
static int is_operator_char(char c);
static void add_token(char ***tokens, int *bufsize, int *position, char *token);
static int find_builtin(const char *command);
static void init_redirection(t_redir *redir);
static int parse_redirections(char **args, t_redir *redir);
static int apply_redirections(t_redir *redir);
static int execute_builtin_with_redirection(int builtin_index, char **args,
                                            t_redir *redir);
static int has_pipeline(char **args);
static int execute_pipeline(char **args);
static void wait_for_child(pid_t pid);

/**
 * print_prompt - Prints the aesthetic shell prompt
 */
void print_prompt(void)
{
    char cwd[1024];

    if (Getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s%s🪐 OrbitShell 🪐%s %s%s%s%s $> ",
               COLOR_BOLD, COLOR_CYAN, COLOR_RESET,
               COLOR_BOLD, COLOR_MAGENTA, cwd, COLOR_RESET);
    }
}

/**
 * sigint_handler - Handles Ctrl+C in the parent shell
 * @sig: Signal number
 */
void sigint_handler(int sig)
{
    (void)sig;
    printf("\n");
    if (g_interactive_mode && !g_foreground_running) {
        print_prompt();
        g_skip_next_prompt = 1;
    }
    fflush(stdout);
}

/**
 * is_delim_char - Checks whether a character separates shell tokens
 * @c: Character to check
 * Return: 1 if delimiter, 0 otherwise
 */
static int is_delim_char(char c)
{
    return (strchr(TOK_DELIM, c) != NULL);
}

/**
 * is_operator_char - Checks whether a character is a shell operator
 * @c: Character to check
 * Return: 1 if operator, 0 otherwise
 */
static int is_operator_char(char c)
{
    return (c == '<' || c == '>' || c == '|');
}

/**
 * add_token - Appends a token to the token array, growing when needed
 * @tokens: Address of token array
 * @bufsize: Address of current token capacity
 * @position: Address of current insertion position
 * @token: Token to append
 */
static void add_token(char ***tokens, int *bufsize, int *position, char *token)
{
    char **new_tokens;

    if (*position >= *bufsize) {
        *bufsize += TOK_BUFSIZE;
        new_tokens = Realloc(*tokens, *bufsize * sizeof(char *));
        *tokens = new_tokens;
    }
    (*tokens)[*position] = token;
    (*position)++;
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
    char *cursor;
    char *start;
    char *write_pos;
    char quote;
    char operator_char;
    int is_append;

    cursor = line;
    while (*cursor != '\0') {
        while (*cursor != '\0' && is_delim_char(*cursor)) {
            *cursor = '\0';
            cursor++;
        }
        if (*cursor == '\0')
            break;
        if (is_operator_char(*cursor)) {
            if (*cursor == '>' && *(cursor + 1) == '>') {
                add_token(&tokens, &bufsize, &position, ">>");
                cursor += 2;
            } else if (*cursor == '>') {
                add_token(&tokens, &bufsize, &position, ">");
                cursor++;
            } else if (*cursor == '<') {
                add_token(&tokens, &bufsize, &position, "<");
                cursor++;
            } else {
                add_token(&tokens, &bufsize, &position, "|");
                cursor++;
            }
            continue;
        }
        start = cursor;
        write_pos = cursor;
        quote = '\0';
        while (*cursor != '\0') {
            if (quote != '\0') {
                if (*cursor == quote) {
                    quote = '\0';
                    cursor++;
                    continue;
                }
                *write_pos = *cursor;
                write_pos++;
                cursor++;
                continue;
            }
            if (*cursor == '\'' || *cursor == '"') {
                quote = *cursor;
                cursor++;
                continue;
            }
            if (is_delim_char(*cursor) || is_operator_char(*cursor))
                break;
            *write_pos = *cursor;
            write_pos++;
            cursor++;
        }
        operator_char = *cursor;
        is_append = (operator_char == '>' && *(cursor + 1) == '>');
        *write_pos = '\0';
        add_token(&tokens, &bufsize, &position, start);
        if (operator_char == '\0')
            break;
        if (is_delim_char(operator_char)) {
            cursor++;
            continue;
        }
        if (is_append) {
            add_token(&tokens, &bufsize, &position, ">>");
            cursor += 2;
        } else if (operator_char == '>') {
            add_token(&tokens, &bufsize, &position, ">");
            cursor++;
        } else if (operator_char == '<') {
            add_token(&tokens, &bufsize, &position, "<");
            cursor++;
        } else {
            add_token(&tokens, &bufsize, &position, "|");
            cursor++;
        }
    }
    add_token(&tokens, &bufsize, &position, NULL);
    return tokens;
}

/**
 * find_builtin - Finds a built-in command by name
 * @command: Command name
 * Return: Built-in index, or -1 if not found
 */
static int find_builtin(const char *command)
{
    int i;

    if (command == NULL)
        return -1;
    i = 0;
    while (g_builtins[i].builtin_name != NULL) {
        if (strcmp(command, g_builtins[i].builtin_name) == 0)
            return i;
        i++;
    }
    return -1;
}

/**
 * init_redirection - Resets redirection state
 * @redir: Redirection structure
 */
static void init_redirection(t_redir *redir)
{
    redir->input = NULL;
    redir->output = NULL;
    redir->append = 0;
}

/**
 * parse_redirections - Removes redirection tokens from args
 * @args: Command arguments
 * @redir: Redirection structure to fill
 * Return: 0 on success, -1 on syntax error
 */
static int parse_redirections(char **args, t_redir *redir)
{
    int read_pos;
    int write_pos;

    init_redirection(redir);
    read_pos = 0;
    write_pos = 0;
    while (args[read_pos] != NULL) {
        if (strcmp(args[read_pos], "<") == 0
            || strcmp(args[read_pos], ">") == 0
            || strcmp(args[read_pos], ">>") == 0) {
            if (args[read_pos + 1] == NULL
                || strcmp(args[read_pos + 1], "<") == 0
                || strcmp(args[read_pos + 1], ">") == 0
                || strcmp(args[read_pos + 1], ">>") == 0
                || strcmp(args[read_pos + 1], "|") == 0) {
                fprintf(stderr, COLOR_RED
                        "minishell: syntax error near redirection\n"
                        COLOR_RESET);
                return -1;
            }
            if (strcmp(args[read_pos], "<") == 0) {
                redir->input = args[read_pos + 1];
            } else {
                redir->output = args[read_pos + 1];
                redir->append = (strcmp(args[read_pos], ">>") == 0);
            }
            read_pos += 2;
        } else {
            args[write_pos] = args[read_pos];
            write_pos++;
            read_pos++;
        }
    }
    args[write_pos] = NULL;
    return 0;
}

/**
 * apply_redirections - Applies parsed input/output redirection
 * @redir: Redirection state
 * Return: 0 on success, -1 on failure
 */
static int apply_redirections(t_redir *redir)
{
    int fd;
    int flags;

    if (redir->input != NULL) {
        fd = open(redir->input, O_RDONLY);
        if (fd == -1) {
            perror(COLOR_RED "minishell input redirection" COLOR_RESET);
            return -1;
        }
        if (dup2(fd, STDIN_FILENO) == -1) {
            perror(COLOR_RED "minishell dup2" COLOR_RESET);
            close(fd);
            return -1;
        }
        close(fd);
    }
    if (redir->output != NULL) {
        flags = O_WRONLY | O_CREAT;
        if (redir->append)
            flags |= O_APPEND;
        else
            flags |= O_TRUNC;
        fd = open(redir->output, flags, 0644);
        if (fd == -1) {
            perror(COLOR_RED "minishell output redirection" COLOR_RESET);
            return -1;
        }
        if (dup2(fd, STDOUT_FILENO) == -1) {
            perror(COLOR_RED "minishell dup2" COLOR_RESET);
            close(fd);
            return -1;
        }
        close(fd);
    }
    return 0;
}

/**
 * execute_builtin_with_redirection - Runs a built-in with temporary fd changes
 * @builtin_index: Index in global built-ins array
 * @args: Built-in arguments
 * @redir: Redirection state
 * Return: Built-in return value
 */
static int execute_builtin_with_redirection(int builtin_index, char **args,
                                            t_redir *redir)
{
    int saved_stdin;
    int saved_stdout;
    int result;

    saved_stdin = -1;
    saved_stdout = -1;
    if (redir->input != NULL) {
        saved_stdin = dup(STDIN_FILENO);
        if (saved_stdin == -1) {
            perror(COLOR_RED "minishell dup" COLOR_RESET);
            return 1;
        }
    }
    if (redir->output != NULL) {
        saved_stdout = dup(STDOUT_FILENO);
        if (saved_stdout == -1) {
            perror(COLOR_RED "minishell dup" COLOR_RESET);
            if (saved_stdin != -1)
                close(saved_stdin);
            return 1;
        }
    }
    if (apply_redirections(redir) == -1) {
        if (saved_stdin != -1) {
            dup2(saved_stdin, STDIN_FILENO);
            close(saved_stdin);
        }
        if (saved_stdout != -1) {
            dup2(saved_stdout, STDOUT_FILENO);
            close(saved_stdout);
        }
        return 1;
    }
    result = (g_builtins[builtin_index].foo)(args);
    fflush(stdout);
    if (saved_stdin != -1) {
        dup2(saved_stdin, STDIN_FILENO);
        close(saved_stdin);
    }
    if (saved_stdout != -1) {
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
    }
    return result;
}

/**
 * has_pipeline - Checks if args contain a pipeline operator
 * @args: Command arguments
 * Return: 1 if a pipe exists, 0 otherwise
 */
static int has_pipeline(char **args)
{
    int i;

    i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], "|") == 0)
            return 1;
        i++;
    }
    return 0;
}

/**
 * wait_for_child - Waits for a specific child, retrying interrupted waits
 * @pid: Child PID
 */
static void wait_for_child(pid_t pid)
{
    int status;
    pid_t wpid;

    while (1) {
        wpid = waitpid(pid, &status, WUNTRACED);
        if (wpid == -1) {
            if (errno == EINTR)
                continue;
            perror(COLOR_RED "minishell waitpid" COLOR_RESET);
            return;
        }
        if (WIFEXITED(status) || WIFSIGNALED(status))
            return;
    }
}

/**
 * execute_pipeline - Executes commands joined by pipe operators
 * @args: Command arguments
 * Return: 1 to continue execution
 */
static int execute_pipeline(char **args)
{
    char ***commands;
    pid_t *pids;
    int command_count;
    int i;
    int prev_read;
    int pipefd[2];
    int builtin_index;
    t_redir redir;

    command_count = 1;
    i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], "|") == 0)
            command_count++;
        i++;
    }
    commands = Malloc(command_count * sizeof(char **));
    pids = Malloc(command_count * sizeof(pid_t));
    command_count = 0;
    commands[command_count] = args;
    command_count++;
    i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], "|") == 0) {
            if (i == 0 || args[i + 1] == NULL || strcmp(args[i + 1], "|") == 0) {
                fprintf(stderr, COLOR_RED
                        "minishell: syntax error near pipe\n" COLOR_RESET);
                free(commands);
                free(pids);
                return 1;
            }
            args[i] = NULL;
            commands[command_count] = &args[i + 1];
            command_count++;
        }
        i++;
    }
    prev_read = -1;
    g_foreground_running = 1;
    for (i = 0; i < command_count; i++) {
        pipefd[0] = -1;
        pipefd[1] = -1;
        if (i < command_count - 1 && pipe(pipefd) == -1) {
            perror(COLOR_RED "minishell pipe" COLOR_RESET);
            break;
        }
        pids[i] = Fork();
        if (pids[i] == 0) {
            signal(SIGINT, SIG_DFL);
            if (prev_read != -1) {
                if (dup2(prev_read, STDIN_FILENO) == -1) {
                    perror(COLOR_RED "minishell dup2" COLOR_RESET);
                    exit(EXIT_FAILURE);
                }
            }
            if (i < command_count - 1) {
                if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
                    perror(COLOR_RED "minishell dup2" COLOR_RESET);
                    exit(EXIT_FAILURE);
                }
            }
            if (prev_read != -1)
                close(prev_read);
            if (pipefd[0] != -1)
                close(pipefd[0]);
            if (pipefd[1] != -1)
                close(pipefd[1]);
            if (parse_redirections(commands[i], &redir) == -1)
                exit(EXIT_FAILURE);
            if (commands[i][0] == NULL)
                exit(EXIT_SUCCESS);
            if (apply_redirections(&redir) == -1)
                exit(EXIT_FAILURE);
            builtin_index = find_builtin(commands[i][0]);
            if (builtin_index != -1) {
                (g_builtins[builtin_index].foo)(commands[i]);
                exit(EXIT_SUCCESS);
            }
            Execvp(commands[i][0], commands[i]);
        }
        if (prev_read != -1)
            close(prev_read);
        if (pipefd[1] != -1)
            close(pipefd[1]);
        prev_read = pipefd[0];
    }
    if (prev_read != -1)
        close(prev_read);
    while (i < command_count) {
        pids[i] = -1;
        i++;
    }
    for (i = 0; i < command_count; i++) {
        if (pids[i] > 0)
            wait_for_child(pids[i]);
    }
    g_foreground_running = 0;
    free(commands);
    free(pids);
    return 1;
}

/**
 * execute_process - Forks a child process and executes the command
 * @args: Array of arguments where args[0] is the command
 * Return: 1 to continue execution, 0 to terminate
 */
int execute_process(char **args)
{
    pid_t pid;
    int status;
    int builtin_index;
    t_redir redir;

    if (args[0] == NULL) {
        /* An empty command was entered */
        return 1;
    }

    if (has_pipeline(args))
        return execute_pipeline(args);

    if (parse_redirections(args, &redir) == -1)
        return 1;
    if (args[0] == NULL)
        return 1;

    /* 1) Check for built-in commands */
    builtin_index = find_builtin(args[0]);
    if (builtin_index != -1) {
        return execute_builtin_with_redirection(builtin_index, args, &redir);
    }

    /* 2) Fork a process for external commands */
    pid = Fork();
    if (pid == 0) {
        /* Child process */
        signal(SIGINT, SIG_DFL);
        if (apply_redirections(&redir) == -1)
            exit(EXIT_FAILURE);
        Execvp(args[0], args);
    } else {
        /* Parent process */
        g_foreground_running = 1;
        Wait(&status);
        g_foreground_running = 0;
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
    
    if (read != -1) {
        g_skip_next_prompt = 0;
        return line;
    }

    if (errno == EINTR) {
        clearerr(stdin);
        free(line);
        line = Malloc(1);
        line[0] = '\0';
        return line;
    }

    /* Handle EOF (Ctrl+D) */
    if (feof(stdin)) {
        printf("\n");
        free(line);
        orbitshell_shutdown();
        exit(EXIT_SUCCESS);
    }
    perror(COLOR_RED "minishell getline" COLOR_RESET);
    free(line);
    exit(EXIT_FAILURE);
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

    g_interactive_mode = isatty(fileno(stdin));
    if (signal(SIGINT, sigint_handler) == SIG_ERR)
        perror(COLOR_RED "minishell signal" COLOR_RESET);

    /* Print the banner if in interactive mode */
    if (g_interactive_mode) {
        print_banner();
    }

    /* REPL loop */
    do {
        if (g_interactive_mode) {
            if (g_skip_next_prompt) {
                g_skip_next_prompt = 0;
            } else {
                print_prompt();
            }
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
