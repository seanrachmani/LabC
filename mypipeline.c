#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    //pipe file descriptors
    int pipefd[2];
    pid_t child1, child2;

    if (pipe(pipefd) == -1) {
        perror("xouldnt pipe");
        exit(1);
    }
    
    //ps -xl  show proccesses in format
    fprintf(stderr, "(parent_process>forking...)\n");
    child1 = fork();
    
    //first child
    if (child1 == 0) {
        fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe...)\n");
        //clos stdout
        close(1);
        //write is1
        //dup put the write side of pipe in the lowest place aka stdout we close
        dup(pipefd[1]);
        close(pipefd[1]);
        close(pipefd[0]);
        //executing ps-xl
        //nul terminated array for ps command
        fprintf(stderr, "(child1>going to execute cmd ps -xl)\n");
        char *argv1[] = {"ps", "-xl", NULL};
        //excvp(file to exe, args for passing)
        execvp("ps", argv1);
        perror("ps child error");
        exit(1);
    }
    
    //parent:
    fprintf(stderr, "(parent_process>created process with id %d)\n", child1);
    fprintf(stderr, "(parent_process>closing the write end of the pipe...)\n");
    close(pipefd[1]);

    fprintf(stderr, "(parent_process>forking...)\n");
    child2 = fork();
    
    if (child2 == 0) {
        fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe...)\n");
        //close stdin
        close(0);
        //dup put read side of pipe in stding
        dup(pipefd[0]);
        close(pipefd[0]);
        //no need to close pfd[1] bc its inherited closed from father
        fprintf(stderr, "(child2>going to execute cmd grep 5)\n");
        char *argv2[] = {"grep", "5", NULL};
        //grep and ps and all common exes are on PATHwhich is env var 
        execvp("grep", argv2);
        perror("grep child error");
        exit(1);
    }
    
    //father: 
    fprintf(stderr, "(parent_process>created process with id %d)\n", child2);
    fprintf(stderr, "(parent_process>closing the read end of the pipe...)\n");
    close(pipefd[0]);
    
    //wait(for who,no store info, 0 for the same process group id)
    fprintf(stderr, "(parent_process>waiting for child processes to terminate...)\n");
    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);  
    
    fprintf(stderr, "(parent_process>exiting...)\n");
    return 0;
}

//in the end the goal is the piep to get all procces from one side and print all which has 5
//grep search patetrns 
//ps show perocesses