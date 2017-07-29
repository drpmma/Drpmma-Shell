#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "myshell.h"

int main(void)
{
    sh_loop();
}

void sh_loop()
{
    char* line;
    char** args;
    int status = 1;
    while(status)
    {
        printf("myshell>");
        line = read_line();
        args = split_line(line);
    }
}

char* read_line()
{
    char *line = NULL;
    ssize_t bufsize = 0;
    getline(&line, &bufsize, stdin);
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

}