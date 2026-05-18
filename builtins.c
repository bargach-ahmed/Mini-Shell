#include "orbitshell.h"

extern char **environ;

/**
 * shell_cd - Change directory
 * @args: Arguments array
 * Return: 1 to continue execution
 */
int shell_cd(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, COLOR_RED "minishell: expected argument to \"cd\"\n" COLOR_RESET);
    } else {
        if (chdir(args[1]) != 0) {
            perror(COLOR_RED "minishell cd" COLOR_RESET);
        }
    }
    return 1;
}

/**
 * shell_env - Print environment variables
 * @args: Arguments array (unused)
 * Return: 1 to continue execution
 */
int shell_env(char **args)
{
    int i;
    (void)args;

    if (!environ)
        return 1;
        
    for (i = 0; environ[i] != NULL; i++) {
        printf("%s\n", environ[i]);
    }
    return 1;
}

/**
 * orbitshell_shutdown - Custom animation for shell exit
 */
void orbitshell_shutdown(void)
{
    int i;
    const char *launch_seq[] = {
        "🚀 [>---------]",
        "🚀 [=>--------]",
        "🚀 [==>-------]",
        "🚀 [===>------]",
        "🚀 [====>-----]",
        "🚀 [=====>----]",
        "🚀 [======>---]",
        "🚀 [=======>--]",
        "🚀 [========>-]",
        "🚀 [=========>]",
        "🌌 ORBIT REACHED 🌌"
    };
    int frames = sizeof(launch_seq) / sizeof(launch_seq[0]);

    printf(COLOR_CYAN "\nBreaking Orbit...\n" COLOR_RESET);

    for (i = 0; i < frames; i++) {
        printf("\r" COLOR_MAGENTA "%s" COLOR_RESET, launch_seq[i]);
        fflush(stdout);
        usleep(300000); /* 300ms per frame */
    }
    printf(COLOR_CYAN "\n\nGoodbye!\n" COLOR_RESET);
}

/**
 * shell_exit - Exit the shell
 * @args: Arguments array (unused)
 * Return: 0 to terminate execution loop
 */
int shell_exit(char **args)
{
    (void)args;
    orbitshell_shutdown();
    return 0;
}
