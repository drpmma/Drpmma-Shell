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

#define NAME_SIZE 64

struct jobs
{
    int id;
    pid_t pid;
    char* name;
    int state;
};

struct command{
    char** args;
    int mode;
};

struct jobs* job_array;

void job_init();
struct jobs* get_new_job();
struct jobs* get_job_byID(int id);
struct jobs* get_job_byPID(pid_t pid);
int check_bg_fg(char** args);
void deal_bg_fg(struct command cmd);
int shell_jobs(char** args);
int shell_kill(char** args);
int shell_fg(struct command cmd);
int shell_bg(struct command cmd);
void handle_child(int sig);
void handle_stop(struct command cmd, pid_t pid);
void change_state(pid_t pid, int state);
void clear_job(pid_t pid);
void clear_job_all();

#endif