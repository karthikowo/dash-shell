#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void dash_loop();
char *dash_read_line();
char **dash_split_line();
int dash_launch(char **args);
int dash_execute(char **args);

//  Function Declarations for builtin shell commands:

int dash_cd(char **args);
int dash_help(char **args);
int dash_exit(char **args);

int main(int argc, char **argv)
{
	// Load config files, if any.

	// Run command loop.
	dash_loop();

	// Perform any shutdown/cleanup.

	return EXIT_SUCCESS;
}

void dash_loop(void)
{
	char *line;
	char **args;
	int status;

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
				bufsize += DASH_TOK_BUFSIZE;
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
		if (execvp(args[0], args) == -1)
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
			wpid = waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;
}

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
	"cd",
	"help",
	"exit"};

int (*builtin_func[])(char **) = {
	&dash_cd,
	&dash_help,
	&dash_exit};

int dash_num_builtins()
{
	return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/
int dash_cd(char **args)
{
	if (args[1] == NULL)
	{
		fprintf(stderr, "dash: expected argument to \"cd\"\n");
	}
	else
	{
		if (chdir(args[1]) != 0)
		{
			perror("dash");
		}
	}
	return 1;
}

int dash_help(char **args)
{
	int i;
	printf("DASH - DEAD AGAIN SHELL\n");
	printf("Type program names and arguments, and hit enter.\n");
	printf("The following are built in:\n");

	for (i = 0; i < dash_num_builtins(); i++)
	{
		printf("  %s\n", builtin_str[i]);
	}

	printf("Use the man command for information on other programs.\n");
	return 1;
}

int dash_exit(char **args)
{
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
		if (strcmp(args[0], builtin_str[i]) == 0)
		{
			return (*builtin_func[i])(args);
		}
	}

	return dash_launch(args);
}