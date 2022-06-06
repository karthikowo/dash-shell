#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void dash_loop();			   // called by the main
int dash_execute(char **args); // called by the loop
char *dash_read_line();		   // reading the line from the user
char **dash_split_line();	   // parsing the line into cmd and arguments
int dash_launch(char **args);  // called by execute
int dash_cd(char **args);	   // built-in command
int dash_help(char **args);	   // built-in command
int dash_exit(char **args);	   // built-in command

// main function
int main(int argc, char **argv)
{

	// the shell loop
	dash_loop();
	return EXIT_SUCCESS;
}

void dash_loop(void)
{
	char *line;	 // array of characters
	char **args; // array of character pointers
	int status;	 // status returned by dash_execute()

	do
	{
		printf("$ ");
		line = dash_read_line();
		args = dash_split_line(line);
		status = dash_execute(args);

		free(line);
		free(args);
	} while (status);
}

#define DASH_RL_BUFSIZE 1024
char *dash_read_line()
{
	int buffersize = DASH_RL_BUFSIZE;
	int position = 0;
	char *buffer = malloc(sizeof(char) * buffersize);
	int c;

	if (!buffer)
	{
		// if buffer allocation fails
		fprintf(stderr, "dash : allocation error\n");
		exit(EXIT_FAILURE);
	}

	while (1)
	{
		c = getchar();

		if (c == EOF || c == '\n')
		{
			buffer[position] = '\0';
			return buffer;
		}
		else
		{
			buffer[position] = c;
		}

		position++;

		// if buffer is full, reallocate
		if (position >= buffersize)
		{
			buffersize += DASH_RL_BUFSIZE;
			buffer = realloc(buffer, buffersize);
			if (!buffer)
			{
				fprintf(stderr, "dash: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

#define DASH_TOK_BUFSIZE 64
#define DASH_TOK_DELIM " \t\r\n\a"

char **dash_split_line(char *line)
{
	int buffersize = DASH_TOK_BUFSIZE, position = 0;
	char **tokens = malloc(buffersize * sizeof(char *));
	char *token;

	if (!tokens)
	{
		fprintf(stderr, "dash:allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, DASH_TOK_DELIM);

	while (token != NULL)
	{
		tokens[position] = token;
		position++;

		if (position >= buffersize)
		{
			buffersize += DASH_TOK_BUFSIZE;
			tokens = realloc(tokens, buffersize * sizeof(char *));

			if (!tokens)
			{
				buffersize += DASH_TOK_BUFSIZE;
				tokens = realloc(tokens, buffersize * sizeof(char *));
			}
		}

		token = strtok(NULL, DASH_TOK_DELIM);
	}
	tokens[position] = NULL;
	return tokens;
}

int dash_launch(char **args)
{
	pid_t pid, wpid;
	int status;

	pid = fork();
	if (pid == 0)
	{
		// Child process
		if (execvp(args[0], args) == -1) // execvp() returns -1 if it fails else the process is called with the args
		{
			perror("dash");
		}
		exit(EXIT_FAILURE);
	}
	else if (pid < 0)
	{
		// Error forking
		perror("dash");
	}
	else
	{
		// Parent process
		do
		{
			wpid = waitpid(pid, &status, WUNTRACED);		  // Wait for child process to terminate
		} while (!WIFEXITED(status) && !WIFSIGNALED(status)); // WIFEXITED AND WIFSIGNALED are the checks for EXITED OR SIGNALED processes
	}

	return 1;
}

// builtin_str is an array of builtin commands
char *builtin_str[] = {
	"cd",
	"help",
	"exit"};

// This function returns the index of the command in the builtin_str array
int (*builtin_func[])(char **) = {
	&dash_cd,
	&dash_help,
	&dash_exit};

// This function returns the number of builtin_str commands
int dash_num_builtins()
{
	return sizeof(builtin_str) / sizeof(char *);
}

// built_in command implementations
int dash_cd(char **args)
{
	if (args[1] == NULL)
	{
		fprintf(stderr, "dash: expected argument to \"cd\"\n");
	}
	else
	{
		if (chdir(args[1]) != 0) // chrdir() function is used for the implementation of the cd command
		{
			perror("dash");
		}
	}
	return 1;
}

int dash_help(char **args)
{
	// Nice display for the shell's help
	int i;
	printf("DASH - DEAD AGAIN SHELL\n");
	printf("Type program names and arguments, and hit enter.\n");
	printf("The following are built in:\n");

	for (i = 0; i < dash_num_builtins(); i++)
	{
		printf("  %s\n", builtin_str[i]);
	}
	return 1;
}

int dash_exit(char **args)
{
	// signal the exit of the terminal
	return 0;
}

int dash_execute(char **args)
{
	int i;
	if (args[0] == NULL)
	{
		// An empty command was entered.
		return 1;
	}

	for (i = 0; i < dash_num_builtins(); i++)
	{
		// Check if the command is a builtin command
		if (strcmp(args[0], builtin_str[i]) == 0)
		{
			// Call the built_in implementation
			return (*builtin_func[i])(args);
		}
	}
	// Execute the command through fork()
	return dash_launch(args);
}