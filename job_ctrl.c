#include "job_ctrl.h"

int check_bg_fg(char** args)
{
    int i;
    if(args[0] == NULL)
        return 0;
    for(i = 0; args[i] != NULL; i++)
    {
        // 到命令末尾
    }
    if(strcmp(args[i - 1], "&") == 0)
    {
        args[i - 1] = NULL;
        return BACKGROUND;
    }
    else
        return FOREGROUND;
}

int shell_bg(char** args)
{

}