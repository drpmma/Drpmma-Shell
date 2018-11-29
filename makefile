OBJS=myshell.o job_ctrl.o args.o
myshell: ${OBJS}
	gcc -o myshell ${OBJS}

myshell.o: myshell.c

job_ctrl.o: job_ctrl.c

args.o: args.c

clean:
	rm -f *.o myshell
