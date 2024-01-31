#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#define FAILURE_CODE 1
#define BUFFER_SIZE 16
#define COMMAND_NUMBER 8


char* read_line() {
	int bufsize = BUFFER_SIZE;
	char *buffer = malloc(sizeof(char) * bufsize);
	if (!buffer) {
		printf("buffer malloc error\n");
		exit(FAILURE_CODE);
	}
	
	int pos = 0;
	while (1) {
		char c = getchar();

		if (c == EOF || c == '\n') {
			*(buffer + pos) = '\0';
			return buffer;
		}

		*(buffer + pos) = c;
		
		if (++pos >= bufsize - 1) {
			bufsize *= 2;
			buffer = realloc(buffer, bufsize);
			if (!buffer) {
				printf("buffer realloc error\n");
				exit(FAILURE_CODE);
			}
		}
	}
}


char** split(char* s) {
	int size = COMMAND_NUMBER + 1;	
	char** res = malloc(sizeof(char*)*size);
	if (!res) {
		printf("lex array malloc error\n");
		exit(FAILURE_CODE);
	}

	int lexN = 0;
	int start = 0;
	int pos = 0;

	do {
		if (*(s + pos) == ' ' || *(s + pos) == '\0') {
			if (pos - start == 0) {
				pos = ++start;
				continue;
			}
			char* word = malloc(sizeof(char)*(pos - start + 1));
			for (int i = 0; i < pos - start; i++) {
				*(word + i) = *(s + start + i);
			}
			if (lexN >= size - 1) {
				size *= 2;
				res = realloc(res, sizeof(char*)*size);
				if (!res) {
					printf("lex array realloc error\n");
					exit(FAILURE_CODE);
				}
			}
			*(res + lexN) = word;
			lexN++;
			start = pos + 1;	
		}
		pos += 1;
	} while (*(s + pos - 1) != '\0');
	
	return res;
}

// shell loop written somewhere around this time

int run_command(char** cmd) {
	int child_status;
	int id = fork();
	if (id == 0) {
		if (execvp(cmd[0], cmd) == -1) perror("Error executing\n");
	} else if (id < 0) {
		printf("Error forking\n");
	} else {
		do {
			waitpid(id, &child_status, WUNTRACED); 
	// with 1 child, I don't see the necessity of specifying id but whatever
		} while (!WIFEXITED(child_status) && !WIFSIGNALED(child_status));
	}
	return 1;
}

int shell_cd(char **lexes);
int shell_help(char **lexes);
int shell_exit(char **lexes);

char *builtins_name[] = {"cd", "help", "exit"};

int (*builtin_func[]) (char **) = {&shell_cd, &shell_help, &shell_exit};

int shell_cd(char **lexes) {
	if (lexes[1] == NULL) fprintf(stderr, "no directory specified\n");
	else if (chdir(lexes[1]) != 0) perror("something with chdir idk its implementation\n");
	return 1;
}

int shell_help(char **lexes) {
	printf("Shell's built-in commands:\n");
	int i = 0;
	while (*(builtins_name + i) != NULL) printf("-->%s\n", *(builtins_name + i++));
	return 1;
}
// dont need an argument at this stage but
// will leave it for a hypothetical future feature
// that can print out specified help info for each func

int shell_exit(char **lexes) {
	return 0;
}
// doesnt need any other implementation
// 0 return signals (not signals()-s) to
// the shell loop that it should stop

int compare_strings(char* a, char* b) {
	int i = 0;
	while(*(a + i) != '\0' || *(b + i) != '\0') {
		if (*(a + i) != *(b + i)) return 0;
		i++;
	}
	return *(a + i) == *(b + i) ? 1:0;
}
int shell_execute(char **lexes) {
	if (lexes[0] == NULL) return 1;
	int i = 0;
	while (builtins_name[i] != NULL) {
		if (compare_strings(lexes[0], builtins_name[i])) {
			return (*(builtin_func + i))(lexes);
		}
		i++;
	}
	return run_command(lexes);
}

void shell_loop() {
	int run_status;
	do {
		printf("command: ");
// user inputting \n or EOF means i probably dont need to manually put it anywhere
		char *input = read_line();
		char **lexes = split(input);
		run_status = shell_execute(lexes);
		free(input);
		free(lexes);
	} while (run_status);
}

int main() {
	shell_loop();
	return 0;
}
