#include "job_ctrl.h"

void job_init()                             // init the job struct array
{
    for(int i = 0; i < JOB_NUMBER; i++)
    {
        job_array[i].id = 0;
        job_array[i].pid = 0;
        job_array[i].name = NULL;
        job_array[i].state = JOB_STATE_INIT;
    }
}

int check_bg_fg(char** args)                // determin foreground or background
{
    int i;
    if(args[0] == NULL)
        return 0;
    for(i = 0; args[i] != NULL; i++)
    {
        // 到命令末尾
    }
    if(strcmp(args[i - 1], "&") == 0)       // if the last argument of command is '&', then it's background
    {
        args[i - 1] = NULL;
        return BACKGROUND;
    }
    else                                    // versus
        return FOREGROUND;
}

void job_ctrl(struct command cmd)           // the three commands of job_ctrl
{
    if(strcmp(cmd.args[0], "fg") == 0)
    {
        shell_fg(cmd);
    }
    else if(strcmp(cmd.args[0], "bg") == 0)
    {
        shell_bg(cmd);
    }
    else if(strcmp(cmd.args[0], "jobs") == 0)
    {
        shell_jobs(cmd.args);
    }
}

int shell_jobs(char** args)                 // print jobs
{
    for(int i = 0; i < JOB_NUMBER; i++)
    {
        if(job_array[i].state == JOB_STATE_RUN)             // according to the difference to print different infos
        {
            printf("[%d]+ 运行中\t", job_array[i].id);
            printf("pid=%d\t", job_array[i].pid);
            printf("state=%d\t", job_array[i].state);
            printf("%s\n", job_array[i].name); 
        }
        if(job_array[i].state == JOB_STATE_STOP)
        {
            printf("[%d]+ 已终止\t", job_array[i].id);
            printf("pid=%d\t", job_array[i].pid);
            printf("state=%d\t", job_array[i].state);
            printf("%s\n", job_array[i].name);
            clear_job(job_array[i].pid);
        }
        if(job_array[i].state == JOB_STATE_PAUSE)
        {
            printf("[%d]+ 已停止\t", job_array[i].id);
            printf("pid=%d\t", job_array[i].pid);
            printf("state=%d\t", job_array[i].state);
            printf("%s\n", job_array[i].name);
        }
    }
    return 0;
}

int shell_kill(char** args)                             // kill process
{
    if(args[1] == NULL)
    {
        printf("kill: 用法: kill %%{id} or kill {pid}\n");
    }
    else if(args[1][0] == '%')                          // kill %id
    {
        int id = atoi(args[1] + 1);
        if(id == 0)
        {
            printf("myshell:参数错误\n");
        }
        else
        {
            struct jobs* temp_job = get_job_byID(id);
            if(temp_job != NULL)
            {
                kill(temp_job->pid, SIGKILL);                   // send SIGKILL signal
                change_state(temp_job->pid, JOB_STATE_STOP);    // set to JOB_STATE_STOP
            }
            else
            {
                printf("myshell:不存在这样的id\n");
            }
        }
    }
    else                                                        // kill pid
    {
        pid_t pid = atoi(args[1]);
        if(pid == 0)
        {
            printf("myshell:参数错误\n");
        }
        else
        {
            struct jobs* temp_job = get_job_byPID(pid);
            if(temp_job != NULL)
            {
                kill(temp_job->pid, SIGKILL);                   // send SIGKILL signal
                change_state(temp_job->pid, JOB_STATE_STOP);    // set to JOB_STATE_STOP
            }
            else
            {
                kill(pid, SIGKILL);                             // send signal even if the process not in the jobs anyway
            }
        }
    }
    return 0;
}

int shell_fg(struct command cmd)                        // set the background process to foreground and run it anyway
{
    int ret;
    if(cmd.args[1] == NULL)
    {
        printf("fg: 用法: fg %%{id} or fg {id}\n");
    }
    else if(cmd.args[1][0] == '%')                              // fg %id
    {
        int id = atoi(cmd.args[1] + 1);
        if(id == 0)
        {
            printf("myshell:参数错误\n");
        }
        else
        {
            struct jobs* temp_job = get_job_byID(id);           // get a job by its id
            if(temp_job != NULL)
            {
                printf("[%d] %d\n", temp_job->id, temp_job->pid);
                cmd.mode = FOREGROUND;                          // set to FOREGROUND
                change_state(temp_job->pid, JOB_STATE_RUN);     // set to run
                kill(temp_job->pid, SIGCONT);                   // send SIGCONT to continue it
                waitpid(temp_job->pid, &ret, WUNTRACED);        // wait the process to stop
                if((WIFSTOPPED(ret)))
                {
                    handle_stop(cmd, temp_job->pid);            // if receive the SIGTSTP, handle it
                }
            }
            else
            {
                printf("myshell:不存在这样的id\n");
            }
        }
    }
    else                                                        // fg pid
    {
        int id = atoi(cmd.args[1]);
        if(id == 0)
        {
            printf("myshell:参数错误\n");
        }
        else
        {
            struct jobs* temp_job = get_job_byID(id);           // get a job by its id
            if(temp_job != NULL)
            {
                printf("[%d] %d\n", temp_job->id, temp_job->pid);
                cmd.mode = FOREGROUND;
                change_state(temp_job->pid, JOB_STATE_RUN);
                kill(temp_job->pid, SIGCONT);
                waitpid(temp_job->pid, &ret, WUNTRACED);        // wait the process to stop
                if((WIFSTOPPED(ret)))
                {
                    handle_stop(cmd, temp_job->pid);            // if receive the SIGTSTP, handle it
                }
            }
            else
            {
                printf("myshell:不存在这样的id\n");
            }
        }
    }
    return 0;
}

int shell_bg(struct command cmd)                        // set the process run in background
{
    if(cmd.args[1] == NULL)
    {
        printf("bg: 用法: bg %%{id} or bg {id}\n");
    }
    else if(cmd.args[1][0] == '%')                      // bg %id
    {
        int id = atoi(cmd.args[1] + 1);
        if(id == 0)
        {
            printf("myshell:参数错误\n");
        }
        else
        {
            struct jobs* temp_job = get_job_byID(id);               // get a job by its id
            if(temp_job != NULL)
            {
                printf("[%d] %d\n", temp_job->id, temp_job->pid);       // print job info
                kill(temp_job->pid, SIGCONT);           // send SIGCONT and not wait it, so the process can run in background
                cmd.mode = BACKGROUND;
                change_state(temp_job->pid, JOB_STATE_RUN);             // change job state
            }
            else
            {
                printf("myshell:不存在这样的id\n");
            }
        }
    }
    else                                                // bg pid
    {   
        int id = atoi(cmd.args[1]);
        if(id == 0)
        {
            printf("myshell:参数错误\n");
        }
        else
        {
            struct jobs* temp_job = get_job_byID(id);       // get a job by its id
            if(temp_job != NULL)
            {
                printf("[%d] %d\n", temp_job->id, temp_job->pid);
                kill(temp_job->pid, SIGCONT);           // send SIGCONT and not wait it, so the process can run in background
                cmd.mode = BACKGROUND;
                change_state(temp_job->pid, JOB_STATE_RUN);     // change job state
            }
            else
            {
                printf("myshell:不存在这样的id\n");
            }
        }
    }
    return 0;
}

struct jobs* get_new_job()                              // when a new job needed, return a new, free job
{
    int max = 0;
    int i;
    for(i = 0; i < JOB_NUMBER; i++)
    {
        if(job_array[i].state == JOB_STATE_INIT)        // find a free job
            break;
    }

    for(int j = 0; j < JOB_NUMBER; j++)
    {
        if(job_array[j].state != JOB_STATE_INIT)        // get a proper id
        {
            if(job_array[j].id > max)
                max = job_array[j].id;
        }
    }
    job_array[i].id = max + 1;                          // and set it
    return job_array + i;
}

void handle_child(int sig)                              // handle
{
    int status;                                         // 
    pid_t pid;
    if((pid = waitpid(-1, &status, WNOHANG)) > 0)       // wait no hang
    {                                                   // so the children process can run in background
        if(WIFEXITED(status))                           // if exit
        {  
            change_state(pid, JOB_STATE_PAUSE);         // set to pause
        }
    }
}

void handle_stop(struct command cmd, pid_t pid)         // when a job is stop, set and print some info
{
    struct jobs* temp_job = get_job_byPID(pid);         // get job by pid
    if(temp_job == NULL)
    {
        temp_job = get_new_job();
        temp_job->pid = pid;
        temp_job->name = malloc(NAME_SIZE);
        memset(temp_job->name, 0, NAME_SIZE);
        for(int i = 0; cmd.args[i] != NULL; i++)        // set name by its commands
        {
            strcat(temp_job->name, cmd.args[i]);
            strcat(temp_job->name, " ");
        }
    }
    cmd.mode = BACKGROUND;
    change_state(pid, JOB_STATE_PAUSE);                 // change state to stop
    printf("\n");
    printf("[%d]+ 已停止\t", temp_job->id);
    printf("%s\n", temp_job->name); 
}

void change_state(pid_t pid, int state)                 // change the job state
{
    for(int i = 0; i < JOB_NUMBER; i++)
    {
        if(job_array[i].pid == pid)
        {
            job_array[i].state = state;
            break;
        }
    }
}

void clear_job(pid_t pid)                               // free the space taken by the job
{
    struct jobs* temp_job = get_job_byPID(pid);
    temp_job->id = 0;
    temp_job->pid = 0;
    free(temp_job->name);
    temp_job->name = NULL;
    temp_job->state = JOB_STATE_INIT;
}

void clear_job_all()                                    // clear job_array
{
    for(int i = 0; i < JOB_NUMBER; i++)
    {
        free(job_array[i].name);
    }
    free(job_array);
}

struct jobs* get_job_byID(int id)                       // get jobs by id
{
    for(int i = 0; i < JOB_NUMBER; i++)
    {
        if(job_array[i].id == id)
            return job_array + i;
    }
    return NULL;
}

struct jobs* get_job_byPID(pid_t pid)                   // get jobs by pid
{
    for(int i = 0; i < JOB_NUMBER; i++)
    {
        if(job_array[i].pid == pid)
            return job_array + i;
    }
    return NULL;
}