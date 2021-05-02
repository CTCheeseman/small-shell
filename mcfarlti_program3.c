// Tim McFarland
// ONID: 934066739
// CS 344
// Assignment 3
// Last Modified: 5/2/2021
// Due Date: 5/3/2021

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>

// input length is counted 1 more than 2048 to account for the \n which must be removed
#define		MAX_INPUT_LENGTH	2049
#define		MAX_ARGS			512

// global variable that defines when to enter foreground mode
bool		foregroundOnly = false;

// global variable that holds all background processes with 512 holders
int	pidHolder[MAX_INPUT_LENGTH];

// this keeps track of the exit status of the last foreground process
int lastForeground;

// struct for user command
struct userCmds
{
	char*	arrayOfArgs[MAX_ARGS];		// holds all the arguments in an array
	char*	fInput;						// holds the name of the input file
	char*	fOutput;					// holds the name of the output file
	bool	background;					// determines if the item is a background process or not (determined by ending &)
	bool	ioRedirect;					// determines if there is I/O redirect needed (may not need)
};

// prototype declarations
struct	userCmds* cmdLine(char* userInput);
char*	moneyCheck(char* token);
char*	expandTheMoney(char* varToExpand);
char*	truncateMoney(char* varWithMoney);
void	handleRedirect(struct userCmds* cmdStruct);
void	theForkParty(struct userCmds* cmdStruct, struct sigaction childSignal);
void	handle_SIGTSTP(int handle);
void	pidExitCheck(void);
void	inBackground(int finishedPid, int finishedStatus);
void	handle_SIGINT(int status);


// Takes the userInput and breaks it down into basic components and adds them to
//	the struct userCmds
struct userCmds* cmdLine(char* userInput)
{

	// this will advance the array that items will be added to
	int j = 0;
	
    // the struct must have memory allocated for it
    struct userCmds *allUserCmds = malloc(sizeof(struct userCmds));

    // initializes the token
    char* token = strtok(userInput, " ");

	// initialize all struct variables
	for (int i = 0; i < MAX_ARGS; i++) {
		allUserCmds->arrayOfArgs[i] = NULL;
	}
	allUserCmds->fInput = NULL;
	allUserCmds->fOutput = NULL;
	allUserCmds->background = false;
	allUserCmds->ioRedirect = false;

	while (token != NULL) {

		// alocate memory
		allUserCmds->arrayOfArgs[j] = calloc(strlen(token) + 1, sizeof(char));
		char* expandedVar = calloc(strlen(token) + 1, sizeof(char));
		char* postMoney = calloc(strlen(token) + 1, sizeof(char));

		strcpy(postMoney, token);

		// check to see if there are any $$'s found and add to the array
		expandedVar = moneyCheck(postMoney);


		if (strcmp(expandedVar, ">") != 0 && strcmp(expandedVar, "<") != 0 && strcmp(expandedVar, "") != 0) {
		 	strcpy(allUserCmds->arrayOfArgs[j], expandedVar);
		}

		// if the user is requesting an output file, prepare that for the user
		if (strcmp(token, ">") == 0) {

			allUserCmds->fOutput = calloc(strlen(token) + 1, sizeof(char));

			// goes to the token afer the '>' and assigns that to the file name
			//	to be created
			token = strtok(NULL, " ");

			// this tells us to have output/input
			allUserCmds->ioRedirect = true;
			
			// this is a catch in case the last item in the argument is a > symbol
			//	so there is no segFault
			if (token == NULL) {
				printf("ERROR: You cannot end with the > command");
				fflush(stdout);
				continue;
			}

			strcpy(postMoney, token);
			expandedVar = moneyCheck(postMoney);

			// assigns output file
			strcpy(allUserCmds->fOutput, expandedVar);
			allUserCmds->arrayOfArgs[j] = NULL;
		}

		if (strcmp(token, "<") == 0) {

			allUserCmds->fInput = calloc(strlen(token) + 1, sizeof(char));

			// goes to the token afer the '<' and assigns that to the file name
			//	to be output to
			token = strtok(NULL, " ");

			// this tells us to have output/input
			allUserCmds->ioRedirect = true;

			// this is a catch in case the last item in the argument is a > symbol
			//	so there is no segFault
			if (token == NULL) {
				printf("ERROR: You cannot end with the < command");
				continue;
			}

			strcpy(postMoney, token);
			expandedVar = moneyCheck(postMoney);

			strcpy(allUserCmds->fInput, expandedVar);
			allUserCmds->arrayOfArgs[j] = NULL;
		}

		// go to the next array position and to the next token
		j++;
		token = strtok(NULL, " ");
	}

	if (allUserCmds->ioRedirect) {}

	// this checks to see if the last item in the input is the &
	//	if it is, then the user has requested a background process
	else if (strcmp(allUserCmds->arrayOfArgs[j - 1], "&") == 0) {

		if (!foregroundOnly) {
			allUserCmds->background = true;
		}

		else {
			allUserCmds->background = false;
		}

		allUserCmds->arrayOfArgs[j - 1] = NULL;			// make it so the & is not passed
	}

    return allUserCmds;
}


//takes the first instance of the $ symbol, truncates it, and returns the string without any $
char* truncateMoney(char* varWithMoney) {
	char* firstMoney;
	int index;
	char* truncMoney = malloc(512);

	// finds the index where the first $ is found;
	//	This will be truncated
	firstMoney = strchr(varWithMoney, '$');
	index = (int)(firstMoney - varWithMoney);

	strcpy(truncMoney, varWithMoney);

	// truncate, starting with the first $.
	truncMoney[index] = '\0';

	return truncMoney;
}

// if any instances of $$ were found, it will be expanded with the
//	shell's pid. if there are an even number of $, then the pid
//	will be added to the string by a factor of two. If there is an
//	odd number of $, the afforementioned will be executed and 
//	a $ will be apended at the end
char* expandTheMoney(char* varToExpand) {
	int moneyCounter = 0;
	char* pidString = malloc(125);
	char* expansionHolder = malloc(MAX_INPUT_LENGTH);
	int expansions = 0;

	// find all instances of the charracter $
	for (int i = 0; i < (strlen(varToExpand)); i++) {
		if (varToExpand[i] == '$') {
			moneyCounter++;
		}
	}

	// truncate all of the $ off of the variable
	expansionHolder = truncateMoney(varToExpand);

	// find out how many times to add pid of shell
	expansions = (moneyCounter / 2);

	// get the string version of the process ID
	sprintf(pidString, "%d", getpid());

	// concatenate the pid to the string the amount of times that $$ was present
	for (int j = 0; j < expansions; j++) {
		strcat(expansionHolder, pidString);
	}

	// add a final $ to the new string if the amount of $$'s is odd
	if (moneyCounter % 2 == 1) {
		strcat(expansionHolder, "$");
	}

	return expansionHolder;
}

// this will look at a string and see if there are any $'s
char* moneyCheck(char* token) {
	char* lastChar = malloc(5);
	char* nextToLastChar = malloc(5);
	char* varExpansion = malloc(MAX_INPUT_LENGTH);

	// find the last character and the second to last character
	sprintf(lastChar, "%c", token[strlen(token) - 1]);
	sprintf(nextToLastChar, "%c", token[strlen(token) - 2]);

	// if the last two characters are $$, then do the variable expansion
	if ((strcmp(lastChar, "$") == 0) && (strcmp(nextToLastChar, "$") == 0)) {
		varExpansion = expandTheMoney(token);
		return varExpansion;
	}

	// if there are not two $'s then do nothing
	return token;
}

// gathers user input
char* getUserInput() {
	char* userInput = calloc(MAX_INPUT_LENGTH, sizeof(char));
	int strSize;

	// using fgets to restrict the input to 2056 characters,
	//	and pointing the access point to the stdin
	fgets(userInput, MAX_INPUT_LENGTH, stdin);

	// this is required to use signals and not get a segfault
	if (strcmp(userInput, "\0") == 0) {
		return NULL;
	}

	// check to see if the input is just \n
	else if (strcmp(userInput, "\n") == 0) {
		return userInput;
	}

	// remove the '\n' character from the command line input
	strSize = strlen(userInput);
	userInput[strSize - 1] = '\0';

	return userInput;
}

// Function takes the user struct and determines if there is input redirect
//	output redirect or both. The folders are prepped and sent out.
void handleRedirect(struct userCmds* commands) {
	int outputOpen;
	int inputOpen;
	int dupOutput;
	int dupInput;

	// check to see if there is a file to input
	if (commands->fInput != NULL) {

		// if there is, input should be opened for reading only
		inputOpen = open(commands->fInput, O_RDONLY);

		// if there is an error opening the file, say so
		if (inputOpen == -1) {
			perror("Cannot open input file\n\n");
			fflush(stdout);
			exit(1);
		}

		// redirect the input, 0 allows that to come in
		dupInput = dup2(inputOpen, 0);

		if (dupInput == -1) {
			perror("Something went wrong!\n\n");
			fflush(stdout);
			exit(1);
		}

		close(inputOpen);
	}

	if (commands->fOutput != NULL) {
		// output file is opened for writing only, is truncated if it already exists,
		//     or created if it doesn't exist
		outputOpen = open(commands->fOutput, O_WRONLY | O_CREAT | O_TRUNC, 0777);

		// error message
		if (outputOpen == -1) {
			perror("Cannot open output file\n\n");
			fflush(stdout);
			exit(1);
		}

		// redirect the output, 1 allows for that to go out
		dupOutput = dup2(outputOpen, 1);

		if (dupOutput == -1) {
			perror("Something went wrong!\n\n");
			fflush(stdout);
			exit(1);
		}

		close(outputOpen);
	}
}

// If the user inputs & then this will process in the background.
void backgroundRedirect(void) {

		// devNullStatus is the Linux trash can
		int devNullStatus = open("/dev/null", O_WRONLY);

		// error message (unlikely to happen)
		if (devNullStatus == -1) {
			perror("could not open /dev/null for some reason\n");
			exit(2);
		}

		// this redirects all output to dev/null. 1 is the code for
		//	output redirect
		int dumpOut = dup2(devNullStatus, 1);

		// error message
		if (dumpOut == -1) {
			perror("something went wrong, we could not get rid of STDOUT!!\n\n");
			exit(2);
		}

		// Dev null is the perfect trash can
		int stdInDump = open("/dev/null", O_RDONLY);

		// error message
		if (stdInDump == -1) {
			perror("source open()");
			exit(2);
		}

		// redirect stdin to /dev/null
		int dumpIn = dup2(stdInDump, 0);

		// error message
		if (dumpIn == -1) {
			perror("something went wrong, we could not get rid of STDIN!!\n\n");
			exit(2);
		}
}


void inBackground(int finishedPid, int finishedStatus) {

	// loop through each pid in the array
	for (int i = 0; i < MAX_INPUT_LENGTH; i++) {

		// check to see if the item has finished
		if (finishedPid != 0 && pidHolder[i] == finishedPid) {

			if (WIFEXITED(finishedStatus) != 0) {
				printf("Background process %d exited with status %d\n", finishedPid, WEXITSTATUS(finishedStatus));
			}

			else if (WIFSIGNALED(finishedStatus) != 0) {
				printf("Background process %d exited with signal %d\n", finishedPid, WTERMSIG(finishedStatus));
			}
		}
	}
}

// 
void pidExitCheck (void) {
	int finishedPid;
	int finishedStatus;

	// waiting for any process to finish
	finishedPid = waitpid(-1, &finishedStatus, WNOHANG);

	inBackground(finishedPid, finishedStatus);	
}

/*
theForkParty is modified from explorations
*/
void theForkParty(struct userCmds* cmdStruct, struct sigaction childSignal) {

	// -5 is simply a holder value
	pid_t childPid = -5;
	pid_t spawnpid = -5;
	int j = 0;			// used to get to next item in pidHolder, which is a global variable

	int pidWait;

	// execStatus will catch the status of the execvp function
	//	if this is a -1, then there was an error
	int execStatus;


	// If fork is successful, the value of spawnpid will be 0 in the child, the child's pid in the parent
	spawnpid = fork();
	switch (spawnpid) {
	case -1:
		// Code in this branch will be exected by the parent when fork() fails and the creation of child process fails as well
		perror("fork() failed!");

		// flushing as often as possible :)
		fflush(stdout);
		exit(1);
		break;

	// This is the child process
	case 0:
		
		// if this is a foreground process, allow ^C to terminate it.
		if (cmdStruct->background != true) {
			// resetting the default for child
			childSignal.sa_handler = SIG_DFL;

			// activating the new handler
			sigaction(SIGINT, &childSignal, NULL);
		}

		// if there is an input or output file, redirect the input/output
		if (cmdStruct->ioRedirect) {
			handleRedirect(cmdStruct);
		}

		// if this is a background process
		else if (cmdStruct->background) {
			backgroundRedirect();
		}
		
		// execute the request from the user
		execStatus = execvp(cmdStruct->arrayOfArgs[0], cmdStruct->arrayOfArgs);

		if (execStatus == -1) {
			// if the return of execvp is -1, there was an error -- show that to user
			perror("Command not found!!\n\n");
			exit(1);
		}
		break;

	// This is the parent process
	default:
		
		// if the user has requested that this be a background process
		if (cmdStruct->background) {

			// the child will execute but another child will be spawned without waiting
			childPid = waitpid(spawnpid, &pidWait, WNOHANG);

			// show the user what the current background PID is
			printf("\nbackground pid is %d\n", spawnpid);
			fflush(stdout);

			// store the background processes in an array and move to the next position of the array
			pidHolder[j] = spawnpid;
			j++;
		}

		else {
			childPid = waitpid(spawnpid, &pidWait, 0);

			// keep track of the last executed foreground process
			lastForeground = pidWait;
			handle_SIGINT(pidWait);
		}

		// wait for any process to finish -- if it finishes and returns, it will compare to the array and will print the exit status for it and remove it from the array
		pidExitCheck();

		break;
	}
	
}

// if a user types ^Z, this will toggle
//	Foreground only mode
void handle_SIGTSTP(int handle) {
	char* message1 = "\nForeground mode is ACTIVE\n";
	char* message2 = "\nForeground mode is INACTIVE\n";

	if (foregroundOnly == false) {
		foregroundOnly = true;
		write(STDOUT_FILENO, message1, strlen(message1));
	}

	else {
		foregroundOnly = false;
		write(STDOUT_FILENO, message2, strlen(message2));
	}
}

// terminates a child process (called in the child fork)
//	and executes the child process
void handle_SIGINT(int status) {
	char* message1 = "\nTerminated by signal ";
	char* num = malloc(12);
	sprintf(num, "%d\n", WTERMSIG(status));

	// We are using write rather than printf
	if (WIFSIGNALED(status) != 0 && status == SIGINT) {
		write(STDOUT_FILENO, message1, strlen(message1));
		write(STDOUT_FILENO, num, strlen(num));
	}
}

int main()
{
	char* userArgs = malloc(MAX_INPUT_LENGTH);
	char poundAscii[5];

	// initialize pid array to all 0's
	for (int q = 0; q < MAX_INPUT_LENGTH; q++) {
		pidHolder[q] = 0;
	}

	// ***************************************************
	//	Signal Handler that modifies ^C's functionality
	// ***************************************************
	// adapted from lecture
	// initialize empty struct
	struct sigaction SIGINT_action = { 0 };
	
	// Fill out the SIGINT_action struct
	// Register handle_SIGINT as the signal handler
	SIGINT_action.sa_handler = SIG_IGN;

	// Block all catchable signals while handle_SIGINT is running
	sigfillset(&SIGINT_action.sa_mask);
	
	// No flags set
	SIGINT_action.sa_flags = 0;

	// Install our signal handler
	sigaction(SIGINT, &SIGINT_action, NULL);

	// ***************************************************
	//	Signal Handler that modifies ^Z's functionality
	// ***************************************************
	// initialize empty struct
	struct sigaction SIGTSTP_action = { 0 };

	// Fill out the SIGTSTP struct
	// Register handle_SIGTSTP as the signal handler
	SIGTSTP_action.sa_handler = handle_SIGTSTP;

	// Block all catchable signals while handle_SIGTSTP is running
	sigfillset(&SIGTSTP_action.sa_mask);

	// No flags set
	SIGTSTP_action.sa_flags = 0;

	// Install the signal handler
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);


	while (true) {

		// print the : prompt to the screen and flush it out
		printf(": ");
		fflush(stdout);

		// must be passed to convert to integer
		userArgs = getUserInput();

		// this will handle the case where ^Z is called
		if (userArgs == NULL) {
			continue;
		}

		// if the string is an empty line, go back to get input
		if (strcmp(userArgs, "\n") == 0) {
			continue;
		}

		// convert the first character to an ASCII character
		//	if it is the symbol '#' then this is a comment
		sprintf(poundAscii, "%c", userArgs[0]);

		// if the first character is a #, treat it as a comment
		if (strcmp(poundAscii, "#") == 0) {
			continue;
		}

		// create a struct of the user input if it is not an empty line
		//	or if it doesn't start with #
		struct userCmds* cmdStruct = cmdLine(userArgs);

		// Changing of directory
		if (strcmp(cmdStruct->arrayOfArgs[0], "cd") == 0) {

			// this will return to the home directory of the user
			if (cmdStruct->arrayOfArgs[1] == NULL) {
				chdir(getenv("HOME"));
			}

			// otherwise, this will go to the stated directory
			else {
				chdir(cmdStruct->arrayOfArgs[1]);
			}
		}

		// handles the argument "status"
		else if (strcmp(cmdStruct->arrayOfArgs[0], "status") == 0) {

			// if foreground process was exited normally
			if (WIFEXITED(lastForeground) != 0) {
				printf("Last foreground process exited with status %d\n", WEXITSTATUS(lastForeground));
			}

			// if foreground process was terminated by user
			else if (WIFSIGNALED(lastForeground) != 0) {
				printf("Last foreground process exited with signal %d\n", WTERMSIG(lastForeground));
			}
		}

		// if the user types in "exit" this will kill all processes and exit the shell.
		else if (strcmp(userArgs, "exit") == 0) {
			kill(0, SIGKILL);
			break;
		}

		// fork a new process
		else {
			theForkParty(cmdStruct, SIGINT_action);
		}
	}
	free(userArgs);
	
    return EXIT_SUCCESS;
}
