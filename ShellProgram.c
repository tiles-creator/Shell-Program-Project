/* COSC 4302 Operating Systems
 * Instructor: Dr. Bo Sun
 * Project: Shell Program
 * @authors: Teresa Iles, Eli Peveto, Sai Terapalli
 * Date: 11/30/2021
 *
 * Description:
 * This program is a shell application that allows the user to enter commands for the
 * operating system to execute.  The program requirements, details, outline, and some
 * code for this program are provided in the textbook "Operating Systems, 3rd Edition"
 * by Gary Nutt, pages 76-82, and 61-65).
 *
 * In order to run this program, first compile it by typing: gcc ShellProgram -o ShellProgram
 * Then to run the program, type: ./ShellProgram
 *
 * A command prompt will appear - it is the $ sign.
 * At this prompt, you can type in a command.
 * The program will create a child process to execute the command.
 * When you are finished running commands, type 'q' or 'Q', to exit the shell program.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define LINE_LEN        80
#define MAX_ARGS        64
#define MAX_ARG_LEN     16
#define MAX_PATHS       64
#define MAX_PATH_LEN    96
#define WHITESPACE      " ,\t\n"

#ifndef NULL
#define NULL '\0'
#endif

/* a struct to hold the command information:
   command name and arguments */
struct command_t {
    char *name;   // command name
    int* argc;    // number of arguments
    char *argv[MAX_ARGS];   // array of arguments
};

/* Print the prompt for the user */
void printPrompt() {
    /* Simple prompt */
    char *promptString = "$";
    printf("\n%s", promptString);
}

char *lookupPath(char *argv, char **dir) {
    /* This function searches the directories identified by the dir argument (the different paths)
     * to see if argv (the file name) appears there in a path.
     * Allocate a new string, place the full path name in it,
     * then return the string.
     */

     char pathName[MAX_PATH_LEN];

    // Check to see if file name is already an absolute path name.  If so, return filename (argv).
    if (argv[0] == '/') {
        if(access(argv, F_OK) == 0) {  //using access this checks if file name argv exists with F_OK
             return argv;
        }
        else { //this file does not exist so return an error
             printf("Error:  %s: command not found\n", argv);
            return NULL;
        }
    }

   if (strcmp(argv, "cd") == 0)  { //deals with the edge case of a traditional cd command
            return "cd";
    }
     //Look in PATH directories
     //Use access() to see if the file is in a directory
     int i;

     for (i=0; i < MAX_PATHS; i++) {
         if(dir[i] == NULL) {break;} //breaks out if its done looking through the directory

        char *temporary =  (char *) malloc(100); //frees up temporary space to make a test file location
        strcpy(temporary, dir[i]);  //adds the next directory word to the temporary variable
        strcat(temporary, "/");  //appends / to the end like directory files do to separate
        strcat(temporary, argv);  //appends the file name to the end

        if(access(temporary, F_OK) == 0) {  //checks if this test file temporary exists, and if it does return it
            return temporary;
        }
        else {
            free(temporary);  //frees up the memorary from malloc for the next loop
        }
     }

     printf("Error: %s: command not found\n", argv); // if file is not found in any directory
     return NULL;
    }

int parseCommand(char *cLine, struct command_t *cmd) {
    /* This function separates the parts of the command into
       the command name (also called filename) and the command's
       arguments.  The arguments are stored in an array
       We use a struct data type to hold the command information */
    int argc;
    char *token = (char *) malloc(1024);
    argc = 0;
    token = (char *) strtok(cLine, WHITESPACE);      // separate first string in command line

    /* This loop separates out the next string and points to*/
    while (token != NULL) {
        cmd->argv[argc] = (char *) malloc(MAX_ARG_LEN);  // allocate memory for argument
        strcpy(cmd->argv[argc], token);   // copy token to cmd.argv[argc]
        argc=argc+1;
        token = (char *) strtok(NULL, WHITESPACE);
    }

    /* Set the command name and argc */
    cmd->argc = (int*)malloc(sizeof(int));
    argc = argc-1;
    cmd->argc = (int*)argc;
    cmd->name = (char *)malloc(sizeof(cmd->argv[0]));
    strcpy(cmd->name, cmd->argv[0]);

    /* debugging code */
    /*printf("Command Name: %s\n", cmd->name);
    printf("Number of command args: %d\n", (int *)cmd->argc);
    int i;
    if ((int*)cmd->argc >= 1) {
    	for (i=1; i <= argc; i++)
    	{
        	char * argument = cmd->argv[i];
        	printf("Args:\n");
        	printf("arg[%d]: %s\n", i, argument);
        }
    } */
    return 1;
}

int parsePath(char *dirs[]) {

    char *pathEnvVar;
    int i;

    // Initialize dirs[i] array
    for (i=0; i<MAX_ARGS; i++) {
      dirs[i] = NULL;
    }

    // Get the path environment variable from the system
    pathEnvVar = (char *) getenv("PATH");

    /* debugging code: print thePath*/
    //printf("The path is ***** %s ****\n", pathEnvVar);

    //strtok(string,"delimiter"); function that delimits a string
    // Resource used:  CodeVault. "How to split strings in C (strtok)" YouTube, https://youtu.be/34DnZ2ewyZo, Jan 24, 2019.  Accessed Nov 19, 2021
    dirs[0] = (char *) strtok(pathEnvVar,":");
    int count = 0;

    //While loop to parse the entire path string
    //strtok(string,"delimiter") returns NULL when there is nothing more to read
    while (dirs[count] != NULL) {
        count++;
        dirs[count] = strtok(NULL,":");
    }

    /* debugging code: print the directories that make up the array dirs[] */
    /*for (i=0; i < count; i++) {
       printf("%s\n", dirs[i]);
    }*/

    return 1;
}

int main() {
    /* ... */
    /* Shell initialization */
    char *pathv[MAX_PATHS];
    char **pathv2=malloc(sizeof(pathv));
    char *commandLine = malloc(1024);
    char *fileToFind = malloc(1024);
    char buffer1[1024];

    //struct command_t empty - use this to reset values in command_t command for each new loop iteration
    struct command_t empty;
    empty.name = NULL;
    empty.argc = 0;
    int i2;
    for (i2 = 0; i2 < MAX_ARGS; i2++) {
        empty.argv[i2]=NULL;
    }


    /*Shell initialization */
    parsePath(pathv);  /* Get directory paths from PATH */
    printf("You have started the minishell program.  At the command prompt, enter 'q' or 'Q' to quit.\n");
    int runMiniShell = 1;

    while (runMiniShell) {

        /* Print minishell command prompt */
        printPrompt();
        struct command_t command = empty;
        /* Read the command line */
        fgets(buffer1, 1024, stdin);
        commandLine = buffer1;

        /* If user hits return with no input, do nothing and show another prompt */
        if (commandLine[0] == 10) {
            continue;
        }

	if (commandLine[0] == 'q' || commandLine[0] == 'Q')  {
            runMiniShell=0;
            break;
        }

        /* Debugging code */
        //printf("This is what was typed: %s\n", commandLine);  // debugging code

        /* This function parses the command line and stores in struct command_t command */
        parseCommand(commandLine, &command);

        pathv2 = &pathv;
        fileToFind = command.name;

        /* Get the full pathname for the file */
        command.name = lookupPath(fileToFind, pathv2);

        if (command.name == NULL) {
              printf("Error:  No file specified. Try again.\n");
              continue;
        }

        /* Create child and execute the command */
        pid_t theChild;
        theChild=fork();
        if (theChild==0) {
            execv(command.name, command.argv);
            exit(0);
        }

        /* Wait for the child to terminate */
        wait(NULL);

        /* debugging code */
        /* printf("\nCommand was executed!"); */

   }

    /* Shell termination */
    if (runMiniShell==0) {
        printf("\nYour minishell session has ended.\n");
    }

}
