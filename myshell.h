#ifndef _MYSHELL_H
#define _MYSHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>  
#include <sys/types.h>  
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include "job_ctrl.h"

#define FILE_PATH_LENGTH 100
#define COMMAND_NUMBER 20
#define COMMAND_SIZE 64
#define ARGUMENT_SIZE 32

extern char** environ;

struct command{
    char** args;
    int mode;
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
int parse_redirect(char** args, int* pfd_in, int* pfd_out);
int parse_pipe(struct command* cmd_array, int size);
int builtin_cmd(char** args);
int execute(struct command cmd, int fd_in, int fd_out, int fd_err);

#endif