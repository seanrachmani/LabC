#link
all : myshell Printers looper mypipe

mypipe : mypipe.o
	gcc -m32 -g -Wall -o mypipe mypipe.o


#compile 
myshell.o : myshell.c
	gcc -m32 -g -Wall -c -o myshell.o myshell.c

mypipe.o : mypipe.c
	gcc -m32 -g -Wall -c -o mypipe.o mypipe.c

#clean
clean :
	rm -f *.o myshell mypipe