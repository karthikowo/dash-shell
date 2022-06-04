void lsh_loop();
char *lsh_read_line();
char **lsh_split_line();

int main(int argc, char **argv)
{
        // Load config files, if any.

        // Run command loop.
        lsh_loop();

        // Perform any shutdown/cleanup.

        return EXIT_SUCCESS;
}

void lsh_loop(void)
{
        char *line;
        char **args;
        int status;

        do
        {
                printf("$ ");
                line = lsh_read_line();
                args = lsh_split_line(line);
                status = lsh_execute(args);

                free(line);
                free(args);
        } while (status);
}

#define LSH_RL_BUFSIZE 1024
char *lsh_read_line(){
        int buffersize = LSH_RL_BUFSIZE;
        int position = 0;
        char *buffer = malloc(sizeof(char)*buffersize);
        int c;

        if(!buffer){
                fprintf(stderr, "lsh : allocation error\n");
                exit(EXIT_FAILURE);
        }



        while(1){
                c = getchar();

                if (c == EOF || c == '\n'){
                        buffer[position] = '\0';
                        return buffer;
                }
                else{
                        buffer[position]=c;
                }

                position++;
		    if (position >= buffersize){
                        buffersize += LSH_RL_BUFSIZE;
                        buffer = realloc(buffer,bufsize);
                        if (!buffer){
                                fprintf(stderr, "lsh: allocation error\n");
                                exit(EXIT_FAILURE);
                        }
                }
        }
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

char **lsh_split_line(char *line){
        int buffersize = LSH_TOK_BUFSIZE, position = 0;
        char **tokens = malloc(buffersize * sizeof(char*));
        char *token;

        if(!tokens){
                fprintf(stderr, "lsh:allocation error\n");
                exit(EXIT_FAILURE);
        }

        token = strtok(line, LSH_TOK_DELIM);

        while (token != NULL){
                tokens[position] = token;
                position++;

                if(position>=buffersize){
                        buffersize += LSH_TOK_BUFSIZE;
                        tokens = realloc(tokens,bufsize* sizeof(char*));

                        if(!tokens){
                                bufsize += LSH_TOK_BUFSIZE;
                                tokens = realloc(tokens,buffersize*sizeof(char*));
                        }
                }

                token = strtok(NULL,LSH_TOK_DELIM);
        }
        tokens[position] = NULL;
        return tokens;
}