#link
all : myshell looper mypipeline

mypipeline : mypipeline.o
	gcc -m32 -g -Wall -o mypipeline mypipeline.o

myshell : LineParser.o myshell.o
	gcc -m32 -g -Wall -o myshell LineParser.o myshell.o


looper : looper.o
	gcc -m32 -g -Wall -o looper looper.o

#compile 
myshell.o : myshell.c
	gcc -m32 -g -Wall -c -o myshell.o myshell.c

mypipeline.o : mypipeline.c
	gcc -m32 -g -Wall -c -o mypipeline.o mypipeline.c

LineParser.o : LineParser.c
	gcc -m32 -g -Wall -c -o LineParser.o LineParser.c	

looper.o : looper.c
	gcc -m32 -g -Wall -c -o looper.o looper.c
#clean
clean :
	rm -f *.o myshell looper mypipeline