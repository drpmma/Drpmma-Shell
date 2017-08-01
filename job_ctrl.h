#ifndef _JOB_CTRL_H
#define _JOB_CTRL_H

#include <stdlib.h>

#define BACKGROUND 1
#define FOREGROUND 0

int check_bg_fg(char** args);
int shell_bg(char** args);

#endif