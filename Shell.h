#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <sys/wait.h>
#include <sys/types.h>

// defines
#define LIMIT 256
#define MAXLINE 1024

// variables
pid_t pid;
int QUIT = 1, prompt;
static char *currentFolder;

// functions
void blueColor();
void greenColor();
void resetColor();
void pipeHandler(char **);
void commandHandler(char **);
void changeDirectory(char **);
void folderIO(char **, char *, char *, int);