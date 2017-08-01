OBJS=myshell.o job_ctrl.o
myshell: ${OBJS}
	gcc -o myshell ${OBJS}

myshell.o: myshell.c

job_ctrl.o: job_ctrl.c

clean:
	rm -f *.o myshell