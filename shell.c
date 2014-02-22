#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define MAX_INPUT_LENGTH 512 //Max input 512 bytes
void displayError() {
	// Display Error
	char error_message[30] = "An error has occurred\n";
	write(STDERR_FILENO, error_message, strlen(error_message));
}

/* Functions */

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

					size_t cmdlen = buflen - 1; // length of the cmd input w/o newline, index of newline char

					// input ends with a line break ('\n')
					if (inputBuf[cmdlen] == '\n') {	// Acceptable input

						// Parses command from inputBuf

						char* argTokens[256] = { NULL }; // stores tokens in array of char*
						int nTokens = 0;   // stores number of tokens

						// Copy command w/o '\n' from inputBuf
						char command[buflen];
						strncpy(command, inputBuf, cmdlen); // Copy input w/o trailing '\n' and '\0' chars
						command[cmdlen] = '\0'; // Add null term. char that was lost due to strncpy

						// Tokenize inputBuf and store into arg array
						char* tok = strtok(command, " ");
						while (tok != NULL ) {

							// Copy each token into array
							argTokens[nTokens] = strdup(tok);
							nTokens++; // Increment nTokens
							tok = strtok(NULL, " "); // Get next token

						} // Done tokenizing the command input into arguments

						// TODO implement whitespace formatting

						if (strcmp("exit", argTokens[0]) == 0) { // builtin command exit
							exit(0);
						} else if (strcmp("pwd", argTokens[0]) == 0) { // builtin command pwd
							// TODO call cwd
						} else if (strcmp("cd", argTokens[0]) == 0) {// builtin command cd
							// Check for arguments
							if (argTokens[1] == NULL ) {	// no-args
								// TODO change to path stored in the $HOME environment variable. Use getenv("HOME") to obtain this.
							} else { // NOTE check for more than one argument?
								// TODO run chdir with argTokens[1]
							}
						} else if (strcmp("wait", argTokens[0]) == 0) {	// builtin command wait
							// TODO Implement wait
						} // else if(){} // TODO Python handler

						// TODO Check command for redirection and background jobs
						// TODO executes the command specified on that line of inputBuf,
						// TODO waits for the command to finish.

					} else { //Line does not terminate with '\n'

						if (buflen + 1 == sizeof inputBuf) { // if buffer full then long input line

							// Display Error
							displayError();
							// Flush stdin
							int c;
							while ((c = getchar()) != '\n' && c != EOF);

							// Start from the top of the while loop
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

					// puts("EOF reached (2.)"); // TEST output*/
					//Exit cleanly
					exit(0);

				} else {
					puts("error in inputBuf (1.)");
				}
			}
		}

	} else if (argc == 2) {  // batch mode in case of arguments

		// check if batch file is valid by opening
		// fopen()

		// get line of input from file
		// fgets()

		return (0);

	} else // Incorrect number of arguments
	{

		displayError();
		//exit gracefully
		exit(EXIT_FAILURE);

	}
}

/* function dump_line
 *  This function reads and dumps any remaining characters on the current input
 *  line of a file.
 *  Parameter:
 *     fp - pointer to a FILE to read characters from
 *  Precondition:
 *     fp points to a open file
 *  Postcondition:
 *     the file referenced by fp is positioned at the end of the next line
 *     or the end of the file.
 */
//	void dump_line( FILE * fp )
//	{
//	  int ch;
//
//	  while( (ch = fgetc(fp)) != EOF && ch != '\n' )
//		/* null body */;
//	}
