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
	tok = strtok(inputString, " ");
	do {
		tokens[ntokens] = strdup(tok);	// Copy each token into array
		ntokens++; // increment number of tokens
		// Tokenize inputBuffer and store into arg array
		tok = strtok(NULL, " ");	// Get next token
	} while (tok != NULL );

	// add null terminating argument to argArray
	tokens[ntokens] = tok;

	// return number of arguments found
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
	int argCount = tokenize(inputBuffer, argTokens);
	// Return # of tokens in cmd input
	return (argCount);
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
	FILE* fnFile = freopen(filename, "w", stdout);

	int freopen_rc = close(STDOUT_FILENO);
	if (freopen_rc < 0) {
		displayError();
		return (-2);
	}
//	// Open a new file with the same fd as STDOUT_FILENO
//	int fd = open(filename, O_RDWR | O_TRUNC, S_IRWXU);
//	if (fd < 0) {
//		displayError();
//		perror("file descriptor won't open..");
//		return (-1);
//	}
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
			int wc = wait(NULL );
		}

		if (inputMode == INTERACTIVE_MODE) {
			printf("mysh> ");
		}

	}
}

/*function checkBuiltinCmds
 *
 * checks for built-in commands
 *
 * returns 1 if found and run, break the loop and reprompt
 * returns -1 if in an incorrect format, break the loop and reprompt
 * returns 0 if not builtin or python, continue loop
 */
int checkBuiltinCmds(char* argTokens[256]) {
	// built-in commands
	if (strcmp("exit", argTokens[0]) == 0) {	// exit program
		if (argTokens[1] == NULL ) {
			exit(0);
			return (-2);
		}

		else {	//bad exit
			displayError();
			return (-1);
		}
	} else if (strcmp("pwd", argTokens[0]) == 0) {	// print working directory

		if (argTokens[1] == NULL ) {	// no other args
			char* currDir = NULL;
			currDir = getcwd(currDir, 0);
			if (currDir) {
				write(STDOUT_FILENO,currDir, strlen(currDir));
				write(STDOUT_FILENO,"\n", 1);
				return (1);
			}
			displayError();
			return (-1);

		} else {	//bad pwd
			displayError();
			return (-1);
		}
	} else if (strcmp("cd", argTokens[0]) == 0) {	// cd

		if (argTokens[1] == NULL ) {	// no args
			chdir(getenv("HOME"));
			return(1);
		}
		// make sure there is only one arg
		else if (argTokens[2] == NULL ) {	// one arg

			if (chdir(argTokens[1]) == -1) {	// check if directory is valid
				displayError();
				return(-1);
			} else {	// bad cd: bad arg
				chdir(argTokens[1]);
				return (1);
			}

		} else {	// bad cd2: extra args
			displayError();
			return (-1);
		}

	} else if (strcmp("wait", argTokens[0]) == 0) {	// wait for all children to exit

		if (argTokens[1] == NULL ) { 	// check for extra args
			int status, pid;
			while ((pid = wait(&status)) != -1) {}	// wait for stuff
			return(0);
		} else {	// bad wait
			displayError();
			return (-1);
		}

	} else if (isPythonFile(argTokens[0])) {

		// replace arg0 after shifting the argTokens to the right
		runPython(argTokens);
	}

	// not a built-in command run regularly
	return (0);

}

int isPythonFile(const char* cmdString) {

	const char *period = strrchr(cmdString, '.');
	if (!period || period == cmdString)
		return (0);
	char* extension;
	extension = period + 1;

	if (strchr(extension, "py")) {
		return (1);
	} else {
		return (0);
	}
}

void runPython(char* cmdArgs[256]) {

	// run python interpreter /usr/bin/python
	// using the cmd args as parameters for it's parameters
	int size = 256;
	char* python = "/usr/bin/python";
	int c;
	// shift each argument over to make space
	// Note: loses the last argument
	for (c = size - 1; c >= 0 - 1; c--)
		cmdArgs[c + 1] = cmdArgs[c];

	// insert command to run python interpereter
	cmdArgs[0] = python;
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

	char inputBuf[MAX_INPUT_LENGTH + 2];

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

				// Built-in commands
				int builtin_rc = checkBuiltinCmds(argTokens);
			    if (builtin_rc == -2) {
					exit(0);
				}
				if (builtin_rc == 0)  // if builtin command not found or python
					// Create a new process to run the command
					runCmd(&argTokens, bg_mode, redir_mode, inputMode, redirFile);

			} else { //Line does not terminate with '\n'
				if (buflen + 1 == sizeof inputBuf) { // input line Too long
					displayError();

					// Flush input stream to get rid of trailing new line character
					dumpline(inputStream);

					// write a newline character to stdout start on a new line
					write(STDOUT_FILENO,"\n",1)

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

