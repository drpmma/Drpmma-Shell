#ifndef _JOB_CTRL_H
#define _JOB_CTRL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>  
#include <sys/wait.h> 

#define BACKGROUND 1
#define FOREGROUND 0
#define JOB_NUMBER 20

#define JOB_STATE_INIT 0
#define JOB_STATE_RUN 1
#define JOB_STATE_PAUSE 2
#define JOB_STATE_STOP 3

struct jobs
{
    int id;
    pid_t pid;
    char* name;
    int state;
};

struct jobs* job_array;

void job_init();
struct jobs* get_new_job();
struct jobs* get_job_byID(int id);
struct jobs* get_job_byPID(pid_t pid);
int check_bg_fg(char** args);
int shell_jobs(char** args);
int shell_kill(char** args);
int shell_fg(char** args);
int shell_bg(char** args);
void handle_child(int sig);
void handle_stop(int sig);
void change_state(pid_t pid, int state);
void clear_job(pid_t pid);

#endif