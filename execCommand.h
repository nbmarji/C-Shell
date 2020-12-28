#include<stdio.h>
#include<string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>


//executes the command with the execvp
void handle_SIGINT(int signo){
	char* message = "Caught SIGINT.\n";
	write(STDOUT_FILENO, message, 20);
    fflush(stdout);
    }

struct Node *execCommand(struct command *command, int *childStatus, int *childPid, struct Node *head, struct sigaction SIGINT_action, int bgAllowed){  
    pid_t spawnpid = -5;
    
	spawnpid = fork();
	switch (spawnpid){
		case -1:
      // to execute if the fork failed
			perror("fork() failed!");
			exit(1);
			break;

		case 0:
      // child will execute the code in this branch
            
            //code for redirecting I/O is adapted from Module 5 Exploration: Processes and I/O
			
            //INPUT
            if(command->input != NULL){
                // Open source file
                int sourceFD = open(command->input, O_RDONLY);
                if (sourceFD == -1){
                    printf("cannot open %s for input\n", command->input);
                    exit(1);
                }

                //redirect stdin to source file
                int result = dup2(sourceFD, 0);
                if(result == -1){
                    perror("source dup2()");
                    exit(2);
                }

            }
            //else if it's a background process, redirect to dev/null
            else if (command->ampersand == 1){
                //open /dev/null
                int sourceFD = open("/dev/null", O_RDONLY);
                if (sourceFD == -1){
                    perror("source open()");
                    exit(1);
                }

                //redirect stdin to /dev/null 
                int result = dup2(sourceFD, 0);
                if(result == -1){
                    perror("source dup2()");
                    exit(2);
                }
            }

            //OUTPUT
            if(command->output != NULL){
                // Open target file
                int targetFD = open(command->output, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if (targetFD == -1) { 
                printf("command output is %s\n", command->output);
                printf("cannot open %s for output\n", command->output); 
                exit(1); 
                }
  
                // Redirect stdout to target file
                int result = dup2(targetFD, 1);
                if (result == -1) { 
                    perror("target dup2()"); 
                    exit(2); 
                }
            }
            //else if it's a background process, redirect to dev/null
            else if (command->ampersand == 1) {
                // Open /dev/null/
                int targetFD = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if (targetFD == -1) { 
                perror("target open()"); 
                exit(1); 
                }
  
                // Redirect stdout to target file
                int result = dup2(targetFD, 1);
                if (result == -1) { 
                    perror("target dup2()"); 
                    exit(2); 
                }
            }
            if(command->ampersand != 1) {
                SIGINT_action.sa_handler = handle_SIGINT;
                
                sigfillset(&SIGINT_action.sa_mask);
	            SIGINT_action.sa_flags = 0;
                sigaction(SIGINT, &SIGINT_action, NULL);
            }
                
            execvp(command->commandAndArgs[0],command->commandAndArgs);
            printf("%s: no such file or directory\n", command->commandAndArgs[0]);
            fflush(stdout);
            exit(EXIT_FAILURE);
			break;

		default:
        // parent will execute the code in this branch
            //background process (if allowed)
            if(command->ampersand == 1 && bgAllowed == 1){

                fflush(stdout);
                waitpid(spawnpid, childStatus, WNOHANG);

                //adds to LL of background jobs to kill on exit
                push(&head, spawnpid);
                printf("A background process with PID %d is running\n", spawnpid); //runs process in the background
                fflush(stdout);
                
            } else {
                //foreground process
                waitpid(spawnpid, childStatus, 0);//parent resumes after execution of child
            }
                
		    break;
	}
    *childPid = spawnpid;
    return head; //returns the head of the background process
}
