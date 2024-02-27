#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <pwd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/stat.h>

//+
// File:    shell.c
//
// Pupose:  This program implements a simple shell program. It does not start
//      processes at this point in time. However, it will change directory
//      and list the contents of the current directory.
//
//      The commands are:
//         cd name -> change to directory name, print an error if the directory doesn't exist.
//                    If there is no parameter, then change to the home directory.
//         ls -> list the entries in the current directory.
//                If no arguments, then ignores entries starting with .
//                If -a then all entries
//         pwd -> print the current directory.
//         exit -> exit the shell (default exit value 0)
//              any argument must be numeric and is the exit value
//
//      if the command is not recognized an error is printed.
//-

#define CMD_BUFFSIZE 1024
#define MAXARGS 10

// Function to filter hidden files in scandir
int filterHidden(const struct dirent *entry) {
    // Filter out entries starting with a dot (hidden files)
    return (entry->d_name[0] != '.');
}

// Comparison function for alphasort
int alphaSort(const struct dirent **a, const struct dirent **b) {
    // Compare two dirent structures by their names
    return strcasecmp((*a)->d_name, (*b)->d_name);
}

int splitCommandLine(char *commandBuffer, char *args[], int maxargs);
int doInternalCommand(char *args[], int nargs);
int doProgram(char *args[], int nargs);

//+
// Function: main
//
// Purpose: The main function. Contains the read
//      eval print loop for the shell.
//
// Parameters: (none)
//
// Returns: integer (exit status of shell)
//-

int main() {
    char commandBuffer[CMD_BUFFSIZE];
    // Note the plus one, allows for an extra null
    char *args[MAXARGS+1];
    // Print prompt.. fflush is needed because
    // Stdout is line buffered, and won't
    // Write to terminal until newline
    printf("%%> ");
    fflush(stdout);
    while(fgets(commandBuffer,CMD_BUFFSIZE,stdin) != NULL){
        //printf("%s",commandBuffer);
        // Remove newline at end of buffer
        int cmdLen = strlen(commandBuffer);
        if (commandBuffer[cmdLen-1] == '\n'){
            commandBuffer[cmdLen-1] = '\0';
            cmdLen--;
            //printf("<%s>\n",commandBuffer);
        }

        // Split command line into words.(Step 2)

        // TODO
        int nargs = splitCommandLine(commandBuffer, args, MAXARGS);

        // Add a null to end of array (Step 2)

        // TODO
        args[nargs] = NULL;

        //Debugging
        // printf("%d\n", nargs);
        // int i;
        // for (i = 0; i < nargs; i++){
        //     printf("%d: %s\n",i,args[i]);
        // }
        // // Element just past nargs
        // printf("%d: %x\n",i, args[i]);

        // TODO: check if 1 or more args (Step 3)
        // TODO: if one or more args, call doInternalCommand  (Step 3)        
        // TODO: if doInternalCommand returns 0, call doProgram  (Step 4)
        // TODO: if doProgram returns 0, print error message (Step 3 & 4) that the command was not found.
        if (nargs > 0) {
            if (doInternalCommand(args, nargs) == 0) {
                if (doProgram(args, nargs) == 0) {
                    printf("Error: Command '%s' not found.\n", args[0]);
                }
            }
        }

        // Print prompt
        printf("%%> ");
        fflush(stdout);
    }
    return 0;
}

////////////////////////////// String Handling (Step 1) ///////////////////////////////////

//+
// Function: skipChar
//
// Purpose: This function skips over a given char in a string
//      For security, will not skip null chars.
//
// Parameters:
//    charPtr Pointer to string
//    skip character to skip
//
// Returns: Pointer to first character after skipped chars
//      ID function if the string doesn't start with skip,
//      or skip is the null character
//-

char * skipChar(char * charPtr, char skip){
    // TODO: contents of function
    if (charPtr != NULL) {
        while (*charPtr != '\0' && *charPtr == skip) {
            charPtr++;
        }
    // TODO: replace null with proper value
    return charPtr;
    }
    
}

//+
// Function: splitCommandLine
//
// Purpose: SplitCommandLine splits a command line input into individual arguments and stores
//          them in the arguments array. It also makes sure number of arguments <= max arguments parameter
//      
//
// Parameters:
//   commandBuffer (String representing the command line input to be split)
//   args (Array to store indiviudal arguements)
//   maxargs (Max number of arguements contained in args)
//
// Returns: Number of arguments stored in args, returns 0 if an error
//          occurs (ex: args > maxargs).
//-

int splitCommandLine(char * commandBuffer, char* args[], int maxargs){
    // TODO: contents of function
    int nargs = 0;
    char *token = commandBuffer;
    // Enter a loop
    do {
        // Skip leading spaces using external function
        token = skipChar(token, ' ');
        // Check if we've reached the end of the input string
        if (*token == '\0') {
            return nargs;
        }
        // Check if we've exceeded the maximum number of arguments
        if (nargs == maxargs) {
            fprintf(stderr, "Error: Too many arguments.\n");
            return 0;
        }
        // Store the current word in the args array and increment the argument count
        args[nargs++] = token;
        // Find the next space character (word separator).
        char *nextSpace = strchr(token, ' ');
        if (nextSpace == NULL){
            // If no more spaces are found, set token to the end of the string which will terminate the loop
            token = strchr(token, '\0');
        } else {
            // If a space is found, update the token to point to the next word and terminate the current word
            token = nextSpace;
            *token++ = '\0';
        }
    } while (*token != '\0');
    // TODO: reutrn proper value
    return nargs;
}

////////////////////////////// External Program  (Note this is step 4, complete doeInternalCommand first!!) ///////////////////////////////////

// List of directorys to check for command
// Terminated by null value
char * path[] = {
    ".",
    "/usr/bin",
    NULL
};

//+
// Function: doProgram
//
// Purpose: Searches for executable file that matches input, in the directories listed in the path array.
//      If executable is found, fork is used to create a child process and attempts to execute
//      the command using execv (takes path of file to execute and the arguements).
//
// Parameters:
//   args (Array containing the command and its arguments)
//   nargs (Number of arguments in args)
//
// Returns:
//   1 = found and executed the file
//   0 = could not find and execute the file
//-

int doProgram(char *args[], int nargs){
    // Find the executable
    // TODO: add body.
    // Note this is step 4, complete doInternalCommand first!!!
    struct stat status;
    char *cmd_path;
    int i = 0;
    int currentDirectory = 0;
    // Loop through the directories in the 'path' array to find the executable
    while (path[currentDirectory] != NULL) {
        // Allocate memory for the command path
        cmd_path = (char *) malloc(strlen(path[currentDirectory]) + strlen(args[0]) + strlen("/"));
        // Create the complete command path
        sprintf(cmd_path, path[currentDirectory]);
        sprintf(cmd_path + strlen(cmd_path), "/");
        sprintf(cmd_path + strlen(cmd_path), args[0]);
        // Check the status of the file
        if (stat(cmd_path, &status) == 0) {
            if (S_ISREG(status.st_mode)) {
                if (status.st_mode&S_IXUSR) {
                    // The file is a regular file and is executable
                    break;
                }
                else {
                    // The file is not executable
                    printf("Error: File found is not executable.\n");
                    return 0;
                }
            }
        }
        currentDirectory++;
    }
    // If cmd_path is still NULL, no executable was found
    if (cmd_path == NULL) {
        return 0;
    }
    // Fork a child process
    int processID = fork();
    if (processID == -1) {
        // Child process could not be created
        printf("Unable to create child process.\n");
        free(cmd_path);
        return 0;
    }
    else if (processID == 0) {
        // This code will be executed in the child process
        // Execute the command using execv
        execv(cmd_path, args);
    }
    else {
        wait(NULL);
    }
    free(cmd_path);
    return 1;
}

////////////////////////////// Internal Command Handling (Step 3) ///////////////////////////////////

///////////////////////////////
// Command Handling Functions //
///////////////////////////////

// TODO: a function for each command handling function
// goes here. Also make sure a comment block prefaces
// each of the command handling functions.

// Define command handling function pointer type
typedef void(*commandFunc)(char * args[], int nargs);

//+
// Function: exitFunc
//
// Purpose: Exits the shell
//
// Parameters:
//   args (Array containing the command and its arguments)
//   nargs (Number of arguments in args)
//
// Returns: (none)
//-

void exitFunc(char *args[], int nargs) {
    // Exits the system
    exit(0);
}

//+
// Function: pwdFunc
//
// Purpose: Retrieves and prints the current working directory.
//
// Parameters:
//   args (Array containing the command and its arguments)
//   nargs (Number of arguments in args)
//
// Returns: (none)
//-

void pwdFunc(char *args[], int nargs) {
    // Get the current working directory.
    char *cwd = getcwd(NULL, 0);
    // Check if getcwd() returned NULL (indicating an error).
    if (cwd == NULL) {
        printf("Error: Could not retrive working directory.\n");
    }
    // If getcwd() was successful, print the current working directory and free the allocated memory.
    else {
        printf("%s\n", cwd);
        free(cwd);
    }
}

//+
// Function: cdFunc
//
// Purpose: Changes the current working directory based on input. If only cd is entered with no second argument,
//          it changes the current directory to one before/above the current. If another arguement is provided
//          after cd it changes the current directory to the specified directory.
//
// Parameters:
//   args (Array containing the command and its arguments)
//   nargs (Number of arguments in args)
//
// Returns: (none)
//-

void cdFunc(char *args[], int nargs) {
    // New character pointer varible for storing start of string of directory
    char *newDirectory;
    // Check if user would like to return back a directory (the current directory is changed to the home directory), if only a single word is input (without a space)
    if (nargs == 1) {
        // Retrieve a pointer to the password file entry of a user
        struct passwd *pw = getpwuid(getuid());
        // Ensure the result is not NULL and print an error if the pw_dir field of the password struct cannout be used to retrive the home directory
        if (pw == NULL) {
            printf("Error: Unable to retrieve home directory.\n");
            return;
        }
        // Set new directory pointer varible to current directory if there are no errors
        newDirectory = pw->pw_dir;
    }
    // Check if user would like to move into a specified directory, if two words are input (separated by space)
    else {
        // Set new directory to the specified directory as the second argument of input (second word after the space), if that directory exists
        newDirectory = args[1];
    }
    // Print an error if the specified directory does not exist
    if (chdir(newDirectory) != 0) {
        printf("Error: Unable to locate the specified directory.\n");
        return;
    }
}

//+
// Function: lsFunc
//
// Purpose: This function lists the contents of the current working directory. If -a is used as the
//          second argument, it can show hidden files (files that begin with .).
//
// Parameters:
//   args (Array containing the command and its arguments)
//   nargs (Number of arguments in args)
//
// Returns: (none)
//-

void lsFunc(char *args[], int nargs) {
    // Flag to show hidden files (default: off)
    int showHidden = 0;
    // Ensure if two paramters are input, that an error message is printed, and the routine is returned at that point
    if (nargs > 1 && strcmp(args[1], "-a") != 0) {
        printf("Error: Invalid second input parameter (should be '-a').\n");
        return;
    }
    // Set hidden flag on if the second word (paramter after the space) is equal to -a
    if (nargs > 1 && strcmp(args[1], "-a") == 0) {
        showHidden = 1;
    }
    // Since scandir function is used to list the contents of a directory, and it is passed a pointer to a pointer to a pointer of the dirent structure (directory entity) and stores an array of pointers to dirents in the pointer, nameList is the pointer to pointers
    struct dirent **nameList;
    // Set numEnts to the number of entries in the array using the pointer to the list, determining whether to include hidden files (files that start with a dot .) in the directory listing, and alphabetically sorting them
    int numEnts = scandir(".", &nameList, showHidden ? NULL : filterHidden, alphaSort);
    // Return if there are no entries and thus not in a current directory
    if (numEnts < 0) {
        return;
    }
    // Print the directory
    for (int i = 0; i < numEnts; i++) {
        printf("%s\n", nameList[i]->d_name);
        free(nameList[i]);
    }
    free(nameList);
}

// Associate a command name with a command handling function
struct cmdStruct{
   char *cmdName;
   commandFunc cmdFunc;
};

// Prototypes for command handling functions
// TODO: add prototype for each comamand function
void exitFunc(char *args[], int nargs);
void pwdFunc(char *args[], int nargs);
void lsFunc(char *args[], int nargs);
void cdFunc(char *args[], int nargs);

// List commands and functions
// Must be terminated by {NULL, NULL} 
// In a real shell, this would be a hashtable.
struct cmdStruct commands[] = {
   // TODO: add entry for each command
   {"exit", exitFunc},
   {"pwd", pwdFunc},
   {"ls", lsFunc},
   {"cd", cdFunc},
   {NULL, NULL}     // Terminator
};

//+
// Function: doInternalCommand
//
// Purpose: Checks if the specified command input is an valid internal command by comparing the input 
//          to a predefined list of commands and their associated functions. If the input matches a predifned
//          command the corresponding function is called with the given arguments. 
//
// Parameters:
//   args (Array containing the command and its arguments)
//   nargs (Number of arguments in args)
//
// Returns:
//   1: If args[0] is an internal command and its corresponding function is executed.
//   0: If args[0] is not an internal command.
//-

int doInternalCommand(char * args[], int nargs){
    // TODO: function contents (step 3)
    int i = 0;
    while (commands[i].cmdName != NULL) {
        //printf("%d ", strcmp(commands[i].cmdName,args[0]));
        if (strcmp(commands[i].cmdName,args[0]) == 0) {
            commands[i].cmdFunc(args, nargs);
            return 1;
        }
        i++;
    }
    return 0;
}