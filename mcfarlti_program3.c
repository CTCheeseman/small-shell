// Tim McFarland
// ONID: 934066739
// CS 344
// Assignment 2
// Last Modified: 4/19/2021
// Due Date: 4/19/2021

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

// input length is counted 1 more than 2048 to account for the \n which must be removed
#define MAX_INPUT_LENGTH	2049
#define MAX_ARGS			512
#define	MAX_BGRND			300
#define OCTOTHORP			35		// this is the ASCII value of the symbol '#'

/* 
//    Parse each line which is comma separated and create
//    a struct of each movie.
//*/

/*
//* Return a linked list of movies by parsing data from
//* each line of the stated csv file
//*/


// struct for user command
struct userCmds
{
	char* arrayOfArgs[MAX_ARGS];	// holds all the arguments in an array
	char* fInput;					// holds the name of the input file
	char* fOutput;				// holds the name of the output file
	bool	background;				// determines if the item is a background process or not (determined by ending &)
	bool	ioRedirect;				// determines if there is I/O redirect needed (may not need)
};

// prototype declarations
struct userCmds* cmdLine(char* userInput);
char* moneyCheck(char* token);
char* expandTheMoney(char* varToExpand);
char* truncateMoney(char* varWithMoney);
void theForkParty(struct userCmds* cmdStruct);

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
		allUserCmds->fInput = calloc(strlen(token) + 1, sizeof(char));
		allUserCmds->fOutput = calloc(strlen(token) + 1, sizeof(char));
		char* expandedVar = calloc(strlen(token) + 1, sizeof(char));
		char* postMoney = calloc(strlen(token) + 1, sizeof(char));


		strcpy(postMoney, token);

		// check to see if there are any $$'s found and add to the array
		expandedVar = moneyCheck(postMoney);
		strcpy(allUserCmds->arrayOfArgs[j], expandedVar);

		//printf("array position %d is %s\n", j, allUserCmds->arrayOfArgs[j]);

		if (strcmp(token, ">") == 0) {

			// goes to the token afer the '>' and assigns that to the file name
			//	to be created
			token = strtok(NULL, " ");
			
			// this is a catch in case the last item in the argument is a > symbol
			//	so there is no segFault
			if (token == NULL) {
				printf("ERROR: You cannot end with the > command");
				continue;
			}

			strcpy(postMoney, token);
			expandedVar = moneyCheck(postMoney);

			// assigns output file
			strcpy(allUserCmds->fOutput, expandedVar);
			printf("the ouput file is: %s\n", allUserCmds->fOutput);

			// increment the array counter, allocate the memory for that location
			//	and copy that item to the array
			j++;
			allUserCmds->arrayOfArgs[j] = calloc(strlen(expandedVar) + 1, sizeof(char));

			// adds item to argument array
			strcpy(allUserCmds->arrayOfArgs[j], expandedVar);
		}

		else if (strcmp(token, "<") == 0) {

			// goes to the token afer the '<' and assigns that to the file name
			//	to be output to
			token = strtok(NULL, " ");

			// this is a catch in case the last item in the argument is a > symbol
			//	so there is no segFault
			if (token == NULL) {
				printf("ERROR: You cannot end with the < command");
				continue;
			}

			strcpy(postMoney, token);
			expandedVar = moneyCheck(postMoney);

			strcpy(allUserCmds->fInput, expandedVar);

			printf("the input file is: %s\n", allUserCmds->fInput);

			// increment the array counter, allocate the memory for that location
			//	and copy that item to the array
			j++;
			allUserCmds->arrayOfArgs[j] = calloc(strlen(expandedVar) + 1, sizeof(char));
			strcpy(allUserCmds->arrayOfArgs[j], expandedVar);
		}

		// moves to the next array position and the next item to be tokenized
		j++;
		token = strtok(NULL, " ");
	}

	// this checks to see if the last item in the input is the &
	//	if it is, then the user has requested a background process
	if (strcmp(allUserCmds->arrayOfArgs[j - 1], "&") == 0) {
		allUserCmds->background = true;
	}
	;
	// test statements
	printf("the last item in the array is: %s\n", allUserCmds->arrayOfArgs[j - 1]);

	printf("%s", allUserCmds->background ? "true" : "false");

	printf("The array you input is:\n");

	for (int k = 0; k < j; k++) {
		printf("%s ", allUserCmds->arrayOfArgs[k]);
	}

	printf("\n");
	// end of test statements

    return allUserCmds;
}

/*
function: truncateMoney

Input: varWithMoney

output: truncMoney

Purpose: This is a function that takes the first instance of the $ symbol, truncates it, and returns the string without any $
*/
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

char* expandTheMoney(char* varToExpand) {
	// counts instances of $
	int moneyCounter = 0;
	char* pidString = malloc(125);
	char* expansionHolder = malloc(MAX_INPUT_LENGTH);
	char* expansionFinalForm = malloc(MAX_INPUT_LENGTH);
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
char* moneyCheck(char *token) {
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

char* getUserInput() {
	char* userInput = calloc(MAX_INPUT_LENGTH, sizeof(char));
	int strSize;

	// show : as prompt for smallsh and flush it out before receiving input from user
	printf(": ");
	fflush(stdout);

	// using fgets to restrict the input to 2056 characters,
	//	and pointing the access point to the stdin
	fgets(userInput, MAX_INPUT_LENGTH, stdin);

	// check to see if the input is just \n
	if (strcmp(userInput, "\n") == 0) {
		return userInput;
	}

	// remove the '\n' character from the command line input
	strSize = strlen(userInput);
	userInput[strSize - 1] = '\0';

	return userInput;
}

/*
theForkParty is modified from explorations
*/
void theForkParty(struct userCmds* cmdStruct) {

	// -5 is simply a holder value
	pid_t childPid = -5;
	pid_t spawnpid = -5;

	int pidWait;

	// If fork is successful, the value of spawnpid will be 0 in the child, the child's pid in the parent
	spawnpid = fork();
	switch (spawnpid) {
	case -1:
		// Code in this branch will be exected by the parent when fork() fails and the creation of child process fails as well
		perror("fork() failed!");
		exit(1);
		break;
	case 0:

		if (cmdStruct->ioRedirect) {
			// setup IO, then exec
			// check and redirect the file
			// then exec
			// use dup2() read on the manpage
		}

		execvp(cmdStruct->arrayOfArgs[0], cmdStruct->arrayOfArgs);
			// there is redirect
		// spawnpid is 0. This means the child will execute the code in this branch
		
		// setup file redirection, and execute
			// file IO stuff and executing
			// open(psTwo);
				// if this is a -1, then this is a failure
				
			// dup2() redirects to a file descriptor
		// write function
			// takes user commands

			// checks for redirection

			// if redirection, open file for reading/writing and use dup2() to redirect to that file descriptor
				// hint: look at man pages

			// FIRST - open file
			// THEN use dup2 to redirect

			// if you have "ls &", you should have that sent to dev/null
				// If the user doesn't redirect the standard input for a background command, then standard input should be redirected to /dev/null
				// If the user doesn't redirect the standard output for a background command, then standard output should be redirected to /dev/null
				// THIS IS WHERE YOU WILL REDIRECT ANYTHING THAT DOESN'T HAVE AN INPUT OR OUTPUT FILE. use dup2() and send to /dev/null **********

		// you'll need to use open(file, 0700)

		// here, you do stuff with your signals

		break;
	default:
		// spawnpid is the pid of the child. This means the parent will execute the code in this branch

		// this is the standard wait process and this will be in the foreground
		if (cmdStruct->background) {
			// keep track of this with an array
			// that way you can kill the zombies of your background processes
			childPid = waitpid(spawnpid, &pidWait, WNOHANG);
		}

		else {
			childPid = waitpid(spawnpid, &pidWait, 0);
		}

		// !!!!!!!!!!!! this is to ensure that a background process occurs
		break;
	}
	
}

int main()
{
	char* userArgs = malloc(MAX_INPUT_LENGTH);
	char poundAscii[5];
	
	while (true) {

		// must be passed to convert to integer
		userArgs = getUserInput();

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
			printf("looks like you have a CD there bro\n\n");

			if (cmdStruct->arrayOfArgs[1] == NULL) {
				chdir(getenv("HOME"));
			}

			else {
				chdir(cmdStruct->arrayOfArgs[1]);
				//printf("The command is %s\n\n", cmdStruct->arrayOfArgs[1]);
			}
		}

		else if (strcmp(cmdStruct->arrayOfArgs[0], "status") == 0) {
			printf("status is here\n\n");
		}

		// if the user types in "exit" this will leave the program.
		else if (strcmp(userArgs, "exit") == 0) {
			// TO DO: End all processes
			break;
		}

		else {
			theForkParty(cmdStruct);

			// if background process is done - KILL IT!!!!!
		}
	}
	free(userArgs);
	
    return EXIT_SUCCESS;
}
