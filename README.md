# Myshell for Linux
## Supporting the following commands
* bg
* fg
* jobs
* kill
* &
* \> < >>
* |
* test
* cd 
* umask
* time
* env environ
* clear clr
* set
* unset
* exec
* help
* exit
* continue
* shift
* echo
* myshell batchfile
* other external command

## bg
### run jobs in the background
```bash
    bg %id OR bg id
```
## fg
### run jobs in the foreground
```bash
    fg %id OR fg id
```
## jobs
### display status of jobs in the current session
```bash
    jobs
```

## kill
### kill a process by id or pid
```bash
    kill %id OR kill pid
```
## &
### run jobs in the background
```bash
    COMMAND &
```
## > < >> 
### (redirecct)
```bash
    COMMAND1 > FILE
    COMMAND2 >> FILE
    COMMAND3 < FILE
```
## |
### (pipe)
```bash
    COMMAND1 | COMMAND2
```
## test
### check file type and compare values
```bash
    test -b FILE
    test -c FILE
    test -e FILE
    test -f FILE
    test -h FILE
    test -l FILE
    test -p FILE
    test -r FILE
    test -w FILE
    test -x FILE
    test INTEGER1 -eq INTEGER2
    test INTEGER1 -ge INTEGER2
    test INTEGER1 -gt INTEGER2
    test INTEGER1 -le INTEGER2
    test INTEGER1 -lt INTEGER2
    test INTEGER1 -ne INTEGER2
```

## cd
### Change the shell working directory
```bash
    cd
    cd /home/usr
```

## umask
### set or display file mode
```bash
    umask
    umask 0666
```

## time
### display the system time
```bash
    time
```

## env environ
### display all the environment variables 
```bash
    env OR environ
```

## clear clr
### clear the terminal screen
``` bash 
    clear OR clr
```

## set
### set the environment variables
```bash
    set VAR
    set VAR VALUE
```

## unset
### unset the environment variables
```bash
    unset VAR
```

## exec
### replaces the shell with specified command
```bash
    exec ls
```

## help
### display the README doc
```bash
    help
```

## exit
### cause normal process termination
```bash
    exit
```

## continue
### return the value of $? to 10
```bash
    continue
```

## shift
### move popositional parameters
```bash
    shift OR shift n
```

## myshell batchfile
### Processes a batch file that contains a set of commands
```bass
    myshell FILE
```