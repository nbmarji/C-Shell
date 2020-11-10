#include<stdio.h>
#include<string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>


//defines command struct
struct command {
    char *commandAndArgs[512];
    char *input;
    char *output;
    int ampersand; //bool for if there's an ampersand 
};

//replaces $$ with PID of current process 
char *replaceWithPid(char *token){
    int pid = getpid();
    char pidstr[7];
    char *newstr= malloc(sizeof(token)+sizeof(pidstr));

    sprintf(pidstr, "%d", pid);

    strcpy(newstr, token);
    newstr[strlen(newstr)-2]= 0;
    strcat(newstr, pidstr);
    
    return newstr;
}

//parses input and puts it into a "command" struct 
struct command *parse_input(char *currLine){
    struct command *currCommand= malloc(sizeof(struct command)); 
    
    //if & is the last, set ampersand = True and removes from line
    char *lastChar = &currLine[strlen(currLine)-2];
    int ampBool;
    if(strcmp(lastChar,"&\n") == 0){
        ampBool = 1; // set bool to True
        currCommand->ampersand = ampBool;
        currLine[strlen(currLine)-2]= 0;
    }
    //otherwise, sets the ampersand value to 0 in the command struct
    else{
        ampBool = 0; //sets bool to false 
        currCommand->ampersand = ampBool;
    }   

    //puts remaining commands and args into an array to be passed to execvp()
    int i = 0;
    char *saveptr;
    char *token= strtok_r(currLine, " \n", &saveptr);
    
    while(token != NULL){
        
        //if input file exists (first), puts it in command-> input, removes from line,checks if there's output
        
        if(strcmp(&token[0],"<")==0){
        token= strtok_r(NULL, " \n", &saveptr);
        currCommand->input = token;
        token= strtok_r(NULL, "\n ", &saveptr);
        
        if(token != NULL && strcmp(&token[0],">")==0){ //checks if there's an output file after
        token= strtok_r(NULL, " \n", &saveptr);
        currCommand->output = token; //if so, puts it in command-> output
        token= strtok_r(NULL, "\n ", &saveptr);
            }
        }
        
        //if output file exists (first), puts in the name of the file in command-> output and removes from line
        else if (strcmp(&token[0],">")==0 && token != NULL){
        token= strtok_r(NULL, " \n", &saveptr);
        currCommand->output = token;
        token= strtok_r(NULL, "\n ", &saveptr);

        if(token != NULL && strcmp(&token[0],"<")==0){// checks if there's an input file after
        token= strtok_r(NULL, " \n", &saveptr);
        currCommand->input = token; //if so assigns it to command->input
        token= strtok_r(NULL, "\n ", &saveptr);
            }
        }

        //checks for $$ and replaces if necessary
        if(token !=NULL && strcmp(&token[strlen(token)-2],"$$")==0){
            token = replaceWithPid(token);
        }
        
        currCommand->commandAndArgs[i] = token; //puts token in args array 
        token= strtok_r(NULL, " \n", &saveptr);
        i++;
    }

    currCommand->commandAndArgs[i] = NULL; //sets null terminator 
    free(token);
    return currCommand;
}
