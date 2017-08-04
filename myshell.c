#include "myshell.h"

int main(int argc, char *argv[])
{
    set_arg(argc, argv);
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
    int file_flag = 0;

    init();
    while(status && !file_flag)
    {
        cmd_array = malloc(COMMAND_NUMBER * sizeof(struct command));
        memset(cmd_array, 0, COMMAND_NUMBER * sizeof(struct command));
        getcwd(file_path, FILE_PATH_LENGTH);
        
        set_env_pid();
        printf("myshell:%s>", file_path);

        line = read_line(&file_flag);
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
        clear_buffer(cmd_array, line, cmds);
    }
    free(new_PATH);
    free(file_path);
    clear_job_all();
}

void init()
{
    file_path=(char*)malloc(FILE_PATH_LENGTH);
    getcwd(file_path, FILE_PATH_LENGTH);

    PATH = getenv("PATH");
    new_PATH = malloc(PATH_LENGTH);
    memset(new_PATH, 0, PATH_LENGTH);
    strcat(new_PATH, "PATH=");
    strcat(new_PATH, PATH);
    strcat(new_PATH, ":");
    strcat(new_PATH, file_path);
    putenv(new_PATH);

    strcat(file_path, "/myshell");
    setenv("MYSHELL", file_path, 0);
    
    HOME = getenv("HOME");

    job_array = malloc(JOB_NUMBER * sizeof(struct jobs));
    job_init(job_array);

    signals();
}

char* read_line(int* pfile_flag)
{
    char *line = NULL;
    ssize_t bufsize = 0;
    char* cmd_name = getenv("0");
    char* cmd_file = getenv("1");
    if(strcmp(cmd_name, "myshell") == 0 && test_file(cmd_file, TEST_R))
    {
        FILE* fp = fopen(cmd_file, "r");
        getline(&line, &bufsize, fp);
        *pfile_flag = 1;
    }
    else
    {
        getline(&line, &bufsize, stdin);            // 通过getline函数可以方便的读入一行
    }
    return line;
}

int parse_redirect(char** args, int* pfd_in, int* pfd_out)
{
    int is_append = 0;
    int flags;                              // open函数的flag
    int mode = S_IRUSR | S_IWUSR;           // open函数的mode
    for (int i = 0; args[i] != NULL; i++) 
    {
        if ((strcmp(args[i], ">") == 0) || (strcmp(args[i], "<") == 0)) {
            flags = (strcmp(args[i], ">") == 0) ? O_WRONLY | O_CREAT | O_TRUNC : O_RDONLY;
            int fd_tmp = open(args[i + 1], flags, mode);
            if (fd_tmp < 0) {
                perror("run_shell: start_prog");
                if (errno != ENOENT)        // 检查ENOENT错误
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
                if (errno != ENOENT)
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

    execute(cmd_array[0], 0, p1[1], 2);
    close(p1[1]);

    for (i = 1; i < size - 1; i++) {
        /* Read from parent's pipe, write to child's */
        /* Close the pipe we read from, and the one we write to */
        if (i % 2) 
        {
            if (pipe(p2))
                perror("myshell: split_line");
            execute(cmd_array[i], p1[0], p2[1], 2);
            close(p1[0]);
            close(p2[1]);
        } 
        else 
        {
            if (pipe(p1))
                perror("myshell: split_line");

            execute(cmd_array[i], p2[0], p1[1], 2);
            close(p2[0]);
            close(p1[1]);
        }
    }

    /* Finish on stdout */
    if (i % 2) 
    {
        execute(cmd_array[i], p1[0], 1, 2);

        close(p1[0]);
    } else 
    {
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

void parse_var(char** args)
{
    char* env_name;
    char* env;
    for(int i = 0; args[i] != NULL; i++)
    {
        if(args[i][0] == '$')
        {
            env_name = malloc(strlen(args[i] + 1));
            strcpy(env_name, args[i] + 1);
            env = getenv(env_name);
            strcpy(args[i], env);
            free(env_name);
        }
    }
}

void parse_quote(char** args)
{
    if( strcmp(args[0], "kill") == 0 || strcmp(args[0], "fg") == 0 || strcmp(args[0], "bg") == 0)
    {
        return;
    }
    int quote_idx_l = -1, quote_idx_r = -1;
    int find = 0;
    for(int i = 0; args[i] != NULL; i++)
    {
        if(args[i][0] == '\"')
        {
            if(find == 0)
            {
                find = 1;
                quote_idx_l = i;
            }
        }
        int len = strlen(args[i]);
        if(args[i][len - 1] == '\"')
        {
            quote_idx_r = i;
        }
    }
    if(quote_idx_l != -1 && quote_idx_r != -1)
    {
        for(int i = quote_idx_l + 1; i <= quote_idx_r; i++)
        {
            strcat(args[quote_idx_l], args[i]);
            args[i] = NULL;
        }
    }

    char* env = NULL;
    for(int i = 0; args[i] != NULL; i++)
    {
        if(args[i][0] == '\"' || args[i][0] == '$')
        {
            if(args[i][1] == '$' || args[i][0] == '$')
            {
                env = malloc(strlen(args[i]));
                memset(env, 0, strlen(args[i]));
                int k = 0;
                for(int j = 0; args[i][j] != 0; j++)
                {
                    if(args[i][j] != '\"')
                        env[k++] = args[i][j];
                }
                strcpy(args[i], env);
                free(env);
            }
        }
    }
}

int execute(struct command cmd, int fd_in, int fd_out, int fd_err)
{
    int status;
    if(cmd.args[0] == NULL)
        return 1;
    parse_quote(cmd.args);
    parse_var(cmd.args);
    parse_redirect(cmd.args, &fd_in, &fd_out);
    status = check_builtin(cmd);
    if(status != -1)
    {
        status = builtin_env_cmd(cmd);
    }
    pid_t pid, w_pid;
    int ret, exec;

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
        
        if(status == 0)
        {
            exec = 1;
            status = builtin_cmd(cmd);
        }
        if(status == -1 && exec == 0)
        {
            execvp(cmd.args[0], cmd.args);
        }
        exit(status);
    }
    else
    {
        job_ctrl(cmd);
        if(cmd.mode == BACKGROUND)
        {
            struct jobs* temp_job = get_new_job(job_array);
            temp_job->pid = pid;
            temp_job->name = malloc(NAME_SIZE);
            memset(temp_job->name, 0, NAME_SIZE);
            for(int i = 0; cmd.args[i] != NULL; i++)
            {
                strcat(temp_job->name, cmd.args[i]);
                strcat(temp_job->name, " ");
            }
            change_state(pid, JOB_STATE_RUN);
            printf("[%d] %d\n", temp_job->id, temp_job->pid);
        }
        else
        {
            waitpid(pid, &ret, WUNTRACED);
            if(WIFSTOPPED(ret))
            {
                handle_stop(cmd, pid);
            }
            if(WIFEXITED(ret))
            {
                set_env_status(WEXITSTATUS(ret));
            }
            status = WEXITSTATUS(ret);
        }
    }
    if(status == -1)
        return 1;
    if(status == 0)
        return 1;
    if(status == 1)
        return 0;
    return status;
}

int check_builtin(struct command cmd)
{
    if(strcmp(cmd.args[0], "cd") == 0)
    {
        return 0;
    }
    else if(strcmp(cmd.args[0], "time") == 0)
    {
        return 0;
    }
    else if(strcmp(cmd.args[0], "umask") == 0)
    {
        return 0;
    }
    else if(strcmp(cmd.args[0], "environ") == 0)
    {
        return 0;
    }
    else if(strcmp(cmd.args[0], "set") == 0)
    {
        return 0;
    }
    else if(strcmp(cmd.args[0], "unset") == 0)
    {
        return 0;
    }
    else if(strcmp(cmd.args[0], "exec") == 0)
    {
        return 0;
    }
    else if(strcmp(cmd.args[0], "help") == 0)
    {
        return 0;
    }
    else if(strcmp(cmd.args[0], "exit") == 0)
    {
        return 0;
    }
    else if(strcmp(cmd.args[0], "jobs") == 0)
    {
        return 0;
    }
    else if(strcmp(cmd.args[0], "kill") == 0)
    {
        return 0;
    }
    else if(strcmp(cmd.args[0], "test") == 0)
    {
        return 0;
    }
    else if(strcmp(cmd.args[0], "continue") == 0)
    {
        return 0;
    }
    else if(strcmp(cmd.args[0], "shift") == 0)
    {
        return 0;
    }
    else if(strcmp(cmd.args[0], "clr") == 0)
    {
        return 0;
    }
    else
        return -1;
}

int builtin_cmd(struct command cmd)
{
    if(strcmp(cmd.args[0], "time") == 0)
    {
        return shell_time(cmd.args);
    }
    else if(strcmp(cmd.args[0], "help") == 0)
    {
        return shell_help(cmd.args);
    }
    else if(strcmp(cmd.args[0], "exit") == 0)
    {
        return shell_exit(cmd.args);
    }
    else if(strcmp(cmd.args[0], "test") == 0)
    {
        return shell_test(cmd.args);
    }
    else if(strcmp(cmd.args[0], "clr") == 0)
    {
        return shell_clr();
    }
    else if(strcmp(cmd.args[0], "environ") == 0)
    {
        return shell_environ(cmd.args);
    }
    else
        return -1;
}

int builtin_env_cmd(struct command cmd)
{
    if(strcmp(cmd.args[0], "cd") == 0)
    {
        return shell_cd(cmd.args);
    }
    else if(strcmp(cmd.args[0], "umask") == 0)
    {
        return shell_umask(cmd.args);
    }
    else if(strcmp(cmd.args[0], "set") == 0)
    {
        return shell_set(cmd.args);
    }
    else if(strcmp(cmd.args[0], "unset") == 0)
    {
        return shell_unset(cmd.args);
    }    
    else if(strcmp(cmd.args[0], "continue") == 0)
    {
        return shell_continue();
    }
    else if(strcmp(cmd.args[0], "shift") == 0)
    {
        return shell_shift(cmd.args);
    }
    else if(strcmp(cmd.args[0], "exec") == 0)
    {
        return shell_exec(cmd.args);
    }
    else if(strcmp(cmd.args[0], "kill") == 0)
    {
        return shell_kill(cmd.args);
    }
    else
        return -1;
}

int shell_cd(char** args)
{
    if(args[1] == NULL)
    {
        chdir(HOME);
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
        printf("用法 set A OR set A B\n");
    }
    else if(args[2] == NULL)
    {
        setenv(args[1], "NULL", 0);
    }
    else
    {
        setenv(args[1], args[2], 1);
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

int shell_test(char** args)
{
    int res;
    if(args[1][0] == '-')
    {
        switch(args[1][1])
        {
            case 'd':
            {
                res = test_dir(args[2]);
                break;
            }
            case 'b':
            {
                res = test_file(args[2], TEST_B);
                break;
            }
            case 'c':
            {
                res = test_file(args[2], TEST_C);
                break;
            }
            case 'e':
            {
                res = test_file(args[2], TEST_E);
                break;
            }
            case 'f':
            {
                res = test_file(args[2], TEST_F);
                break;
            }
            case 'h':
            {
                res = test_file(args[2], TEST_H);
                break;
            }
            case 'l':
            {
                res = test_file(args[2], TEST_L);
                break;
            }
            case 'p':
            {
                res = test_file(args[2], TEST_P);
                break;
            }
            case 'r':
            {
                res = test_file(args[2], TEST_R);
                break;
            }
            case 'w':
            {
                res = test_file(args[2], TEST_W);
                break;
            }
            case 'x':
            {
                res = test_file(args[2], TEST_X);
                break;
            }
            // default: 
            // {
                
            // }
        }
    }
    else if(args[2][0] == '-')
    {
        if(strcmp(args[2], "-eq") == 0)
        {
            res = test_logic(args, TEST_EQ);
        }
        else if(strcmp(args[2], "-ge") == 0)
        {
            res = test_logic(args, TEST_GE);
        }
        else if(strcmp(args[2], "-gt") == 0)
        {
            res = test_logic(args, TEST_GT);
        }
        else if(strcmp(args[2], "-le") == 0)
        {
            res = test_logic(args, TEST_LE);
        }
        else if(strcmp(args[2], "-lt") == 0)
        {
            res = test_logic(args, TEST_LT);
        }
        else if(strcmp(args[2], "-ne") == 0)
        {
            res = test_logic(args, TEST_NE);
        }
        else
        {
            printf("错误的参数\n");
            return 0;
        }
    }
    if(res == 0)
        printf("false\n");
    else
        printf("true\n");
    return 0;
}

int shell_continue()
{
    char* status_string = IntToString(RETURN_CONTINUE);
    setenv("?", status_string, 1);
    printf("$?=%s\n", getenv("?"));
    return 0;
}

int shell_shift(char** args)
{
    int num;
    if(args[1] == NULL)
        num = 1;
    else
        num = atoi(args[1]);
    char* arg_list = getenv("@");
    char** para = split_str(arg_list, ARGUMENT_SIZE, " ");
    int len = 0;
    for(int i = 0; para[i] != 0; i++)
    {
        len = i;
    }
    len++;
    char* s_arg_len;
    char buf[2];
    if(num >= len)
    {
        s_arg_len = IntToString(0);
        setenv("@", "", 1);
        setenv("*", "", 1);
        setenv("#", s_arg_len, 1);
    }
    else
    {
        arg_list = malloc(ARGUMENT_SIZE);
        memset(arg_list, 0, ARGUMENT_SIZE);
        s_arg_len = IntToString(len - num);
        for(int i = 0; para[i] != NULL; i++)
        {
            if(i + num < len)
            {
                para[i] = para[i + num];
                char* num_s = IntToString(i + 1);
                setenv(num_s, para[i], 1);
                free(num_s);
            }
            else
            {
                char* num_s = IntToString(i + 1);
                unsetenv(num_s);
                free(num_s);
                para[i] = NULL;
            }
        }
        for(int i = 0; i < len - num; i++)
        {
            buf[0] = ' ';
            buf[1] = 0;
            strcat(arg_list, para[i]);
            strcat(arg_list, buf);

        }
        setenv("@", arg_list, 1);
        setenv("*", arg_list, 1);
        setenv("#", s_arg_len, 1);
    }
    free(s_arg_len);
    free(para);
    return 0;
}

int shell_clr()
{
    printf("\033[H\033[J");
    return 0;
}

int test_dir(char* arg)
{
    DIR* dir = NULL;
    struct stat dir_info;
    struct dirent* dirp = NULL;
    char* path = malloc(FILE_PATH_LENGTH);
    if(arg == NULL)
    {
        return 0;
    }
    if(arg[0] != '/' && arg[0] != '~')
    {
        getcwd(path, FILE_PATH_LENGTH);
        int len = strlen(path);
        path[len] = '/';
        path[len + 1] = '\0';
        len++;
        strcpy(path + len, arg);
        printf("%s\n", path);
    }
    else if(arg[0]=='~')
    {
        strcpy(path, HOME);
        strcpy(path + strlen(HOME), arg + 1);
    }
    else
    {
        strcpy(path, arg);
    }
    if(stat(path, &dir_info) == -1)
    {
        return 0;
    }
    else
    {
        if(S_ISDIR(dir_info.st_mode))
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

int test_file(char* arg, int flag)
{
    DIR* dir = NULL;
    struct stat dir_info;
    struct dirent* dirp = NULL;
    char* path = malloc(FILE_PATH_LENGTH);
    if(arg == NULL)
    {
        return 0;
    }
    if(arg[0] != '/' && arg[0] != '~')
    {
        getcwd(path, FILE_PATH_LENGTH);
        int len = strlen(path);
        path[len] = '/';
        path[len + 1] = '\0';
        len++;
        strcpy(path + len, arg);
        printf("%s\n", path);
    }
    else if(arg[0]=='~')
    {
        strcpy(path, HOME);
        strcpy(path + strlen(HOME), arg + 1);
    }
    else
    {
        strcpy(path, arg);
    }

    if(lstat(path, &dir_info) == -1)
    {
        return 1;
    }
    else
    {
        switch(flag)
        {
            case TEST_B:
            {
                if(S_ISBLK(dir_info.st_mode))
                {
                    return 1;
                }
                else
                {
                    return 0;
                }
            }
            case TEST_C:
            {
                if(S_ISCHR(dir_info.st_mode))
                {
                    return 1;
                }
                else
                {
                    return 0;
                }
            }
            case TEST_E:
            {
                return 1;
            }
            case TEST_F:
            {
                if(S_ISREG(dir_info.st_mode))
                {
                    return 1;
                }
                else
                {
                    return 0;
                }
            }
            case TEST_H: case TEST_L:
            {
                if(S_ISREG(dir_info.st_mode))
                {
                    return 1;
                }
                else
                {
                    return 0;
                }
            }
            case TEST_P:
            {
                if(S_ISFIFO(dir_info.st_mode))
                {
                    return 1;
                }
                else
                {
                    return 0;
                }
            }
            case TEST_R:
            {
                if(access(path, R_OK) == 0)
                {
                    return 1;
                }
                else
                {
                    return 0;
                }
            }
            case TEST_W:
            {
                if(access(path, W_OK) == 0)
                {
                    return 1;
                }
                else
                {
                    return 0;
                }
            }
            case TEST_X:
            {
                if(access(path, X_OK) == 0)
                {
                    return 1;
                }
                else
                {
                    return 0;
                }
            }
        }
    }
}

int test_logic(char** args, int flag)
{
    int num1 = atoi(args[1]);
    int num2 = atoi(args[3]);
    switch(flag)
    {
        case TEST_EQ:
        {
            return num1 == num2;
        }
        case TEST_GE:
        {
            return num1 >= num2;
        }
        case TEST_GT:
        {
            return num1 > num2;
        }
        case TEST_LE:
        {
            return num1 <= num2;
        }
        case TEST_LT:
        {
            return num1 < num2;
        }
        case TEST_NE:
        {
            return num1 != num2;
        }
    }
}

void clear_buffer(struct command* cmd_array, char* line, char** cmds)
{
    free(line);
    free(cmds);
    for(int i = 0; i < COMMAND_NUMBER; i++)
    {
        free(cmd_array[i].args);
    }
    free(cmd_array);
}

void signals()
{
    if(signal(SIGCHLD, handle_child) == SIG_ERR)
    {
        printf("signal error.\n");
        return;
    }
    // signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGCONT, SIG_DFL);
}

void set_arg(int argc, char* argv[])
{
    char buf[2];
    char* arg_buf = IntToString(argc);
    char* arg_list = malloc(ARGUMENT_SIZE);
    memset(arg_list, 0, ARGUMENT_SIZE);
    setenv("#", arg_buf, 1);
    free(arg_buf);
    for(int i = 0; argv[i] != 0; i++)
    {
        if(i != 0)
        {
            buf[0] = ' ';
            buf[1] = 0;
            strcat(arg_list, argv[i]);
            strcat(arg_list, buf);
        }
        arg_buf = IntToString(i);
        setenv(arg_buf, argv[i], 1);
    }
    setenv("*", arg_list, 1);
    setenv("@", arg_list, 1);
    free(arg_buf);
    free(arg_list);
}

void set_env_pid()
{
    char* pidstring = IntToString(getpid());
    setenv("$", pidstring, 1);
    free(pidstring);
}

void set_env_status(int status)
{
    char* status_string = IntToString(status);
    setenv("?", status_string, 1);
    free(status_string);
}

char* IntToString(int i)
{
    char* buf = malloc(ARGUMENT_SIZE);
    memset(buf, 0, ARGUMENT_SIZE);
    sprintf(buf, "%d", i);
    return buf;    
}