#include "orbitshell.h"

/**
 * Malloc - Wrapper for malloc with error handling
 * @size: Size to allocate
 * Return: Pointer to allocated memory
 */
void *Malloc(size_t size)
{
    void *ptr;

    if (size == 0)
        return NULL;
    ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, COLOR_RED "minishell: Malloc allocation failed\n" COLOR_RESET);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

/**
 * Realloc - Wrapper for realloc with error handling
 * @ptr: Pointer to memory block
 * @size: New size
 * Return: Pointer to reallocated memory
 */
void *Realloc(void *ptr, size_t size)
{
    void *new_ptr;

    new_ptr = realloc(ptr, size);
    if (!new_ptr && size != 0) {
        fprintf(stderr, COLOR_RED "minishell: Realloc failed\n" COLOR_RESET);
        exit(EXIT_FAILURE);
    }
    return new_ptr;
}

/**
 * Fork - Wrapper for fork with error handling
 * Return: PID of child, exits on failure
 */
pid_t Fork(void)
{
    pid_t pid;

    pid = fork();
    if (pid < 0) {
        fprintf(stderr, COLOR_RED "minishell: Fork failed\n" COLOR_RESET);
        exit(EXIT_FAILURE);
    }
    return pid;
}

/**
 * Wait - Wrapper for waitpid with error handling
 * @status: Pointer to status integer
 */
void Wait(int *status)
{
    pid_t wpid;

    do {
        wpid = waitpid(-1, status, WUNTRACED);
        if (wpid == -1) {
            fprintf(stderr, COLOR_RED "minishell: Waitpid failed\n" COLOR_RESET);
            break;
        }
    } while (!WIFEXITED(*status) && !WIFSIGNALED(*status));
}

/**
 * Execvp - Wrapper for execvp with error handling
 * @file: File to execute
 * @argv: Arguments array
 */
void Execvp(const char *file, char *const argv[])
{
    if (!file || !argv) {
        fprintf(stderr, COLOR_RED "minishell: Execvp invalid arguments\n" COLOR_RESET);
        exit(EXIT_FAILURE);
    }
    if (execvp(file, argv) == -1) {
        perror(COLOR_RED "minishell" COLOR_RESET);
        exit(EXIT_FAILURE);
    }
}

/**
 * Getcwd - Wrapper for getcwd
 * @buf: Buffer to store path
 * @size: Size of buffer
 * Return: Pointer to buf
 */
char *Getcwd(char *buf, size_t size)
{
    char *result;

    result = getcwd(buf, size);
    if (!result)
        perror(COLOR_RED "Getcwd failed" COLOR_RESET);
    return result;
}

/**
 * print_banner - Prints the OrbitShell ASCII art banner
 */
void print_banner(void)
{
    printf(COLOR_GREEN " ██████╗ ██████╗ ██████╗ ██╗████████╗    ███████╗██╗  ██╗███████╗██╗     ██╗     \n" COLOR_RESET);
    printf(COLOR_GREEN "██╔═══██╗██╔══██╗██╔══██╗██║╚══██╔══╝    ██╔════╝██║  ██║██╔════╝██║     ██║     \n" COLOR_RESET);
    printf(COLOR_GREEN "██║   ██║██████╔╝██████╔╝██║   ██║       ███████╗███████║█████╗  ██║     ██║     \n" COLOR_RESET);
    printf(COLOR_GREEN "██║   ██║██╔══██╗██╔══██╗██║   ██║       ╚════██║██╔══██║██╔══╝  ██║     ██║     \n" COLOR_RESET);
    printf(COLOR_GREEN "╚██████╔╝██║  ██║██████╔╝██║   ██║       ███████║██║  ██║███████╗███████╗███████╗\n" COLOR_RESET);
    printf(COLOR_GREEN " ╚═════╝ ╚═╝  ╚═╝╚═════╝ ╚═╝   ╚═╝       ╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝\n" COLOR_RESET);
    printf(COLOR_CYAN "                     v1.0 | A custom minimal Unix shell\n\n" COLOR_RESET);
}
