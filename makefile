#link
all : myshell looper mypipe

mypipe : mypipe.o
	gcc -m32 -g -Wall -o mypipe mypipe.o

myshell : LineParser.o myshell.o
	gcc -m32 -g -Wall -o myshell LineParser.o myshell.o


looper : looper.o
	gcc -m32 -g -Wall -o looper looper.o

#compile 
myshell.o : myshell.c
	gcc -m32 -g -Wall -c -o myshell.o myshell.c

mypipe.o : mypipe.c
	gcc -m32 -g -Wall -c -o mypipe.o mypipe.c

LineParser.o : LineParser.c
	gcc -m32 -g -Wall -c -o LineParser.o LineParser.c	

looper.o : looper.c
	gcc -m32 -g -Wall -c -o looper.o looper.c
#clean
clean :
	rm -f *.o myshell looper mypipe