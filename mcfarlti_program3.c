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
#define MAX_INPUT_LENGTH 2049
#define MAX_ARGS 512
#define	MAX_BGRND 300

/* 
//    Parse each line which is comma separated and create
//    a struct of each movie.
//*/
//struct movie* createMovie(char* currLine)
//{
//
//    // the struct must have memory allocated for it
//    struct movie* currMovie = malloc(sizeof(struct movie));
//
//    char* saveptr;
//
//    // The first token is the title of the movie
//    char* token = strtok_r(currLine, ",", &saveptr);
//    currMovie->title = calloc(strlen(token) + 1, sizeof(char));
//    strcpy(currMovie->title, token);
//
//    // The second token is the year of the movie's release
//    token = strtok_r(NULL, ",", &saveptr);
//    int yearNum = atoi(token);
//    currMovie->year = yearNum;
//
//    // The third token is the languages
//    token = strtok_r(NULL, ",", &saveptr);
//    currMovie->languages = calloc(strlen(token) + 1, sizeof(char));
//    strcpy(currMovie->languages, token);
//
//    // The fourth token is the rating
//    token = strtok_r(NULL, "\n", &saveptr);
//    currMovie->rating = strtod(token, &saveptr);
//
//    // Set the next node to NULL
//    currMovie->next = NULL;
//
//    return currMovie;
//}
//
/*
//* Return a linked list of movies by parsing data from
//* each line of the stated csv file
//*/
//struct movie* processFile(char* filePath)
//{
//    // Open the csv file as a read
//    FILE* movieFile = fopen(filePath, "r");
//
//    char* currLine = NULL;
//    size_t len = 0;
//    ssize_t nread;
//
//    // The head of the linked list
//    struct movie* head = NULL;
//
//    // The tail of the linked list
//    struct movie* tail = NULL;
//
//    // Read the .csv file line by line
//    while ((nread = getline(&currLine, &len, movieFile)) != -1)
//    {
//        // Get a new movie node corresponding to the current line
//        struct movie* newNode = createMovie(currLine);
//
//        if (head == NULL)
//        {
//            // If this is the first node in the linked list
//            //  treat it as though it is the only item in the list
//            head = newNode;
//            tail = newNode;
//        }
//        else
//        {
//            // Otherwise, if this is not the first item in the linked list
//            // Add this node to the end of the list and advance it to the next item
//            tail->next = newNode;
//            tail = newNode;
//        }
//    }
//    free(currLine);
//    fclose(movieFile);
//    return head;
//}
//
//void outputMovieByYear(struct movie* list, int year, char* dirName)
//{
//    /*
//    outputMovieByYear has no return value. if the year is found in the .csv file, then a file is created
//        with the name YYYY.txt, and each of the movies that exist for that year are output to the file.
//        There will be n files made for n amount of years in the .csv file. All files are placed in the
//        dirName directory (which is accessible from the root) and each of the files will have permissions
//        of '-rw-r-----'
//    */
//    
//    int yearCounter = 0;
//    char* tempYearText = malloc(MAX_SIZE);
//    char* movieTitle = malloc(MAX_SIZE);
//    char* outputDir = malloc(MAX_SIZE);
//    char* fileLocation = malloc(MAX_SIZE);
//    FILE* fPointer;
//    char tempDirName[25];
//    char tempName[MAX_SIZE] = { "./" };
//
//    // find if the year has any movies in it -- if it does, have this year print out ONCE, then create a file with this name
//
//    while (list != NULL)
//    {
//
//        if (year == list->year)
//        {
//            // if the movie year matches the input, print the movie title
//            yearCounter++;
//
//            if (yearCounter == 1) {
//
//                // create it so 'directory_name' becomes './directory_name'
//                strcpy(tempDirName, dirName);
//                strcat(tempName, tempDirName);
//                strcpy(outputDir, tempName);
//
//                // change the directory to the new directory
//                chdir(outputDir);
//
//                // convert the year to a string, and add .txt to it
//                sprintf(tempYearText, "%d", year);
//
//                // concatenate the number to the directory prefix name
//                strcat(tempYearText, ".txt");
//                fPointer = fopen(tempYearText, "w");
//            }
//
//            strcpy(movieTitle, list->title);
//            strcat(movieTitle, "\n");
//            fprintf(fPointer, movieTitle);
//        }
//
//        // move to the next node in the list
//        list = list->next;
//    }
//
//    if (yearCounter > 0)
//    {
//        fclose(fPointer);
//
//        // change the most recent file to rw-r-----
//        chmod(tempYearText, 0640);
//
//        // return to the parent directory
//        chdir("..");
//    }
//    free(movieTitle);
//    free(tempYearText);
//    free(outputDir);
//    free(fileLocation);
//}
//
//void yearLooper(struct movie* list, char* dirName) {
//
//        /* 
//        iterates through a list of all years that movies have existed
//            calls printMovieByRating, which prints out the highest
//            rated movie for each year searched
//        */
//    
//        // 1888 is the first year a movie was created
//        // have a buffer to go through 2025
//        int firstMovie = 1885;
//        int currentYear = 2025;
//    
//        // print the highest rated movie that year
//        for (int eachYear = firstMovie; eachYear <= currentYear; eachYear++)
//        {
//            outputMovieByYear(list, eachYear, dirName);
//        }
//    
//}
//
//int isCsv(char* fileName) {
//    /*
//    Helper function that determines if the file extension is a .csv
//        worked directly with Michael Slater to develop this function
//    */
//    char* dot = strrchr(fileName, '.');
//
//    if (dot != NULL && (strcmp(dot, ".csv") == 0)) {
//        return 1;
//    }
//    return 0;
//}
//
//char* findSizeOfFiles(int choice) {
//    /*
//    Parts of this method is modified from the Directories Exploration from Module 3. 
//
//    This method goes through each file in the root directory and does several things. If the choice
//        is 1, this will find the largest .csv file in the root directory.
//        If the choice is 2, this will find the smallest .csv file in the root directory.
//        This will return the name of smallest or largest file based on the user choice.
//    */
//
//    // Open the current directory
//    DIR* currDir = opendir(".");
//    struct dirent* eachDir;
//    size_t fileSize;
//    struct stat dirStat;
//    char* fileName = malloc(MAX_SIZE);
//
//    // initialize fileSize
//    fileSize = 0;
//
//    // Go through all the entries
//    while ((eachDir = readdir(currDir)) != NULL) {
//
//        // checks to see if the name of the file starts with "movies_"
//        if ((strncmp(PREFIX, eachDir->d_name, strlen(PREFIX)) == 0) && isCsv(eachDir->d_name) == 1) {
//
//            stat(eachDir->d_name, &dirStat);
//
//            // this is for the first time that the files are passed
//            if (fileSize == 0) {
//                fileSize = dirStat.st_size;
//                memset(fileName, '\0', MAX_SIZE);
//                strcpy(fileName, eachDir->d_name);
//            }
//
//            // if the user wants to pick the largest file
//            if (choice == 1) {
//                // if the size of the file is larger than the previous, set the item
//                if (dirStat.st_size > fileSize) {
//                    fileSize = dirStat.st_size;
//                    memset(fileName, '\0', MAX_SIZE);
//                    strcpy(fileName, eachDir->d_name);
//                }
//            }
//
//            // if the user wants to pick the smallest file
//            else if (choice == 2) {
//                // if the size of the file is smaller than the previous, set the item
//                if (dirStat.st_size < fileSize) {
//                    fileSize = dirStat.st_size;
//                    memset(fileName, '\0', MAX_SIZE);
//                    strcpy(fileName, eachDir->d_name);
//                }
//            }
//        }
//    }
//
//    // Close the directory
//    closedir(currDir);
//   // printf("the name of the file is: %s and the size of the file is %d\n", fileName, fileSize);
//    return fileName;
//}
//
//char* generateDirectoryName (void) {
//
//    /*
//    This method generates a name for the directory by creating a random number,
//        concatenating that random number with "mcfarlti.movies." and then creating
//        that directory within the root folder. If the directory is created, then
//        the name of the directory is returned. Otherwise the word "FAILED" is returned
//    */
//
//    int randNum;
//    int dirResponse;
//    char numName[25];   
//    char* noSuccess = malloc(10);
//    char temporaryName[MAX_SIZE] = { "mcfarlti.movies." };
//    char* directoryName = malloc(MAX_SIZE);
//
//    noSuccess = "FAILED";
//
//    // seed a random number using current time
//    srand(time(0));
//
//    // restrict random number to 99,999 as max
//    randNum = (rand() % MAX_RAND);
//
//    // convert random number to a string
//    sprintf(numName, "%d", randNum);
//
//    // concatenate the number to the directory prefix name
//    strcat(temporaryName, numName);
//    strcpy(directoryName, temporaryName);
//
//    // attemps to make the directory, if this fails, -1 will be 
//    //  the return value of mkdir
//    dirResponse = mkdir(directoryName, 0750);
//
//    if (dirResponse == -1) {
//        return noSuccess;
//    }
//
//    return directoryName;
//}
//
//int verifyFileName(char* fileName) {
//
//    /*
//    This method checks to see if the file exists. If it does, 0 is returned
//        if the file is not found in the root folder, then -1 is returned
//    */
//
//    DIR* currDir = opendir(".");
//    struct dirent* eachDir;
//
//    // Go through all the entries
//    while ((eachDir = readdir(currDir)) != NULL) {
//
//        // checks to see the the name of the input is an existing file
//        if ((strncmp(fileName, eachDir->d_name, strlen(eachDir->d_name)) == 0)) {
//
//            closedir(currDir);
//            return 0;
//        }
//    }
//
//    closedir(currDir);
//    return -1;
//}
//
//int executeUserChoice(int choice, char* fileName) {
//    /*
//    This will only occur if the user choice is 1 or 2. If the user choice is 1,
//        The largest .csv file will be found. If it is 2, the smallest will be found
//        The directory is also created and all of the file is processed and 
//        output as .txt files from sub-methods called here.
//    */
//
//    char* nameOfFile = malloc(MAX_SIZE);
//    char* directoryName = malloc(MAX_SIZE);
//
//    // first, if the user choice is 3, check to see if file exists
//    if (choice == 3) {
//        if (verifyFileName(fileName) == 0) {
//            nameOfFile = fileName;
//        }
//        else if (verifyFileName(fileName) != 0) {
//            return -1;
//        }
//    }
//
//    // first, create the directory
//    directoryName = generateDirectoryName();
//
//    // if the directory nanme fails, try again
//    if (strcmp(directoryName, "FAILED") == 0) {
//        directoryName = generateDirectoryName();
//    }
//
//    if (choice == 1 || choice == 2) {
//        // find the name of the file to be processed
//        nameOfFile = findSizeOfFiles(choice);
//    }
//
//    // create the struct from the .csv that was found
//    struct movie* list = processFile(nameOfFile);
//
//    // loop through each year and create files for movies found
//    //  insert the files in the directory created
//    yearLooper(list, directoryName);
//
//    // show to the user that there was success
//    printf("\nNow processing the chosen file named: %s", nameOfFile);
//    printf("\nCreated directory with the name: %s\n", directoryName);
//
//    // unallocate nameOfFile if the file isn't reassigned to fileName
//    if (choice == 1 || choice == 2) {
//        free(nameOfFile);
//    }
//    free(directoryName);
//    return 0;
//}

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
		allUserCmds->arrayOfArgs[j] = calloc(strlen(token) + 1, sizeof(char));
		allUserCmds->fInput = calloc(strlen(token) + 1, sizeof(char));
		allUserCmds->fOutput = calloc(strlen(token) + 1, sizeof(char));
		strcpy(allUserCmds->arrayOfArgs[j], token);

		printf("array position %d is %s\n", j, allUserCmds->arrayOfArgs[j]);

		if (strcmp(token, ">") == 0) {

			// goes to the token afer the '>' and assigns that to the file name
			//	to be created
			token = strtok(NULL, " ");
			strcpy(allUserCmds->fInput, token);
			printf("the input file is: %s\n", allUserCmds->fInput);

			// increment the array counter, allocate the memory for that location
			//	and copy that item to the array
			j++;
			allUserCmds->arrayOfArgs[j] = calloc(strlen(token) + 1, sizeof(char));
			strcpy(allUserCmds->arrayOfArgs[j], token);
		}

		else if (strcmp(token, "<") == 0) {
			// goes to the token afer the '<' and assigns that to the file name
			//	to be output to
			token = strtok(NULL, " ");
			strcpy(allUserCmds->fOutput, token);

			printf("the output file is: %s\n", allUserCmds->fOutput);

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

	printf("The array you input is:\n");

	for (int k = 0; k < j; k++) {
		printf("%s ", allUserCmds->arrayOfArgs[k]);
	}

	printf("\n");

	// if the character '&' is the last item in the arguments
			// make this a background process
	
	// if > is in the arguments, and is surrounded 

	// if # is the first item in the input, do nothing

	// if the line is empty, do nothing

	// if < is found, then the next item in the array will be an input file

	// if > is found, then the next item in the array will be an output file
				
    // The second token is the year of the movie's release
    //token = strtok_r(NULL, ",", &saveptr);
    //int yearNum = atoi(token);
    //currMovie->year = yearNum;

    //// The third token is the languages
    //token = strtok_r(NULL, ",", &saveptr);
    //currMovie->languages = calloc(strlen(token) + 1, sizeof(char));
    //strcpy(currMovie->languages, token);

    //// The fourth token is the rating
    //token = strtok_r(NULL, "\n", &saveptr);
    //currMovie->rating = strtod(token, &saveptr);

    return allUserCmds;
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

	// remove the '\n' character from the command line input
	strSize = strlen(userInput);
	userInput[strSize - 1] = '\0';

	return userInput;
}

int main()
{
	char* userArgs = malloc(MAX_INPUT_LENGTH);
	
	while (true) {

		// must be passed to convert to integer
		userArgs = getUserInput();

		printf("the length is: %d\n", sizeof(userArgs));

		// create a struct of the user input
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
