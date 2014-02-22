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

void displayError() {
	// Display Error
	char error_message[30] = "An error has occurred\n";
	write(STDERR_FILENO, error_message, strlen(error_message));
}

/*
 * Parses a string buffer ending in a line break. Removes the '\n' char
 * and tokenizes buffer into a string array
 *
 *
 * @returns the number of tokens or items in the array
 *
 */


int parseCmdLn(char inputBuffer[MAX_INPUT_LENGTH + 2], char* argTokens[MAX_INPUT_LENGTH/2]) {

	int numArgs = 0;

	int bufferLength = strlen(inputBuffer);
	// replace the '\n' char with '\0', truncating it by 1
	inputBuffer[bufferLength-1 ] = '\0';

//	char commandLine[bufferLength];
//	// trim '\n' and '\0' from input buffer using strncpy
//	strncpy(commandLine, inputBuf, cmdlen);// Copy input w/o trailing '\n' and '\0' chars
//	commandLine[cmdlen] = '\0';// Add null term. char that was lost due to strncpy


	// Tokenize inputBuf and store into arg array
	char* tok = strtok(inputBuffer, " ");
	while (tok != NULL ) {
		argTokens[numArgs] = strdup(tok);	// Copy each token into array
		numArgs++;	// Increment nTokens
		tok = strtok(NULL, " ");	// Get next token
	}
	return (numArgs);
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

					// length of the cmd input w/o newline, index of newline char
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

						//Temporary built-in command implementation with if-else (exit working)

						//						if (strcmp("pwd", argTokens[0]) == 0) { // builtin command pwd
						//							// TODO implement cwd
						//						} else if (strcmp("cd", argTokens[0]) == 0) {// builtin command cd
						//							// Check for # of arguments
						//							if (argTokens[1] == NULL ) {	// no-args
						//								// TODO change to path stored in the $HOME environment variable. Use getenv("HOME") to obtain this.
						//							} else { // NOTE check for more than one argument?
						//								// TODO run chdir with argTokens[1]
						//							}
						//						} else if (strcmp("wait", argTokens[0]) == 0) {	// builtin command wait
						//							// TODO Implement wait
						//						} // else if(){} // TODO Python handler

						// TODO Check command for redirection and background jobs

						// Creates a new process and executes the command
						// Call fork()
						int rc = fork();

						// fork() creates two copies of this process
						// Both processes continue execution from *this point*

						// Failure
						if (rc < 0) {
							fprintf(stderr, "fork() failed\n");
							exit(1);
						}

						// Child uses exec to run a different program
						else if (rc == 0) {

							// TODO implement redirection
							//							// Close the file descriptor associated with stdout
							//							int close_rc = close(STDOUT_FILENO);
							//							if (close_rc < 0) {
							//								perror("close");
							//								exit(1);
							//							}
							//							// Open a new file
							//							// This new file will be assigned the smallest available descriptor, which
							//							// will equal STDOUT_FILENO, which we made available using close()
							//							int fd = open("redirected_output.txt",
							//									O_RDWR | O_TRUNC, S_IRWXU);
							//							if (fd < 0) {
							//								perror("open");
							//								exit(1);
							//							}

							// Call execvp() to change the process
							// First argument is name of program to run
							// Second argument is the pointer to the array of command line args
							execvp(argTokens[0], argTokens);

							// A sucessful call to execvp() never returns
							// TODO CASE: The command does not exist
							displayError();

						} // end child process

						// Parent waits for child to finish
						else {

							// TODO implement background jobs
							// By default, wait() stops until any one child of this parent finishes
							int wc = wait(NULL );

							printf(
									"I'm the parent of %d (PID = %d).  I waited until %d finished.\n",
									rc, (int) getpid(), wc);
						}

					} else { //Line does not terminate with '\n'
						if (buflen + 1 == sizeof inputBuf) { // if buffer full then long input line

							// Display Error
							displayError();
							// Flush stdin
							int c;
							while ((c = getchar()) != '\n' && c != EOF)
								;

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
