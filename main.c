

#include<stdio.h>
#include<string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "linkedlist.h"
#include "parseInput.h"
#include "execCommand.h"

int bgAllowed = 1; //global variable to manage fg only mode 

//gets command input from user using getline()
char* read_input(){
    char *line = NULL;
    size_t size = 0;
    getline(&line, &size, stdin); 
    return line;
}

//handler for ^Z (SIGTSTP)
//toggles foreground-only mode
void handle_SIGTSTP(int signo){
    //if bgAllowed is set to 1, set it to 0 and tell user they have entered fg only mode
    if(bgAllowed == 1){
        char *message = "Entering foreground-only mode (& is now ignored)\n";
        write(STDOUT_FILENO, message, 49);
        fflush(stdout);
        bgAllowed = 0;
    }

    //else, set it to zero and tell user they are exiting fg only mode
    else {
        char *message ="Exiting foreground-only mode.\n";
        write(STDOUT_FILENO, message, 29);
        fflush(stdout);
        bgAllowed = 1;
    }
}

//prints exit status 
//this code was adapted from Module 4 Exploration: Process API 
void printStatus(int exitStatus) {
    if(WIFEXITED(exitStatus)){ //if normal termination
      printf("exit value %d\n", WEXITSTATUS(exitStatus));
    } else{
      printf("terminated by signal %d\n", WTERMSIG(exitStatus));
    }
}


int main(int argc, char **argv){
    
    char *userInput;
    struct command *input;
    int exitStatus = 0; //inits exit status to 0
    int exitBool = 1;
    int childPid = 0;
    struct Node* bgLLHead = NULL; //inits LL of background processes
    struct sigaction SIGINT_action = {0}; //init SIGINT action struct
    struct sigaction SIGTSTP_action ={0}; //init SIGTSTP action struct 


    //handler to ignore ^C in shell 
    SIGINT_action.sa_handler = SIG_IGN;
    SIGINT_action.sa_flags = SA_RESTART;
    sigfillset(&(SIGINT_action.sa_mask));
    sigaction(SIGINT, &SIGINT_action, NULL);

    //handles SIGTSTP/redirects it to handleSIGSTP which toggles foreground-only mode
    SIGTSTP_action.sa_handler = handle_SIGTSTP;
    SIGTSTP_action.sa_flags= SA_RESTART;
    sigfillset(&(SIGTSTP_action.sa_mask));
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);
    
    while(exitBool == 1){ //while "exit" command does not change the exitBool
    printf(": ");
    fflush(stdout);
    userInput = read_input();

    //if the user has input a valid (non-comment) command 
    if(strncmp(&userInput[0], "#",1) != 0){
         input = parse_input(userInput); //returns a command struct 

        if(*input->commandAndArgs == NULL){
            //do nothing, prompt another command  
            //free(input);
        }

        //exit built in function - sets exitBool to 0 to exit the while loop
        else if(strcmp(input->commandAndArgs[0], "exit") == 0) {
			exitBool=0;
            //free(input);
		    }

        //cd built in function - uses chdir to change directories 
        else if (strcmp(input->commandAndArgs[0], "cd")==0) {
            
            if(input->commandAndArgs[1]== NULL){ //if there are no arguments
                chdir(getenv("HOME")); //changes to the directory specified in the HOME env
            }
            else{
                chdir(input->commandAndArgs[1]); //changes to directory specified in first argument
            }
            //free(input);
        }

        //status builtin function - returns 
        else if (strcmp(input->commandAndArgs[0], "status")==0){
            printStatus(exitStatus);
            fflush(stdout);
            //free(input);
        }

        else {
        //exec command functions returns bgLLHead, a linked list of background functions
        bgLLHead = execCommand(input, &exitStatus, &childPid, bgLLHead, SIGINT_action, bgAllowed);
        //free(input);
        }

    }
    

    //check if any background processes have terminated
    //remove terminated background processes from LL
    struct Node *head = bgLLHead;

    while(head != NULL) {
        
        pid_t childExitMethod = -5;
        
        int result = waitpid(head->data, &childExitMethod, WNOHANG);
        if (result != 0){
            printf("Background pid %d is done: ", head->data);
            printStatus(childExitMethod); //prints exit status
            fflush(stdout);
            deleteNode(&bgLLHead, head->data); //deletes finished from LL 
        }
        head= head->next;
    }
    
    free(userInput);
    fflush(stdout);
    }
    
    //CLEAN UP/ terminate all outstanding background processes
    while(bgLLHead != NULL){
        kill(bgLLHead->data, SIGKILL);
        deleteNode(&bgLLHead, bgLLHead->data);
        bgLLHead= bgLLHead->next;
    }
    //exit
    free(input);
    }

    
