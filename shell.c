#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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

/* function tokenize
 *
 * Converts an inputString into an array of tokens separated by whitespace
 * Returns the number of tokens in the inputString
 */
int tokenize(char inputString[MAX_INPUT_LENGTH + 2],
		char* tokens[MAX_INPUT_LENGTH / 2]) {

	int ntokens = 0;
//	tokens = NULL;
	char* tok = NULL;
	do {
		// Tokenize inputBuffer and store into arg array
		tok = strtok(inputString, " ");

		tokens[ntokens] = strdup(tok);	// Copy each token into array

		ntokens++;	// Increment ntokens

		tok = strtok(NULL, " ");	// Get next token

	} while (tok != NULL );

	return (ntokens);
}

char* prepRedirInput(char* redirStr) {
	// Pointer to second half of string
	char* redir_output = NULL;
	redir_output = redirStr + 1;
	// size_t maxOutputSize = strlen(redir_output) + 1;
	// Truncate the command line section to r/m redir section
	redirStr[0] = '\0';

//	redirStr = &redir_output;
//	redirStr++;
	return (strdup(redir_output));

}

/* function checkRedir
 *
 * Detects if the command in the input buffer needs to be redirected.
 * Sets redir_mode accordingly
 *
 *returns 0 for when redir is Not found
 *returns 1 when redir is found, and correctly formatted
 *returns -1 when redir is found, but incorrectly formatted
 */
int checkRedir(char inputBuffer[MAX_INPUT_LENGTH + 2], int* redir_mode,
		char* outFile) {

	outFile = NULL;

	// check '>' exists in inputStream
	char* redirStr = NULL;
	redirStr = strchr(inputBuffer, '>');

	if (redirStr) {

		char* outputFileName;
		// Pointer to second half of string
		outputFileName = prepRedirInput(redirStr);
		//outputFileName = strdup(redirStr);
		// Token array to parse redir output into
		char* oTokens[strlen(outputFileName)];

		// validate that the output section is valid
		// by tokenizing output and counting # of tokens
		size_t numTokens = tokenize(outputFileName, oTokens);

		if (numTokens != 1) { // check if redir commmand is formatted correctly
			displayError();
			return (-1);
		} else {
			*redir_mode = TRUE;	// set redirect mode to true
			outFile = strdup(*oTokens);
			return (1);
		}

	}

	return (0); // no redirect found
}

/* function checkBgJob
 *
 * Detects if the command in the input bufferr is a background job.
 * Sets bg_mode accordingly
 *
 */
void checkBgJob(char inputBuffer[MAX_INPUT_LENGTH + 2], int* bg_mode) {
	// detect background jobs
	if (inputBuffer[strlen(inputBuffer) - 1] == '&') {
		*bg_mode = TRUE;	// set background mode to true
		// replace the '&' char with '\0', truncating it by 1
		inputBuffer[strlen(inputBuffer) - 1] = '\0';
	}
}

/* function parseCmdLn
 *
 * Parses a string buffer ending in a line break. Removes the '\n' char
 * and tokenizes buffer into a string array
 *
 * Returns the number of tokens or items in the array on success
 * Returns -1 if there was an error during parsing
 *
 */

int parseCmdLn(char inputBuffer[MAX_INPUT_LENGTH + 2],
		char* argTokens[MAX_INPUT_LENGTH / 2], int* bg_mode, int* redir_mode,
		char* redirFile) {

	//	int bufferLength = strlen(inputBuffer);
	// replace the '\n' char with '\0', truncating it by 1
	inputBuffer[strlen(inputBuffer) - 1] = '\0';

	// detect background jobs
	checkBgJob(inputBuffer, bg_mode);

	// check '>' exists in inputStream
	int redir_rc = checkRedir(inputBuffer, redir_mode, redirFile);

	if (redir_rc < 0) {	// error in redir format
		return (-1);
	}

	// Tokenize inputBuffer and store into arg array
	return (tokenize(inputBuffer, argTokens)); // Return # of tokens in cmd input
}

/* function redirectOutput
 *
 * Closes whatever stream is at STDOUT_FILENO and attempts to open
 * the given file
 * Returns 0 if file was successfully opened
 * Returns -1 if there was an error opening the redirect file
 * Returns -2 if there was an error closing stdout file descriptor
 *
 */
int redirectOutput(char* filename) {
	// Close the file descriptor associated with stdout
	int close_rc = close(STDOUT_FILENO);
	if (close_rc < 0) {
		displayError();
		return (-2);
	}
	// Open a new file with the same fd as STDOUT_FILENO
	int fd = open(filename, O_RDWR | O_TRUNC, S_IRWXU);
	if (fd < 0) {
		displayError();
		perror("file descriptor won't open..");
		return (-1);
	}
	return (0);
}

/* function runCmd
 *
 * runs the char** array in a child process.
 * bg_mode controls whether parent continues executing or waits.
 * Sets bg_mode accordingly
 * redir_mode redirects the output to the properly formatted file
 *
 * Will not complete if there is an error opening the file
 */
void runCmd(char** argTokens[256], int bg_mode, int redir_mode, int inputMode,
		char* redirFile) {

	// Creates a new process and executes the command
	int rc = fork();
	if (rc < 0) {	// Failure
		displayError();
		exit(1);
	}
	// Child uses execvp to run a different program
	else if (rc == 0) {

		// If redir_mode is set:
		// close the fd for stdout and open the redirected file
		if (redir_mode)
			redirectOutput(redirFile);

		execvp(argTokens[0], argTokens);
		// Error Case: The command does not exist
		displayError();

	} // end child process

	// Parent waits for child to finish if bg_mode is not sett
	else {
		if (bg_mode == FALSE) {
			int wc = wait(NULL ); // TODO build custom wait
		}

		if (inputMode == INTERACTIVE_MODE) {
			printf("mysh> ");
		}

	}
}

void checkBuiltinCmds(char* argTokens[256]) {
	// TODO built-in commands
	if (strcmp("exit", argTokens[0]) == 0) {
		exit(0);
	}
}

void printInteractivePrompt(int inputMode) {
	if (inputMode == INTERACTIVE_MODE) {
		printf("mysh> ");
	}
}

/* function execShell
 *
 * Executes commands in a inputStream
 * inputMode determines whether interactive or batch mode runs
 *
 */
void execShell(FILE* inputStream, int inputMode) {

	char inputBuf[MAX_INPUT_LENGTH + 2] ;

	// read file line by line until EOF or error reading inputstream
	while (fgets(inputBuf, (MAX_INPUT_LENGTH + 2), inputStream)) {
		// check to make sure fgets returns valid input
		if (inputBuf != NULL ) {

			size_t buflen = strlen(inputBuf); // Number of characters read into buffer (sans '\0')

			// Prints the command back in batch mode
			if (inputMode == BATCH_MODE)
				write(STDOUT_FILENO, inputBuf, buflen);

			// newline char index, if it exists
			if (inputBuf[buflen - 1] == '\n') {	// input ends with ('\n')Acceptable length

				char* argTokens[MAX_INPUT_LENGTH / 2] = { NULL }; // stores cmd line args

				// Flags to keep track of special modes
				int bg_mode = 0;
				int redir_mode = 0;
				char* redirFile = NULL;

				// Parse command line and store into argTokens[]
				// Also sets the various special flags and files
				int parse_rc = parseCmdLn(inputBuf, argTokens, &bg_mode,
						&redir_mode, redirFile);

				// If bad return value, restart loop to get new input
				if (parse_rc <= 0)  // redirection error or no input
					continue;

				// TODO built-in commands
				checkBuiltinCmds(argTokens);

				// Create a new process to run the command
				// Parent waits until it exits if not background mode
				runCmd(&argTokens, bg_mode, redir_mode, inputMode, redirFile);

			} else { //Line does not terminate with '\n'
				if (buflen + 1 == sizeof inputBuf) { // input line Too long
					displayError();

					// Flush input stream to get rid of trailing new line character
					dumpline(inputStream);

					// write a newline character to stdout start on a new line
					FILE* outStream = stdout;
					putc("\n", outStream);
					// prints "mysh> "
					printInteractivePrompt(inputMode);
					continue;

				} else { // EOF reached before line break
					break;
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

