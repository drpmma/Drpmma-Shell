#include "myshell.h"

int main(void)
{
    main_loop();                          // 主循环
}

void main_loop()
{
    char* line;
    char** cmds;
    struct command* cmd_array;
    char** args;
    int status = 1;
    int i;
    char *file_path;

    file_path=(char *)malloc(FILE_PATH_LENGTH);
    getcwd(file_path, FILE_PATH_LENGTH);
    file_path=strcat(file_path, "/myshell");
    setenv("MYSHELL", file_path, 0);

    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGCONT, SIG_DFL);

    cmd_array = malloc(COMMAND_NUMBER * sizeof(struct command));
    memset(cmd_array, 0, COMMAND_NUMBER);
    while(status)
    {
        getcwd(file_path, FILE_PATH_LENGTH);
        printf("myshell:%s>", file_path);

        line = read_line();
        cmds = split_str(line, COMMAND_SIZE, "|");
        for(i = 0; cmds[i] != NULL; i++)
        {
            args = split_str(cmds[i], ARGUMENT_SIZE, " \t\n");
            cmd_array[i].mode = check_bg_fg(args);
            cmd_array[i].args = args;
        }
        if(i == 1)
            status = execute(cmd_array[0], 0, 1 ,2);
        else
        {
            parse_pipe(cmd_array, i);
        }
    }
}

char* read_line()
{
    char *line = NULL;
    ssize_t bufsize = 0;
    getline(&line, &bufsize, stdin);            // 通过getline函数可以方便的读入一行
    return line;
}

int parse_redirect(char** args, int* pfd_in, int* pfd_out)
{
    int is_append = 0;
    int flags;                    /* Flags for open() */ 
    int mode = S_IRUSR | S_IWUSR; /* Mode for open() */
    for (int i = 0; args[i] != NULL; i++) 
    {
        if ((strcmp(args[i], ">") == 0) || (strcmp(args[i], "<") == 0)) {
            /* Check syntax error -- redirection where innappropriate */
            // if (numpipes > 1) {
            //     if (!strcmp(argv[i], ">") && pipeno + 1 < numpipes)
            //         return 1;
            //     if (!strcmp(argv[i], "<") && pipeno != 0)
            //         return 1;
            // }
            /* Check syntax error -- no file given */
            // if (i >= argc)
            //     return 1;

            flags = (strcmp(args[i], ">") == 0) ? O_WRONLY | O_CREAT | O_TRUNC : O_RDONLY;
            int fd_tmp = open(args[i + 1], flags, mode);
            if (fd_tmp < 0) {
                perror("run_shell: start_prog");
                if (errno != ENOENT) /* Consider ENOENT (file not found) a syntax error */
                    exit(EXIT_FAILURE);
                return 1;
            }
            *pfd_in = (strcmp(args[i], "<") == 0) ? fd_tmp : *pfd_in;
            *pfd_out = (strcmp(args[i], ">") == 0) ? fd_tmp : *pfd_out;
            args[i] = NULL;
        }
        else if(strcmp(args[i], ">>") == 0)
        {
            flags = O_WRONLY | O_CREAT | O_APPEND;
            int fd_tmp = open(args[i + 1], flags, mode);
            if (fd_tmp < 0) {
                perror("run_shell: start_prog");
                if (errno != ENOENT) /* Consider ENOENT (file not found) a syntax error */
                    exit(EXIT_FAILURE);
                return 1;
            }
            *pfd_out = fd_tmp;
            args[i] = NULL;
        }
    }
}

int parse_pipe(struct command* cmd_array, int size)
{
    int i;
    int p1[2];   /* Pipe for parent */
    int p2[2];   /* Pipe for child */

    if (pipe(p1))
        perror("myshell: split_line");

    /* Get started from stdin */
    // if (start_prog(0, nchunks, commands[0].argv[0], commands[0].argc, commands[0].argv, 0, p1[1])) {
	// free_commands(commands, nchunks);
	// free(commands);
    //     return 1;
    // }
    execute(cmd_array[0], 0, p1[1], 2);
    close(p1[1]);

    for (i = 1; i < size - 1; i++) {
        /* Read from parent's pipe, write to child's */
        /* Close the pipe we read from, and the one we write to */
        if (i % 2) {
            if (pipe(p2))
                perror("run_shell: handle_line");
        //     if (start_prog(i, nchunks, commands[i].argv[0], commands[i].argc, commands[i].argv, p1[0], p2[1])) {
		// free_commands(commands, nchunks);
		// free(commands);
        //         return 1;
	    // }
            execute(cmd_array[i], p1[0], p2[1], 2);
            close(p1[0]);
            close(p2[1]);
        } else {
            if (pipe(p1))
                perror("run_shell: handle_line");
        //     if (start_prog(i, nchunks, commands[i].argv[0], commands[i].argc, commands[i].argv, p2[0], p1[1])) {
		// free_commands(commands, nchunks);
		// free(commands);
        //         return 1;
	    // }
            execute(cmd_array[i], p2[0], p1[1], 2);
            close(p2[0]);
            close(p1[1]);
        }
    }

    /* Finish on stdout */
    if (i % 2) {
    //     if (start_prog(i, nchunks, commands[i].argv[0], commands[i].argc, commands[i].argv, p1[0], 1)) {
	//     free_commands(commands, nchunks);
	//     free(commands);
    //         return 1;
	// }
        execute(cmd_array[i], p1[0], 1, 2);

        close(p1[0]);
    } else {
        // if (start_prog(i, nchunks, commands[i].argv[0], commands[i].argc, commands[i].argv, p2[0], 1)) {
	    // free_commands(commands, nchunks);
	    // free(commands);
            // return 1;
	// }
        execute(cmd_array[i], p2[0], 1, 2);
        close(p2[0]);
    }

    /* Free every command struct in commands */
    // free_commands(commands, nchunks);
    // free(commands);

    return 0;
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

int execute(struct command cmd, int fd_in, int fd_out, int fd_err)
{
    if(cmd.args[0] == NULL)
        return 1;

    parse_redirect(cmd.args, &fd_in, &fd_out);

    for (int i = 0; i < sizeof(internal_str) / sizeof(char*); i++) 
    {
        if (strcmp(cmd.args[0], internal_str[i]) == 0) 
        {
            return !((*internal_cmd[i])(cmd.args));
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
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGCONT, SIG_DFL);

        if(fd_out==STDIN_FILENO)
        { /* Move stdout out of the way of stdin */
            fd_out=dup(fd_out);
        }

        while(fd_err==STDIN_FILENO || fd_err==STDOUT_FILENO)
        { /* Move stderr out of the way of stdin/stdout */
            fd_err=dup(fd_err);
        }

        fd_in=dup2(fd_in,STDIN_FILENO);
        fd_out=dup2(fd_out,STDOUT_FILENO);
        fd_err=dup2(fd_err,STDERR_FILENO);

        if(execvp(cmd.args[0], cmd.args) == -1)
        {
            perror("myshell");
        }
        exit(1);
    }
    else
    {
        if(cmd.mode == BACKGROUND)
            signal(SIGCHLD, SIG_IGN);
        else
        // do {
            waitpid(pid, &ret, WUNTRACED);
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