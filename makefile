OBJS=myshell.o
myshell: ${OBJS}
	gcc -o myshell ${OBJS}
myshell.o: myshell.c

clean:
	rm -f *.o