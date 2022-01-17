#include "Shell.h"

void pipeHandler(char *args[])
{
    pid_t pid;
    int filedes[2];
    int filedes2[2];
    int num_cmds = 0;
    int i = 0, j = 0;
    int k = 0, l = 0;
    char *command[256];
    int err = -1, end = 0;

    while (args[l] != NULL)
    {
        if (strcmp(args[l], "|") == 0)
        {
            num_cmds++;
        }
        l++;
    }
    num_cmds++;

    while (args[j] != NULL && end != 1)
    {
        k = 0;
        while (strcmp(args[j], "|") != 0)
        {
            command[k] = args[j];
            j++;
            if (args[j] == NULL)
            {
                end = 1;
                k++;
                break;
            }
            k++;
        }

        command[k] = NULL;
        j++;

        if (i % 2 != 0)
        {
            pipe(filedes);
        }
        else
        {
            pipe(filedes2);
        }

        pid = fork();

        if (pid == -1)
        {
            if (i != num_cmds - 1)
            {
                if (i % 2 != 0)
                {
                    close(filedes[1]);
                }
                else
                {
                    close(filedes2[1]);
                }
            }
            printf("Child process could not be created\n");
            return;
        }

        if (pid == 0)
        {
            if (i == 0)
            {
                dup2(filedes2[1], STDOUT_FILENO);
            }
            else if (i == num_cmds - 1)
            {
                if (num_cmds % 2 != 0)
                {
                    dup2(filedes[0], STDIN_FILENO);
                }
                else
                {
                    dup2(filedes2[0], STDIN_FILENO);
                }
            }
            else
            {
                if (i % 2 != 0)
                {
                    dup2(filedes2[0], STDIN_FILENO);
                    dup2(filedes[1], STDOUT_FILENO);
                }
                else
                {
                    dup2(filedes[0], STDIN_FILENO);
                    dup2(filedes2[1], STDOUT_FILENO);
                }
            }

            if (execvp(command[0], command) == err)
            {
                kill(getpid(), SIGTERM);
            }
        }

        if (i == 0)
        {
            close(filedes2[1]);
        }

        else if (i == num_cmds - 1)
        {
            if (num_cmds % 2 != 0)
            {
                close(filedes[0]);
            }
            else
            {
                close(filedes2[0]);
            }
        }
        else
        {
            if (i % 2 != 0)
            {
                close(filedes2[0]);
                close(filedes[1]);
            }
            else
            {
                close(filedes[0]);
                close(filedes2[1]);
            }
        }
        waitpid(pid, NULL, 0);
        i++;
    }
}

void folderIO(char *args[], char *inputFile, char *outputFile, int option)
{
    int error = -1, fileDescriptor;

    if ((pid = fork()) == -1)
    {
        printf("Child process could not be created\n");
        return;
    }

    if (pid == 0)
    {
        if (option == 0)
        {
            fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
            dup2(fileDescriptor, STDOUT_FILENO);
            close(fileDescriptor);
        }

        else if (option == 1)
        {
            fileDescriptor = open(inputFile, O_RDONLY, 0600);
            dup2(fileDescriptor, STDIN_FILENO);
            FILE *fp;
            char *line = NULL;
            size_t len = 0;
            ssize_t read;

            fp = fopen(inputFile, "r");
            if (fp == NULL)
                exit(EXIT_FAILURE);

            while ((read = getline(&line, &len, fp)) != -1)
            {
                printf("%s", line);
            }

            fclose(fp);
            if (line)
                free(line);
            close(fileDescriptor);
        }

        setenv("parent", getcwd(currentFolder, 1024), 1);

        if (execvp(args[0], args) == error)
        {
            printf("Error!");
            kill(getpid(), SIGTERM);
        }
    }

    waitpid(pid, NULL, 0);
}

void commands(char *command[], int background)
{
    int error = -1;

    if ((pid = fork()) == -1)
    {
        printf("Something went wrong!\n");
        return;
    }

    // child process
    if (pid == 0)
    {
        signal(SIGINT, SIG_IGN);
        setenv("parent", getcwd(currentFolder, 1024), 1);

        if (execvp(command[0], command) == error)
        {
            printf("Command not found");
            kill(getpid(), SIGTERM);
        }
    }

    // parent process
    else if (pid > 0)
    {
        if (background == 0)
        {
            waitpid(pid, NULL, 0);
        }
        else
        {
            printf("Process created with PID: %d\n", pid);
        }
    }
}

void changeDirectory(char *command[])
{
    // sadece 'cd' yazilirsa 'Home' dizisine gitsin
    if (command[1] == NULL || (strcmp(command[1], "~") == 0))
        chdir(getenv("HOME"));

    // cd'den sonra dosya yazilirsa oraya gitsin
    else if (chdir(command[1]) == -1)
        printf("%s: no such directory\n", command[1]);
}

void commandHandler(char *command[])
{
    int i = 0, j = 0;
    char *commandTemp[256];
    int temp, background = 0;

    // ozel karakterleri ariyoruz, bulunca komutun kendisini yeni bir diziye kopyalariyoruz
    while (command[j] != NULL)
    {
        if ((strcmp(command[j], ">") == 0) || (strcmp(command[j], "<") == 0) || (strcmp(command[j], "&") == 0))
        {
            break;
        }

        commandTemp[j] = command[j];
        j++;
    }

    // exit komutu girilince programdan cikilir
    if (strcmp(command[0], "exit") == 0)
    {
        exit(0);
    }
    // pwd komutu girilince oldugumuz dosyanin yolunu yazdirilir
    else if (strcmp(command[0], "pwd") == 0)
    {
        printf("%s\n", getcwd(currentFolder, 1024));
    }
    // clear komutu girilince ekran temizlenecek
    else if (strcmp(command[0], "clear") == 0)
    {
        system("clear");
    }
    // cd komutu girilince dosyayi degistirecek olan fonksyon cagirilir
    else if (strcmp(command[0], "cd") == 0)
    {
        changeDirectory(command);
    }
    // girilien command, gecen komutlardan degilse
    // dosya I/O, pipe islemi ya da arkaplan isi olabilir
    else
    {
        while (command[i] != NULL && background == 0)
        {
            // eger islem bir arkaplan islemi ise donguden cikilir
            if (strcmp(command[i], "&") == 0)
            {
                background = 1;
            }

            // eger pipe islemi ise pipe isleyicisi cagirilir
            else if (strcmp(command[i], "|") == 0)
            {
                pipeHandler(command);
            }

            else if (strcmp(command[i], "<") == 0)
            { // input
                temp = i + 1;
                if (command[temp] == NULL)
                {
                    printf("Not enough input arguments\n");
                }
                else
                {
                    folderIO(commandTemp, command[temp], NULL, 1);
                }
            }

            else if (strcmp(command[i], ">") == 0)
            { // output
                if (command[i - 1] == NULL || command[i + 1] == NULL)
                {
                    printf("Not enough input arguments\n");
                }
                else
                {
                    folderIO(commandTemp, command[i - 1], command[i + 1], 0);
                }
            }
            i++;
        }

        commandTemp[i] = NULL;
        commands(commandTemp, background);
    }
}

void blueColor()
{
    printf("\e[0;34m");
}

void greenColor()
{
    printf("\e[0;32m");
}

void resetColor()
{
    printf("\033[0m");
}

int main(int argc, char const *argv[])
{
    system("clear");
    int tokensNumber;
    char line[MAXLINE]; // buffer for the user input
    char *token[LIMIT]; // array for the different tokens in the command
    prompt = 0;

    while (1)
    {
        if (prompt == 0)
        {
            greenColor();
            printf("%s@%s:", getlogin(), getlogin());
            blueColor();
            printf("%s", getcwd(currentFolder, 1024));
            resetColor();
            printf("$ ");
        }
        prompt = 0;
        memset(line, '\0', MAXLINE);

        // We wait for user input
        fgets(line, MAXLINE, stdin);

        // Bos girilirse dongumuz kendisini tekrar eder;
        if ((token[0] = strtok(line, " \n\t")) == NULL)
        {
            continue;
        }

        // Girilen komutu okuyup komutIsleyicisi fonksyonuna aktariyoruz;
        tokensNumber = 1;
        while ((token[tokensNumber] = strtok(NULL, " \n\t")) != NULL)
        {
            tokensNumber++;
        }

        commandHandler(token);
    }

    return EXIT_SUCCESS;
}