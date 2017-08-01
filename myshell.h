#define FILE_PATH_LENGTH 100
#define COMMAND_SIZE 64
#define ARGUMENT_SIZE 32

extern char** environ;

struct command{
    char **args;
    int num;
};

char* internal_str[] = {
    "cd",
    "umask",
    "time",
    "environ",
    "set",
    "unset",
    "exec",
    "help",
    "exit"
};
int shell_cd(char** args);
int shell_umask(char** args);
int shell_time(char** args);
int shell_environ(char** args);
int shell_set(char** args);
int shell_unset(char** args);
int shell_exec(char** args);
int shell_help(char** args);
int shell_exit(char** args);
int (*internal_cmd[]) (char **) = {
    &shell_cd,
    &shell_umask,
    &shell_time,
    &shell_environ,
    &shell_set,
    &shell_unset,
    &shell_exec,
    &shell_help,
    &shell_exit
};

void main_loop();
char* read_line();
char** split_str(char* line, int size, char* delims);
int parse_redirect();
int parse_pipe(char** cmd1, char** cmd2);
int execute(char** args);
