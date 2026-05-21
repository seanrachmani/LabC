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


//global var:
int debug = 0;



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
        fgets(input,2048,stdin);
        //parse:
        cmdLine* line = parseCmdLines(input);
        //if the user clicked enter etc the parsing is NULL
        if(line != NULL){
            char* commandName = line->arguments[0];
            if(strcmp(commandName,"quit") == 0){
                freeCmdLines(line);
                exit(0);
            }
            if(strcmp(commandName,"cd") == 0){
                if(chdir(line->arguments[1])==-1){
                    fprintf(stderr,"cd operation has failed\n");
                }  
            }
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
                        if(strcmp(commandName,"stop") == 0){
                            kill(pid,SIGTSTP);
                        }
                        else if(strcmp(commandName,"wakeup") == 0){
                            kill(pid,SIGCONT);
                        }
                        else if(strcmp(commandName,"ice") == 0){
                            kill(pid,SIGINT);
                        }
                        else if(strcmp(commandName,"nuke") == 0){
                            kill(pid*-1,SIGKILL);
                        }
                    }
                }
                else{
                    execute(line);
                }
            }
            freeCmdLines(line);
        }
    }
}