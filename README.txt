
Smallsh is a shell written in C.  It includes the following features:
1. Provides a prompt for running commands
2. Handles blank lines and comments, which are lines beginning with the # character
3. Provides expansion for the variable $$
4. Executes 3 commands exit, cd, and status via code built into the shell
5. Executes other commands by creating new processes using a function from the exec family of functions
6. Supports input and output redirection
7. Supports running commands in foreground and background processes
8. Implements custom handlers for 2 signals, SIGINT and SIGTSTP


To compile and run this program with the gcc compiler, please follow these steps:

1.	Run the command:

	gcc --std=gnu99 -o smallsh main.c

	This compiles the program with the gcc compiler using the gnu99 standard. 
	It will create an object file called smallsh.

2.	Run the command: 

	./smallsh

	This command will run the program. 

3.	Interact with the program!

