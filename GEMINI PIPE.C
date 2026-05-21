#include stdio.h
#include stdlib.h
#include unistd.h
#include systypes.h
#include syswait.h

int main() {
    int pipefd[2];
    pid_t child1, child2;

    if (pipe(pipefd) == -1) {
        perror(pipe);
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, (parent_processforking...)n);
    child1 = fork();
    
    if (child1 == 0) {
        fprintf(stderr, (child1redirecting stdout to the write end of the pipe...)n);
        close(1);
        dup(pipefd[1]);
        close(pipefd[1]);
        close(pipefd[0]);
        
        fprintf(stderr, (child1going to execute cmd ps -xl)n);
        char argv1[] = {ps, -xl, NULL};
        execvp(ps, argv1);
        perror(execvp ps);
        exit(EXIT_FAILURE);
    }
    
    fprintf(stderr, (parent_processcreated process with id %d)n, child1);
    fprintf(stderr, (parent_processclosing the write end of the pipe...)n);
    close(pipefd[1]);

    fprintf(stderr, (parent_processforking...)n);
    child2 = fork();
    
    if (child2 == 0) {
        fprintf(stderr, (child2redirecting stdin to the read end of the pipe...)n);
        close(0);
        dup(pipefd[0]);
        close(pipefd[0]);
        
        fprintf(stderr, (child2going to execute cmd grep 5)n);
        char argv2[] = {grep, 5, NULL};
        execvp(grep, argv2);
        perror(execvp grep);
        exit(EXIT_FAILURE);
    }
    
    fprintf(stderr, (parent_processcreated process with id %d)n, child2);
    fprintf(stderr, (parent_processclosing the read end of the pipe...)n);
    close(pipefd[0]);

    fprintf(stderr, (parent_processwaiting for child processes to terminate...)n);
    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);

    fprintf(stderr, (parent_processexiting...)n);
    return 0;
}