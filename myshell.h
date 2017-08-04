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
#include <dirent.h> 
#include "job_ctrl.h"

#define FILE_PATH_LENGTH 100
#define PATH_LENGTH 500
#define COMMAND_NUMBER 20
#define COMMAND_SIZE 64
#define ARGUMENT_SIZE 32

#define TEST_B 0
#define TEST_C 1

#define TEST_E 3
#define TEST_F 4

#define TEST_H 6
#define TEST_L 8
#define TEST_P 10
#define TEST_R 11
#define TEST_W 16
#define TEST_X 17

#define TEST_EQ 30
#define TEST_GE 31
#define TEST_GT 32
#define TEST_LE 33
#define TEST_LT 34
#define TEST_NE 35

#define RETURN_CONTINUE 10

#define NONE         "\033[m"  
#define RED          "\033[0;32;31m"
#define LIGHT_RED    "\033[1;31m"
#define GREEN        "\033[0;32;32m"
#define LIGHT_GREEN  "\033[1;32m"
#define BLUE         "\033[0;32;34m"
#define LIGHT_BLUE   "\033[1;34m"
#define DARY_GRAY    "\033[1;30m"
#define CYAN         "\033[0;36m"
#define LIGHT_CYAN   "\033[1;36m"
#define PURPLE       "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN        "\033[0;33m"
#define YELLOW       "\033[1;33m"
#define LIGHT_GRAY   "\033[0;37m"
#define WHITE        "\033[1;37m"

extern char** environ;

struct jobs* job_array;

char* file_path;
char* HOME;
char* PATH;
char* new_PATH;

int shell_cd(char** args);
int shell_umask(char** args);
int shell_time(char** args);
int shell_environ(char** args);
int shell_set(char** args);
int shell_unset(char** args);
int shell_exec(char** args);
int shell_help(char** args);
int shell_exit(char** args);
int shell_test(char** args);
int shell_continue();
int shell_shift(char** args);
int shell_clr();
int test_dir(char* arg);
int test_file(char* arg, int flag);
int test_logic(char** args, int flag);

void main_loop();
void init();
char* read_line(int* pfile_flag);
void parse_var(char** args);
void parse_quote(char** args);
char** split_str(char* line, int size, char* delims);
int parse_redirect(char** args, int* pfd_in, int* pfd_out);
int parse_pipe(struct command* cmd_array, int size);
int check_builtin(struct command cmd);
int builtin_cmd(struct command cmd);
int builtin_env_cmd(struct command cmd);
int execute(struct command cmd, int fd_in, int fd_out, int fd_err);
void clear_buffer(struct command* cmd_array, char* line, char** cmds);
void signals();
void set_arg(int argc, char* argv[]);
void set_env_pid();
void set_env_status();
char* IntToString(int i);

#endif