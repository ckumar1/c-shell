#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define MAX_INPUT_LENGTH 512
/* Functions */


int main(int argc, char *argv[]) {

	if (argc == 1) {  //if no arguments are specified run in interactive mode

		// Interactive loop to keep asking user for input

		// Repeated until the user types "exit"

		while (TRUE) {

			// Input variables


			//string to store User input (max size is 512 bytes)
			char input[512];

			// prints a prompt
			printf("mysh> ");

			// Get a newline terminated string from stdin
			gets(input);

			// TESTING print out input (what user just entered)
			printf("\ntest:: %s\n", input);

			// Parses command input
			// strtok()


			// Runs the appropriate type of command

			// TODO executes the command specified on that line of input,
			// TODO waits for the command to finish.

		}

	} else if (argc == 2) {  // batch mode in case of arguments

		// check if batch file is valid by opening
		// fopen()


		// get line of input from file
		// fgets()

		return (0);

	} else // Incorrect number of arguments
	{

		// Print error message to stderr
		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
		//exit gracefully
		exit(EXIT_FAILURE);

	}
}

