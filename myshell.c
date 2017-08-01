#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>  
#include <sys/types.h>  
#include <time.h>
#include "myshell.h"


int main(void)
{
    main_loop();                          // 主循环
}

void main_loop()
{
    char* line;
    char** args;
    int status = 1;
    char *file_path;

    file_path=(char *)malloc(FILE_PATH_LENGTH);
    getcwd(file_path, FILE_PATH_LENGTH);
    file_path=strcat(file_path, "/myshell");
    setenv("MYSHELL", file_path, 0);

    // char ** a = malloc(100);
    // a[0] = "ls";
    // char ** b = malloc(100);
    // b[0] = "grep";
    // b[1] = "m";
    // parse_pipe(a, b);
    while(status)
    {
        getcwd(file_path, FILE_PATH_LENGTH);
        printf("myshell:%s>", file_path);

        line = read_line();
        args = split_str(line, ARGUMENT_SIZE, " \t\n");
        status = execute(args);
    }
}

char* read_line()
{
    char *line = NULL;
    ssize_t bufsize = 0;
    getline(&line, &bufsize, stdin);            // 通过getline函数可以方便的读入一行
    return line;
}


int parse_pipe(char** cmd1, char** cmd2)
{
    int fd[2];

    pipe(fd);

    if (!fork()) {

        // close STD_OUT
        close(1);
        // make STD_OUT same as fd[1]
        dup(fd[1]);
        // we don't need this
        close(fd[0]);
        execute(cmd1);

    } else {

        // close STD_IN
        close(0);
        // make STD_IN same as fd[0]
        dup(fd[0]);
        // we don't need this
        close(fd[1]);
        execute(cmd2);
    }
}

char** split_str(char* line, int size, char* delims)
{
    int buf_size = size, pos = 0;
    char* arg;
    char** args = malloc(size * sizeof(char*));

    arg = strtok(line, delims);
    while(arg != NULL)
    {
        args[pos] = arg;
        pos++;

        if(pos >= buf_size)
        {
            buf_size += size;
            args = realloc(args, buf_size * sizeof(char*));
        }

        arg = strtok(NULL, delims);
    }
    args[pos] = NULL;
    return args;
}

int execute(char** args)
{
    if(args[0] == NULL)
        return 1;

    for (int i = 0; i < sizeof(internal_str) / sizeof(char*); i++) 
    {
        if (strcmp(args[0], internal_str[i]) == 0) 
        {
            return !((*internal_cmd[i])(args));
        }
    }

    pid_t pid, w_pid;
    int ret, status;

    pid = fork();
    if(pid < 0)
    {
        perror("myshell");
    }
    else if(pid == 0)
    {
        // signal(SIGINT, SIG_DFL);
        // signal(SIGTSTP, SIG_DFL);
        // signal(SIGCONT, SIG_DFL);


        if(execvp(args[0], args) == -1)
        {
            perror("myshell");
        }
        exit(1);
    }
    else
    {
        // signal(SIGINT, SIG_IGN);
        // signal(SIGTSTP, SIG_IGN);
        // signal(SIGCONT, SIG_DFL);
        // do {
            w_pid = waitpid(pid, &ret, WUNTRACED);
        // } while (!WIFEXITED(ret) && !WIFSIGNALED(ret));

        status = WEXITSTATUS(ret);
    }
    return 1;
}

int shell_cd(char** args)
{
    if(args[1] == NULL)
    {
        chdir(getenv("HOME"));
    }
    else
    {
        if(chdir(args[1]) != 0)
            perror("myshell");
    }
    return 0;
}

int shell_time(char** args)
{
    time_t now;
    struct tm* time_now;
    time(&now);
    time_now = localtime(&now);
    printf("%s", asctime(time_now));
    return 0;
}

int shell_umask(char** args)
{
    mode_t new_umask = 0666, old_umask;
    old_umask = umask(new_umask);
    if(args[1] == NULL)
    {
        printf("%04o\n", old_umask);
        umask(old_umask);
    }
    else
    {
        new_umask = strtoul(args[1], 0, 8);
        printf("%04o\n", new_umask);
        umask(new_umask);
    }
    return 0;
}

int shell_environ(char** args)
{
    int i = 0;
    for(i = 0; environ[i] != NULL; i++)
    {
        printf("%s\n",environ[i]);
    }
    return 0;
}

int shell_set(char** args)
{
    if(args[1] == NULL)
    {
        shell_environ(args);
    }
    else if(args[2] == NULL)
    {
        setenv(args[1], "NULL", 0);
    }
    else
    {
        setenv(args[1], args[2], 0);
    }
    return 0;
}

int shell_unset(char** args)
{
    if(args[1] != NULL)
    {
        unsetenv(args[1]);
    }
    return 0;
}

int shell_exec(char** args)
{
    execvp(args[1], args + 1);
}

int shell_help(char** args)
{
    char* cmd[] = {"more", "README.md"};
    execvp(cmd[0], cmd);
}

int shell_exit(char** args)
{
    return 1;
}