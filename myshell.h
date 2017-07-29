#define FILE_PATH_LENGTH 100

char* internal_str[] = {
    "cd",
    "umask",
    "time",
    "help",
    "exit"
};
int shell_cd(char** args);
int shell_time(char** args);
int shell_umask(char** args);
int shell_help(char** args);
int shell_exit(char** args);
int (*internal_cmd[]) (char **) = {
    &shell_cd,
    &shell_umask,
    &shell_time,
    &shell_help,
    &shell_exit
};

void main_loop();
char* read_line();
char** split_line(char* line);
int execute(char** args);
int external_cmd(char** args);
