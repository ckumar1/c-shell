#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define MAX_INPUT_LENGTH 512 //Max input 512 bytes
#define INTERACTIVE_MODE 0
#define BATCH_MODE 1
/* Functions */

/* function displayError
 * Writes the generic error message to stderr
 *
 */
void displayError() {
	// Display Error
	char error_message[30] = "An error has occurred\n";
	write(STDERR_FILENO, error_message, strlen(error_message));
}

/* function dumpline
 *  This function reads and dumps any remaining characters on the current input
 *  line of a file. fp is a pointer to the FILE to read characters from.
 */
void dumpline(FILE * fp) {
	int c;
	while ((c = fgetc(fp)) != '\n' && c != EOF)
		;
}

/* function parseCmdLn
 * 
 * Parses a string buffer ending in a line break. Removes the '\n' char
 * and tokenizes buffer into a string array
 *
 * Returns the number of tokens or items in the array
 */

int parseCmdLn(char inputBuffer[MAX_INPUT_LENGTH + 2],
		char* argTokens[MAX_INPUT_LENGTH / 2], int* bg_mode, int* redir_mode) {

	int numArgs = 0;

	int bufferLength = strlen(inputBuffer);
	// replace the '\n' char with '\0', truncating it by 1
	inputBuffer[--bufferLength] = '\0';


	// TODO Check for redir and background jobs

	// detect background jobs
	if (inputBuffer[bufferLength-1] == '&') {
		*bg_mode = TRUE;	// set background mode to true
		// replace the '&' char with '\0', truncating it by 1
		inputBuffer[--bufferLength] = '\0';

	}

	// Tokenize inputBuffer and store into arg array
	char* tok = strtok(inputBuffer, " ");
	while (tok != NULL ) {
		argTokens[numArgs] = strdup(tok);	// Copy each token into array
		numArgs++;	// Increment nTokens
		tok = strtok(NULL, " ");	// Get next token
	}
	return (numArgs);
}

void runCmd(char** argTokens[256], int bg_mode, int redir_mode) {

	// Creates a new process and executes the command
	int rc = fork();
	if (rc < 0) {	// Failure
		displayError();
		exit(1);
	}

	// Child uses exec to run a different program
	else if (rc == 0) {
		// Call execvp() to change the process
		execvp(argTokens[0], argTokens);
		// A sucessful call to execvp() never returns
		// Error Case: The command does not exist
		displayError();

	} // end child process

	// Parent waits for child to finish
	else {
		// TODO implement background tasks
		// wait() stops until any one child of this parent finishes
		int wc = wait(NULL );
	}
}

void execShell(FILE* inputStream, int inputMode) {

	char inputBuf[MAX_INPUT_LENGTH + 2];

	// read file line by line until EOF or error reading inputstream
	while (fgets(inputBuf, (MAX_INPUT_LENGTH + 2), inputStream)) {
		// check to make sure fgets returns valid input
		if (inputBuf != NULL ) {

			size_t buflen = strlen(inputBuf); // Number of characters read into buffer not inc null term.

			// Prints the command back in batch mode
			if (inputMode == BATCH_MODE)
				write(STDOUT_FILENO, inputBuf, buflen);

			// newline char index, if it exists
			size_t newlinePos = buflen - 1;
			if (inputBuf[newlinePos] == '\n') {	// input ends with ('\n')Acceptable length

				char* argTokens[256] = { NULL }; // cmd line args

				int bg_mode = 0;
				int redir_mode = 0;

				// Tokenize inputBuf and store into arg array
				parseCmdLn(inputBuf, argTokens, &bg_mode, &redir_mode);

//				printf("Successful background read!");

				// TODO built-in commands
				if (strcmp("exit", argTokens[0]) == 0) {
					exit(0);
				}

				// Create a new process to run the command
				// Parent waits until it exits if not background mode
				runCmd(&argTokens, &bg_mode, &redir_mode);

			} else { //Line does not terminate with '\n'
				if (buflen + 1 == sizeof inputBuf) { // Too long input line

					// Display Error
					displayError();

					// Flush input stream to get rid of trailing new line character
					dumpline(inputStream);

					// write a newline character to stdout
					FILE* outStream = stdout;
					putc("\n", outStream); // TODO Redirection fix

					if (inputMode == INTERACTIVE_MODE) {
						printf("mysh> ");
					}
					// restart from the top of the while loop
					continue;

				} else { // EOF reached before line break
					puts("EOF reached before line break (3.1.)"); /* shouldn't happen */
				}
			}

		}

	}
}

int main(int argc, char *argv[]) {

	if (argc == 1) {  // if no arguments are specified run in Interactive mode
		// prints initial prompt
		printf("mysh> ");
		// Interactive loop to keep asking user for input
		execShell(stdin, 0);

	} else if (argc == 2) {  // batch mode in case of arguments
		// replace stdin stream with batchFile stream
		FILE *batchFile = freopen(argv[1], "r", stdin);
		if (batchFile == NULL ) {	// unable to open filename
			displayError();
			exit(1);
		}
		execShell(batchFile, BATCH_MODE);
		exit(0);

	} else { // invalid number of args (n>2)
		displayError();
		//exit gracefully
		exit(EXIT_FAILURE);
	}
	return (0);
}

