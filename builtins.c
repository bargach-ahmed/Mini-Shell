#include "orbitshell.h"

extern char **environ;

/**
 * build_home_path - Builds a HOME-relative path for ~/...
 * @home: HOME directory
 * @arg: Original cd argument
 * Return: Allocated path, or NULL if no allocation is needed
 */
static char *build_home_path(const char *home, const char *arg)
{
    char *path;
    size_t len;

    if (arg == NULL || strcmp(arg, "~") == 0)
        return NULL;
    if (strncmp(arg, "~/", 2) != 0)
        return NULL;
    len = strlen(home) + strlen(arg + 1) + 1;
    path = Malloc(len);
    strcpy(path, home);
    strcat(path, arg + 1);
    return path;
}

/**
 * update_pwd_env - Updates OLDPWD and PWD after a successful chdir
 * @old_pwd: Previous working directory
 * Return: 0 on success, -1 if PWD cannot be resolved
 */
static int update_pwd_env(const char *old_pwd)
{
    char new_pwd[PATH_MAX];

    if (Getcwd(new_pwd, sizeof(new_pwd)) == NULL)
        return -1;
    if (old_pwd != NULL && setenv("OLDPWD", old_pwd, 1) == -1)
        perror(COLOR_RED "minishell setenv OLDPWD" COLOR_RESET);
    if (setenv("PWD", new_pwd, 1) == -1)
        perror(COLOR_RED "minishell setenv PWD" COLOR_RESET);
    return 0;
}

/**
 * shell_cd - Change directory
 * @args: Arguments array
 * Return: 1 to continue execution
 */
int shell_cd(char **args)
{
    char old_pwd[PATH_MAX];
    char new_pwd[PATH_MAX];
    const char *target;
    const char *home;
    char *allocated_target;
    int print_new_pwd;

    target = args[1];
    allocated_target = NULL;
    print_new_pwd = 0;
    if (Getcwd(old_pwd, sizeof(old_pwd)) == NULL) {
        if (getenv("PWD") != NULL)
            strncpy(old_pwd, getenv("PWD"), sizeof(old_pwd) - 1);
        else
            old_pwd[0] = '\0';
        old_pwd[sizeof(old_pwd) - 1] = '\0';
    }
    if (target == NULL || strcmp(target, "~") == 0
        || strncmp(target, "~/", 2) == 0) {
        home = getenv("HOME");
        if (home == NULL) {
            fprintf(stderr, COLOR_RED "minishell: cd: HOME not set\n"
                    COLOR_RESET);
            return 1;
        }
        allocated_target = build_home_path(home, target);
        if (allocated_target != NULL)
            target = allocated_target;
        else
            target = home;
    } else if (strcmp(target, "-") == 0) {
        target = getenv("OLDPWD");
        if (target == NULL) {
            fprintf(stderr, COLOR_RED "minishell: cd: OLDPWD not set\n"
                    COLOR_RESET);
            return 1;
        }
        print_new_pwd = 1;
    }
    if (chdir(target) != 0) {
        perror(COLOR_RED "minishell cd" COLOR_RESET);
        free(allocated_target);
        return 1;
    }
    if (update_pwd_env(old_pwd[0] != '\0' ? old_pwd : NULL) == 0
        && print_new_pwd) {
        if (Getcwd(new_pwd, sizeof(new_pwd)) != NULL)
            printf("%s\n", new_pwd);
    }
    free(allocated_target);
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
