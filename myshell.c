#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
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
    
    while(status)
    {
        file_path=(char *)malloc(FILE_PATH_LENGTH);
        getcwd(file_path, FILE_PATH_LENGTH);

        printf("myshell:%s>", file_path);
        line = read_line();
        args = split_line(line);
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

char** split_line(char* line)
{
    int buf_size = 64, pos = 0;
    char* arg;
    char** args = malloc(buf_size * sizeof(char*));

    arg = strtok(line, " \t\n");
    while(arg != NULL)
    {
        args[pos] = arg;
        pos++;

        if(pos >= buf_size)
        {
            buf_size += buf_size;
            args = realloc(args, buf_size * sizeof(char*));
        }

        arg = strtok(NULL, " \t\n");
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
            return (*internal_cmd[i])(args);
        }
    }

    return external_cmd(args);
}

int external_cmd(char** args)
{
    pid_t pid, w_pid;
    int status;
    pid = fork();
    if(pid < 0)
    {
        perror("myshell");
    }
    else if(pid == 0)
    {
        if(execvp(args[0], args) == -1)
        {
            perror("myshell");
        }
        exit(1);
    }
    else
    {
        do {
            w_pid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
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
}

int shell_help(char** args)
{
    char* cmd[] = {"more", "README.md"};
    external_cmd(cmd);
}

int shell_exit(char** args)
{
    return 0;
}