#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFSIZE 512

//debug bool
int debug = NULL;
void sig_handler_ignore(int);

void tokenize (char *input, char **tokens) {
  char *temp = strtok(input, "\n ");
  int x = 0;
  //may cause issues - revisit
  while(temp) {
    tokens[x] = temp;
    x++;
    temp = strtok(NULL, "\n ");
  }

  tokens[x] = NULL;
}

void change_dir (const char *path) {
  //for cd home
  if(!path || strcmp(path, "~") == 0) {
    path = getenv("HOME");
  }
  //error message and debug
  int ret = chdir(path);
  if(ret == 0 && debug) {
    printf("successfully change directory to %s\n", path);
  }
  if(ret == -1) {
    printf("Could not change directory to %s\n", path);
  }
}

void set_env_var (const char *var, const char *val) {
    int ret = setenv(var, val, 1);
    if(ret == 0 && debug) {
      printf("env var %s was set to %s\n", var, val);
    }
    if(ret == -1) {
      printf("env var was not set/created successfully\n");
    }
}

void unset_env_var (const char *var) {
  int ret = unsetenv(var);
  if(ret == 0 && debug) {
    printf("env var %s was unset\n", var);
  }
  if(ret == -1) {
    printf("env var was unable to be unset\n");
  }
}

void command(char **tokens) {
  //forks program
  pid_t p = fork();
  // if not child process
  if(p != 0) {
    signal(SIGINT, sig_handler_ignore); // trap ctrl-c in parents
    int x; // status storage - arbitrary
    waitpid(-1, &x, 0); //wait for any child to terminate, no options
  }
  else if (p == 0) {
    // execute non-built-in command
    int ret = execvp(tokens[0], tokens);
    if(ret == -1) {
      printf("Invalid Command!\n");
      exit(0);
    }
  }
}
//trap ctrl in parent processes
void sig_handler_ignore(int sig) {
  signal(SIGINT, sig_handler_ignore);
}

//allow for termination in lowest child
void sig_handler_term(int sig) {
  signal(SIGINT, sig_handler_term);
  exit(0);
}

void command_parse (char **tokens) {
  //built-in funtions
  if(strcmp(tokens[0], "exit") == 0) {
    exit(0);
  }
  else if(strcmp(tokens[0], "cd") == 0) {
    change_dir(tokens[1]);
  }
  else if(strcmp(tokens[0], "setenv") == 0) {
    set_env_var(tokens[1], tokens[2]);
  }
  else if(strcmp(tokens[0], "unsetenv") == 0) {
    unset_env_var(tokens[1]);
  }
  // non-built-in funtions
  else if(strcmp(tokens[0], "\n") != 0) {
    command(tokens);
  }
}

void rc_init(char **tokens) {
  FILE *fp;
  char *home = getenv("HOME");
  char file[BUFFSIZE];
  strcpy(file, home);
  strcat(file, "/.kapishrc");
  fp = fopen(file, "r");
  char line[BUFFSIZE];

  while(fgets(line, sizeof(line), fp)) {
    tokenize(line, tokens);
    printf(".kapishrc: %s\n", line);
    command_parse(tokens);
  }
  fclose(fp);
}

int main () {

  // start up dialog
  printf("***************************************\n");
  printf("*KLAMSHELL BY MATTHEW BELFORD         *\n");
  printf("*TYPE YOUR COMMAND AND HIT \"ENTER\"    *\n");
  printf("*TO EXIT TYPE \"exit\" OR HIT \"CTRL+D\"  *\n");
  printf("***************************************\n\n");

  // variable declaration
  char *input;
  size_t input_size = 0;
  char *tokens[BUFFSIZE];


  //initialize with .kapishrc
  rc_init(tokens);

  //used to detect CTRL-D
  int f = 0;

  while(1) {
    //CTRL-C Handler allows termination
    signal(SIGINT, sig_handler_term);

    printf("$ ");
    f = getline (&input, &input_size, stdin);


    // f detects the EOF flag from getline
    // if input is empty goto start of loop
    if(f == EOF) {
      printf("^D\n");
      exit(0);
    }
    else if(strcmp(input, "\n") == 0) {
      continue;
    }

    // gets tokens from input
    tokenize(input, tokens);
    // prints all tokens
    if(debug) {
      int i = 0;
      char *temp = tokens[i];
      while(temp) {
        printf("%s\n", temp);
        temp = tokens[++i];
      }
    }

    command_parse(tokens);
  }

  return 0;
}
