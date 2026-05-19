#ifndef ORBITSHELL_H
#define ORBITSHELL_H

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/* Aesthetic constants */
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_WHITE   "\x1b[37m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BOLD    "\x1b[1m"
#define COLOR_RESET   "\x1b[0m"

#define TOK_DELIM " \t\r\n\a"
#define TOK_BUFSIZE 64

/* Struct for Built-in commands */
typedef struct s_builtin
{
    const char *builtin_name;
    int (*foo)(char **args);
} t_builtin;

/* Built-in prototypes */
int shell_cd(char **args);
int shell_env(char **args);
int shell_exit(char **args);

extern t_builtin g_builtins[];

/* Wrapper prototypes */
void *Malloc(size_t size);
void *Realloc(void *ptr, size_t size);
pid_t Fork(void);
void Wait(int *status);
void Execvp(const char *file, char *const argv[]);
char *Getcwd(char *buf, size_t size);

/* Aesthetic prototypes */
void print_prompt(void);
void orbitshell_shutdown(void);
void print_banner(void);

#endif /* ORBITSHELL_H */
