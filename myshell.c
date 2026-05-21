#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/limits.h>
#include "LineParser.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

//given
#define TERMINATED  -1
#define RUNNING 1
#define SUSPENDED 0
#define HISTLEN 10


//this is given:
typedef struct process {
    cmdLine* cmd; //parsed cmdline
    pid_t pid;
    int status;
    struct process *next; //next process in chain
} process;


//global var:
int debug = 0;
process* process_list = NULL;
char* history[HISTLEN];
int history_count = 0;
int history_newest = 0;
int history_oldest = 0;

//LAB C part3.a
/*
Create process list to the begining
Receive a process list (process_list), a command (cmd), and the process id (pid) of the process running the command. 
Note that process_list is a pointer to a pointer so that we can insert at the beginning of the list if we wish.
//bc its passing the ponter as ref so its the copy of the actual memoery address,
so if we only put one * we didnt really have the memoey in order to change the node address
*/
void addProcess(process** process_list, cmdLine* cmd, pid_t pid) {
    process* newProc = (process*)malloc(sizeof(process));
    newProc->cmd = cmd;
    newProc->pid = pid;
    newProc->status = RUNNING;
    newProc->next = *process_list;
    //this is the actual memory adressof the list we adding node to:
    *process_list = newProc;
}

//print process list
void printProcessList(process** process_list) {
    //labc3b addition
    updateProcessList(process_list);
    //labc3a
    fprintf(stdout, "PID\t\tSTATUS\t\tCommand\n");
    process* curr = *process_list;
    process* prev = NULL;
    
    while (curr != NULL) {
        const char* status_str = "";
        if (curr->status == RUNNING) status_str = "Running";
        else if (curr->status == SUSPENDED) status_str = "Suspended";
        else if (curr->status == TERMINATED) status_str = "Terminated";

        fprintf(stdout, "%d\t\t%s\t\t", curr->pid, status_str);
        //print cmd name and args like sleep 5
        for (int i = 0; i < curr->cmd->argCount; i++) {
            fprintf(stdout, "%s ", curr->cmd->arguments[i]);
        }
        fprintf(stdout, "\n");

        //LABC3b addition:go throght list find terminated procs and delete them
        //if a proc was freshly terminated delte it after printing it 
        if (curr->status == TERMINATED) {
            process* delete = curr;
            if (prev == NULL) { //its the first node
                *process_list = curr->next;
            } 
            else { //middle of list
                prev->next = curr->next;
            }
            curr = curr->next;
            freeCmdLines(delete->cmd);
            free(delete);
        } 
        else {
            prev = curr;
            curr = curr->next;
        }
    }
}

//free al memory allocated for list
void freeProcessList(process* process_list) {
    process* curr = process_list;
    while (curr != NULL) {
        process* delete = curr;
        curr = curr->next;
        freeCmdLines(delete->cmd);
        //we free what we malloc in addprocess:
        free(delete);
    }
}


/*
check if proc done for all process and update it to actual status
waitpid Flags (The Instructions)
WNOHANG ➔ With No Hang: Makes waitpid non-blocking; returns immediately if the child process is still running without freezing the shell.
WUNTRACED ➔ Wait if Untraced: Tells waitpid to also report status if a child process was stopped/suspended (e.g., via SIGTSTP).
WCONTINUED ➔ Wait if Continued: Tells waitpid to also report status if a stopped child process was resumed/woken up (e.g., via SIGCONT).
WIF Macros (The Evaluation)
WIFSTOPPED(status) ➔ Wait If Stopped: Returns true if the child process was temporarily suspended/stopped.
WIFCONTINUED(status) ➔ Wait If Continued: Returns true if the child process was resumed and is running again.
WIFEXITED(status) ➔ Wait If Exited: Returns true if the child process terminated normally (e.g., called exit or reached the end of main).
WIFSIGNALED(status) ➔ Wait If Signaled: Returns true if the child process was forcefully killed by an external signal (e.g., SIGINT or SIGKILL).
*/
void updateProcessList(process **process_list) {
    process* curr = *process_list;
    while (curr != NULL) {
        int status;
        pid_t res = waitpid(curr->pid, &status, WNOHANG | WUNTRACED | WCONTINUED);
        if (res > 0) {
            if (WIFSTOPPED(status)) {
                curr->status = SUSPENDED;
            } else if (WIFCONTINUED(status)) {
                curr->status = RUNNING;
            } else if (WIFEXITED(status) || WIFSIGNALED(status)) {
                curr->status = TERMINATED;
            }
        //if he cant find it so it dies long time ago
        } else if (res == -1) {
            curr->status = TERMINATED;
        }
        curr = curr->next;
    }
}


//find the process with pid in processlist anf chang its status to recieved status
void updateProcessStatus(process* process_list, int pid, int status) {
    process* curr = process_list;
    while (curr != NULL) {
        if (curr->pid == pid) {
            curr->status = status;
            return;
        }
        curr = curr->next;
    }
}




//part4 LABC
void addToHistory(const char* input) {
    if (history_count == HISTLEN) {
        free(history[history_oldest]);
        history_oldest = (history_oldest + 1) % HISTLEN;
    } else {
        history_count++;
    }
    history[history_newest] = strdup(input);
    history_newest = (history_newest + 1) % HISTLEN;
}

void printHistory() {
    int idx = history_oldest;
    for (int i = 0; i < history_count; i++) {
        fprintf(stdout, "%d %s\n", i + 1, history[idx]);
        idx = (idx + 1) % HISTLEN;
    }
}

const char* getHistoryCommand(int n) {
    if (n < 1 || n > history_count) {
        return NULL;
    }
    int idx = (history_oldest + n - 1) % HISTLEN;
    return history[idx];
}


/*
==========execute==========
==========task 0a notes:==========
PRE:receives a parsed line
POST:invokes the program specified in the cmdLine using the proper system call
EXECV:int execv(const char* path,char* const argv[])
path is saved in argv[0](parse did it for us), returns only in case of an error


==========task 1b notes:==========
1)fork returns pid
2)if(!(pid=fork())) bc we want to save the son id or somethign like this, might understand later
3)fork bomb is massive duplication withoud execv and without waiting for child to end -- crash...
==========task 1b theoretical:==========
1)Q:If execvp fails, use _exit() (see man) to terminate the process. (Why?)
  A:regular exit closes open files, print stuff etc and we dont
  want  the child to do it, bc he is the same as his parent and
  we will create duplication. _exit terminate immediately 
*/
void execute(cmdLine* pCmdLine){
    //if next !=NULL we have piped
    if (pCmdLine->next != NULL) {
        //we dont want output in left bc the job of left side to put stuff into pipe(right will stuck bc we redirect to out and
        //ddintput anything in pipe.)
        //we dont want input in the right bc then we put something in the pipe and then redirect to file so no one reads from pipe
        if (pCmdLine->outputRedirect != NULL || pCmdLine->next->inputRedirect != NULL) {
            fprintf(stderr, "invalid redirection for pipeline\n");
            return;
        }

        int pipefd[2];
        pid_t pid1, pid2;

        if (pipe(pipefd) == -1) {
            perror("pipe error");
            return;
        }
        //first child --->pipe
        if (!(pid1 = fork())) {
            //input file other that stdin
            if (pCmdLine->inputRedirect != NULL) {
                int inFd = open(pCmdLine->inputRedirect, O_RDONLY);
                if (inFd == -1) {
                    perror("open input error:");
                    _exit(1);
                }
                close(0);
                dup(inFd);
                close(inFd);
            }
            close(1);
            dup(pipefd[1]);
            close(pipefd[1]);
            close(pipefd[0]);
            //execvp(name of process, args for process)
            execvp(pCmdLine->arguments[0], pCmdLine->arguments);
            //exe open entirly differenet proces so if we have gone here we got some error.
            perror("execvp child 1 error");
            _exit(1);
        }
        //second child >---pipe
        if (!(pid2 = fork())) {
            //changeoutput from stdout
            if (pCmdLine->next->outputRedirect != NULL) {
                int outFd = open(pCmdLine->next->outputRedirect, O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR);
                if (outFd == -1) {
                    perror("open output error:");
                    _exit(1);
                }
                close(1);
                dup(outFd);
                close(outFd);
            }
            close(0);
            dup(pipefd[0]);
            close(pipefd[0]);
            close(pipefd[1]);
            execvp(pCmdLine->next->arguments[0], pCmdLine->next->arguments);
            perror("execvp child 2 error");
            _exit(1);
        }
        //parent  closures =)
        close(pipefd[0]);
        close(pipefd[1]);
        //================end of pipeline support



        addProcess(&process_list, pCmdLine, pid1);
        addProcess(&process_list, pCmdLine->next, pid2);

        if (pCmdLine->blocking == 1) {
            waitpid(pid1, NULL, 0);
            updateProcessStatus(process_list, pid1, TERMINATED);
        }
        if (pCmdLine->next->blocking == 1) {
            waitpid(pid2, NULL, 0);
            updateProcessStatus(process_list, pid2, TERMINATED);
        }
    } else {
        const char* path = pCmdLine->arguments[0];
        pid_t pid;
        //code taken from lecture 2:
        if(!(pid=fork())){ //child
        //end of taken code
            //task 3 - redirection:
            //note: in the next part we override stdin stdout and then execv will use our fds
            char const* in = pCmdLine->inputRedirect;	
            char const* out = pCmdLine->outputRedirect;
            int inFd,outFd;
            if(in != NULL){
                inFd = open(in,O_RDONLY);
                if(inFd == -1){
                    perror("open input error:");
                    freeCmdLines(pCmdLine);
                    _exit(1);
                }
                else{
                    //its fine to close 0 since its child. O Is fd for stdin 
                    close(0);
                    //npw fd=0 is available
                    dup(inFd);
                    //now our file is loaded to input fd(dup using the lowest unused fd and its 0 bc were just cleared it)
                    close(inFd);
                }  
            }
            if(out != NULL){
                //trunc- ride existed file. 
                //open(path,flags,permissions)
                outFd = open(out,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR);
                if(outFd == -1){
                    perror("open output error:");
                    freeCmdLines(pCmdLine);
                    _exit(1);
                }
                else{
                    //1 is fd for stdout
                    close(1);
                    //now 1 is available for dup:
                    dup(outFd);
                    //now our file is loaded to stdput
                    close(outFd);
                }
            }
            execvp(path,pCmdLine->arguments);
            //if we got to these line we returned aka error:
            perror("execvp execute error:");
            freeCmdLines(pCmdLine);
            _exit(1);
        }
        
        addProcess(&process_list, pCmdLine, pid);
        
        if(debug){
            //were printing info about child from the parent process
            fprintf(stderr,"PID: %d\nfile name: %s\n",pid,path);
            if(pCmdLine->blocking){
                //parent blocked, child in foreground
                fprintf(stderr,"foreground\n");
            }
            else{
                //parent unblocked(available), child in background
                fprintf(stderr,"background\n");
            }   
        }
        //wait for child iff no & - child is in foreground - blocking is 1 - shell blocked:
        if(pCmdLine->blocking == 1){
            waitpid(pid,NULL,0);
            updateProcessStatus(process_list, pid, TERMINATED);
        }
    }
}


/*
==========0a theoretical==========
1)deleting main memory:
Q:Although you loop infinitely, the execution ends after execv. Why is that?
A:successful execv deletes current process memory includes our main & infinity loop,
and the current PID is changing to the code of the new command we parsed. 
2)full path:
Q:You must place the full path of an executable file in-order to run properly. why?
A:bc execv search the command in the directory we are currently in.
and there is no basic commands in the lab folder. 
3)execvp:
-after replacing to execvp no need for full path sunce execvp knows variable named PATH
that saving default folders for basic commands. so just ls working
-Q:Wildcards, as in "ls *", are not working. (Again, why?)
A:bc wildcards being maintained by shell and not by os,
and right now our shell doesnt have this feature. 
for example bash sending collecting file/folders names in current folder and 
sending the os ls command with this list so we get all subfolders too.
*/

/*
===========myNotes==========
1)cant use == for strings
2)cd has to be here- cnat be child bc we need the path changing in here
if it was the child it would change it but then die and we remain with same path
3)int chdir(const char *path); 0 on success, -1 on failure
*/

int main(int argc, char **argv){
    //check debug:
    for(int i=0; i<argc; i++){
        if(strcmp(argv[i],"-d") == 0){
            debug = 1;
        }
    }
    while(1){
        char buf[PATH_MAX];
        char* path = getcwd(buf,PATH_MAX);
        fprintf(stdout,"current working directory is:%s\n",path);
        char input[2048];
        //read from user
        if (fgets(input,2048,stdin) == NULL) {
            break;
        }
        
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) {
            continue;
        }

        if (input[0] == '!') {
            const char* hist_cmd = NULL;
            if (strcmp(input, "!!") == 0) {
                hist_cmd = getHistoryCommand(history_count);
            } else {
                int n = atoi(&input[1]);
                hist_cmd = getHistoryCommand(n);
            }
            
            if (hist_cmd == NULL) {
                fprintf(stderr, "Event not found\n");
                continue;
            }
            strcpy(input, hist_cmd);
            fprintf(stdout, "%s\n", input);
        }

        addToHistory(input);
        
        //parse:
        cmdLine* line = parseCmdLines(input);
        //if the user clicked enter etc the parsing is NULL
        if(line != NULL){
            char* commandName = line->arguments[0];
            if(strcmp(commandName,"quit") == 0){
                freeCmdLines(line);
                //process list is global var
                freeProcessList(process_list);
                for (int i = 0; i < history_count; i++) {
                    free(history[(history_oldest + i) % HISTLEN]);
                }
                exit(0);
            }
            else if(strcmp(commandName,"cd") == 0){
                if(chdir(line->arguments[1])==-1){
                    fprintf(stderr,"cd operation has failed\n");
                }
                freeCmdLines(line);
            }
            else if (strcmp(commandName, "history") == 0) {
                printHistory();
                freeCmdLines(line);
            }
            //part 3a labc
            else if (strcmp(commandName, "procs") == 0) {
                //takes procces and make it ** ptr to ptr
                printProcessList(&process_list);
                freeCmdLines(line);
            }
            //end of lab c addition
            //LAB2 code:
            else{
                //task 2, shell commands:
                //kill(pid,signal)
                char* stringPid;
                int pid;
                if(
                (strcmp(commandName,"stop") == 0)   ||
                (strcmp(commandName,"wakeup") == 0) ||
                (strcmp(commandName,"ice") == 0)    ||
                (strcmp(commandName,"nuke") == 0)
                ){
                    stringPid = line->arguments[1];
                    if(stringPid==NULL){
                        fprintf(stderr,"no valid pid\n");
                        
                    }
                    else{
                        pid = atoi(stringPid);
                        //LAB C ADiition: update status of process according to usr requiremnet
                        if(strcmp(commandName,"stop") == 0){
                            if(kill(pid,SIGTSTP) == 0) updateProcessStatus(process_list, pid, SUSPENDED);
                        }
                        else if(strcmp(commandName,"wakeup") == 0){
                            if(kill(pid,SIGCONT) == 0) updateProcessStatus(process_list, pid, RUNNING);
                        }
                        else if(strcmp(commandName,"ice") == 0){
                            if(kill(pid,SIGINT) == 0) updateProcessStatus(process_list, pid, TERMINATED);
                        }
                        else if(strcmp(commandName,"nuke") == 0){
                            //-1 bc acccording to man this is how linux can treat all group(negative num nc all pid are positive)
                            if(kill(pid*-1,SIGKILL) == 0) updateProcessStatus(process_list, pid, TERMINATED);
                        }
                    }
                    freeCmdLines(line);
                }
                else{
                    execute(line);
                }
            }
        }
    }
    return 0;
}