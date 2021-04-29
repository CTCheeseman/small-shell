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

// prototype declarations
char* moneyCheck(char* token);
char* expandTheMoney(char* varToExpand);
char* truncateMoney(char* varWithMoney);


// struct for user command
struct userCmds
{
	char*	arrayOfArgs[MAX_ARGS];	// holds all the arguments in an array
	char*	fInput;					// holds the name of the input file
	char*	fOutput;				// holds the name of the output file
	bool	background;				// determines if the item is a background process or not (determined by ending &)
	bool	ioRedirect;				// determines if there is I/O redirect needed (may not need)
};

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
		char* moneyCatcher = calloc(strlen(token) + 1, sizeof(char));
		char* postMoney = calloc(strlen(token) + 1, sizeof(char));



		printf("this token is %s\n\n", token);

		strcpy(postMoney, token);

		printf("\npostMoney is: %s\n", postMoney);

		// check to see if there are any $'s found
		moneyCatcher = moneyCheck(postMoney);

		strcpy(allUserCmds->arrayOfArgs[j], token);

		//printf("array position %d is %s\n", j, allUserCmds->arrayOfArgs[j]);

		if (strcmp(token, ">") == 0) {

			// handle the case where '>' is the last item so it doesn't Segfault

			// goes to the token afer the '>' and assigns that to the file name
			//	to be created
			token = strtok(NULL, " ");
			
			// this is a catch in case the last item in the argument is a > symbol
			//	so there is no segFault
			if (token == NULL) {
				continue;
			}

			// assigns output file
			strcpy(allUserCmds->fOutput, token);
			printf("the ouput file is: %s\n", allUserCmds->fOutput);

			// increment the array counter, allocate the memory for that location
			//	and copy that item to the array
			j++;
			allUserCmds->arrayOfArgs[j] = calloc(strlen(token) + 1, sizeof(char));

			// adds item to argument array
			strcpy(allUserCmds->arrayOfArgs[j], token);
		}

		else if (strcmp(token, "<") == 0) {

			// handle the case where '<' isn't the last item so it doesn't Segfault

			// goes to the token afer the '<' and assigns that to the file name
			//	to be output to
			token = strtok(NULL, " ");

			// this is a catch in case the last item in the argument is a > symbol
			//	so there is no segFault
			if (token == NULL) {
				continue;
			}

			strcpy(allUserCmds->fInput, token);

			printf("the input file is: %s\n", allUserCmds->fInput);

			// increment the array counter, allocate the memory for that location
			//	and copy that item to the array
			j++;
			allUserCmds->arrayOfArgs[j] = calloc(strlen(token) + 1, sizeof(char));
			strcpy(allUserCmds->arrayOfArgs[j], token);
		}

		else if (strcmp(token, "&") == 0) {
			printf("Hey, look at that, you have an &\n");
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

	for (int i = 0; i < (strlen(varToExpand)); i++) {
		if (varToExpand[i] == '$') {
			moneyCounter++;
		}
	}
	
	// truncate all of the $ off of the variable
	expansionHolder = truncateMoney(varToExpand);
	
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

	sprintf(lastChar, "%c", token[strlen(token) - 1]);
	sprintf(nextToLastChar, "%c", token[strlen(token) - 2]);

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

int main()
{
	char* userArgs = malloc(MAX_INPUT_LENGTH);
	char poundAscii[5];

	printf("pid is %d", getpid());
	
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

		if (strcmp(poundAscii, "#") == 0) {
			continue;
		}

		// create a struct of the user input if it is not an empty line
		//	or if it doesn't start with #
		struct userCmds* catcher = cmdLine(userArgs);
				
		// if the user types in "exit" this will leave the program.
		if (strcmp(userArgs, "exit") == 0) {
			// TO DO: End all processes
			break;
		}
	}
	free(userArgs);
	//pid_t spawnpid = -5;
	//int intVal = 10;
	//// If fork is successful, the value of spawnpid will be 0 in the child, the child's pid in the parent
	//spawnpid = fork();
	//switch (spawnpid) {
	//case -1:
	//	// Code in this branch will be exected by the parent when fork() fails and the creation of child process fails as well
	//	perror("fork() failed!");
	//	exit(1);
	//	break;
	//case 0:
	//	// spawnpid is 0. This means the child will execute the code in this branch
	//	intVal = intVal + 1;
	//	printf("I am the child! intVal = %d\n", intVal);
	//	printf("The pid of the child is %d\n", getpid());
	//	break;
	//default:
	//	// spawnpid is the pid of the child. This means the parent will execute the code in this branch
	//	intVal = intVal - 1;
	//	printf("I am the parent! ten = %d\n", intVal);
	//	printf("The pid of the parent is %d\n", getpid());
	//	break;
	//}
	//printf("This will be executed by both of us!\n");

    return EXIT_SUCCESS;
}
