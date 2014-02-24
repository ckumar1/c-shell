#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define MAX_INPUT_LENGTH 512 //Max input 512 bytes
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
		char* argTokens[MAX_INPUT_LENGTH / 2]) {

	int numArgs = 0;

	int bufferLength = strlen(inputBuffer);
	// replace the '\n' char with '\0', truncating it by 1
	inputBuffer[bufferLength - 1] = '\0';

	// Tokenize inputBuffer and store into arg array
	char* tok = strtok(inputBuffer, " ");
	while (tok != NULL ) {
		argTokens[numArgs] = strdup(tok);	// Copy each token into array
		numArgs++;	// Increment nTokens
		tok = strtok(NULL, " ");	// Get next token
	}
	return (numArgs);
}

void runCmd(char** argTokens[256]) {

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

int main(int argc, char *argv[]) {

	if (argc == 1) {  // if no arguments are specified run in Interactive mode

		// Interactive loop to keep asking user for input
		while (TRUE) {

			// Input variables

			char inputBuf[MAX_INPUT_LENGTH + 2]; // input buffer (max size + '\n' char + '\0' char)
			size_t buflen; // Number of characters read into buffer not inc null term.

			// prints a prompt
			printf("mysh> ");

			// Get a newline terminated string from stdin

			// check to make sure fgets returns valid input
			if (fgets(inputBuf, (MAX_INPUT_LENGTH + 2), stdin)) {

				// buflen is the number of chars read including newline
				buflen = strlen(inputBuf);

				if (buflen > 0) {

					// expected index of newline char
					size_t newlinePos = buflen - 1;

					// input ends with a line break ('\n')
					if (inputBuf[newlinePos] == '\n') {	// Acceptable input

						// Parses command from input buffer
						char* argTokens[256] = { NULL }; // cmd line args
						// Tokenize inputBuf and store into arg array
						int tokCount = parseCmdLn(inputBuf, argTokens); // number of tokens
						// check for exit built-in command
						if (strcmp("exit", argTokens[0]) == 0) {
							exit(0);
						}


						// Creates a new process and executes the command
						runCmd(&argTokens);


					} else { //Line does not terminate with '\n'
						if (buflen + 1 == sizeof inputBuf) { // Too long input line
							// Display Error
							displayError();
							// Flush stdin
							dumpline(stdin);
							// write a newline character to stdout
							write(STDOUT_FILENO, '\n', 1);
							// restart from the top of the while loop
							continue;
						} else { // EOF reached before line break
							puts("EOF reached before line break (3.1.)"); /* shouldn't happen */
						}
					}
				} else {
					puts("EOF reached before line break (3.1.)"); /* shouldn't happen */
				}

			} else {
				if (feof(stdin)) { // unlikely to reach in interactive mode
					// puts("EOF reached (2.)");
					// Exit cleanly
					exit(0);
				} else {
					// puts("error in inputBuf (1.)");
					displayError();
				}
			}
		}

	} else if (argc == 2) {  // TODO batch mode in case of arguments

		// Alternate implementation with freopen()
		FILE *batchFile = freopen(argv[1], "r", stdin);
		if (batchFile == NULL ) {	// unable to open filename
			displayError();
			exit(1);
		}

		// Close the file descriptor associated with stdin
		// int close_rc = close(STDIN_FILENO);
		// if (close_rc < 0) {
		// 	perror("stdin closing stdin");
		// 	exit(1);
		// }
		// Open a new file
		// This new file will be assigned the smallest available descriptor, which
		// will equal STDIN_FILENO, which we made available using close()
		// FILE *batchFile = fopen(argv[1], "r");

		// Prints a prompt interactive mode
		// printf("mysh> ");

		char inputBuf[MAX_INPUT_LENGTH + 2]; // stores cmdline input (+ '\n' char + '\0' char)

		// read file line by line until EOF or error reading inputstream
		while (fgets(inputBuf, (MAX_INPUT_LENGTH + 2), batchFile)) {

			// check to make sure fgets returns valid input
			if (inputBuf != NULL ) {
				size_t buflen = strlen(inputBuf); // Number of characters read into buffer not inc null term.

				// Prints the command back in batch mode
				write(STDOUT_FILENO, inputBuf, buflen);

				// newline char index, if it exists
				size_t newlinePos = buflen - 1;
				if (inputBuf[newlinePos] == '\n') {	// input ends with ('\n')Acceptable length

					// Parses command from input buffer
					char* argTokens[256] = { NULL }; // cmd line args

					// Tokenize inputBuf and store into arg array
					int tokCount = parseCmdLn(inputBuf, argTokens);

					// check for exit built-in command
					if (strcmp("exit", argTokens[0]) == 0) {
						exit(0);
					}
					// Create a new process to run the command
					// Parent waits until it exits if not background mode
					runCmd(argTokens);

				} else { //Line does not terminate with '\n'
					if (buflen + 1 == sizeof inputBuf) { // Too long input line

						fwrite("\n", 2, stdout);
						// Display Error
						displayError();
						// Flush input stream
						dumpline(batchFile);
						// write a newline character to stdout
//						write(STDOUT_FILENO, '\n', 2);

						// restart from the top of the while loop
						continue;
					} else { // EOF reached before line break
						puts("EOF reached before line break (3.1.)"); /* shouldn't happen */
					}
				}

			}

		}
		// REACHED EOF
		// Exit cleanly
		exit(0);

	} else { // invalid number of args (n>2)
		displayError();
		//exit gracefully
		exit(EXIT_FAILURE);

	}
}

