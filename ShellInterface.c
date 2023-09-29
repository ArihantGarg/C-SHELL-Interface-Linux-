#include<stdio.h>
#include<unistd.h>
#include<sys/times.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<sys/stat.h>

char inputFile[128];
char outputFile[128];
char prevCommands[10][128];
int startCommand=0,endCommand=0;


void printPrompt() // Prints user prompt to write command
{
    printf("\n\nType next command : \n");
    char currentDirectory[128];
    getcwd(currentDirectory,sizeof(currentDirectory));
    printf("%s > ",currentDirectory);
}


void readCommandLine(char* cmdLine,size_t *bufsize) // Reads user command
{
    getline(&cmdLine, bufsize, stdin);

    if(strlen(cmdLine)==0) // When input is from file , exiting interface after end of file
    {
        exit(0);
    }
    
    // Adding command to history

    strcpy(prevCommands[endCommand],cmdLine);

    endCommand=(endCommand+1)%10;
    if(startCommand==endCommand)
        startCommand=(startCommand+1)%10;
}


char* removeWhitespaceNewline(char* cmdLine) // Parses whitespaces and newline characters
{
    char temp[128];
    int currentIndex=0,tempIndex=0;

    while(cmdLine[currentIndex]!='\n' && cmdLine[currentIndex]!='\0')
    {
        if(cmdLine[currentIndex]==' ' && cmdLine[currentIndex+1]==' ')
        {
            currentIndex++;
            continue;
        }

        temp[tempIndex++]=cmdLine[currentIndex++];
    }

    temp[tempIndex]='\0';

    strcpy(cmdLine,temp);

    return cmdLine;
}


void modifyCmdLine(char* cmdLine) // Modifies command for redirection operators and filename separation
{
    char* inputOperator = "<";
    char* outputOperator = ">";
    char token[128];  // Define a fixed-size char array for token
    char finalCmdLine[128];  // Define a fixed-size char array for the modified command line
    finalCmdLine[0] = '\0'; // Initialize finalCmdLine as an empty string

    strcpy(token, cmdLine); // Copy the cmdLine into token

    char* tokenPtr = strtok(token, " ");  // Use a separate pointer for tokenization

    while (tokenPtr != NULL) {
        if (strcmp(tokenPtr, inputOperator) == 0 || strcmp(tokenPtr, outputOperator) == 0) {
            // Skip the operator and the associated filename
            tokenPtr = strtok(NULL, " ");
            if (tokenPtr) {
                // Skip the filename
                tokenPtr = strtok(NULL, " ");
            }
        } else {
            // Append the token to the final command line
            if (finalCmdLine[0] == '\0') {
                strcpy(finalCmdLine, tokenPtr); // Copy tokenPtr into finalCmdLine
            } else {
                // Concatenate the token to the existing finalCmdLine
                strcat(finalCmdLine, " ");
                strcat(finalCmdLine, tokenPtr);
            }
            tokenPtr = strtok(NULL, " ");
        }
    }

    strcpy(cmdLine, finalCmdLine);
}


void fileParsing(char* cmdLine) // Parses for redirection operators ( '<' and '>' )
{
    strcpy(inputFile,"");
    strcpy(outputFile,"");

    char* inputOperator = "<";
    char* outputOperator = ">";
    char token[128];

    strcpy(token, cmdLine);

    char* tokenPtr = strtok(token, " ");

    while (tokenPtr != NULL) {
        if (strcmp(tokenPtr, inputOperator) == 0)
        {
            tokenPtr = strtok(NULL, " ");
            if (tokenPtr)
                strcpy(inputFile, tokenPtr);
        }
        else if (strcmp(tokenPtr, outputOperator) == 0)
        {
            tokenPtr = strtok(NULL, " ");
            if (tokenPtr)
                strcpy(outputFile, tokenPtr);
        }
        else
        {
            tokenPtr = strtok(NULL, " ");
        }
    }

    modifyCmdLine(cmdLine);
}


int parseCommand(char* cmdLine) // Parses command
{
    cmdLine=removeWhitespaceNewline(cmdLine);

    // Parsing for redirection( '<' and '>' operators )
    fileParsing(cmdLine);

    // Parsing for pwd command
    if(strcmp("pwd",cmdLine)==0)
        return 1;

    // Parsing for exit command    
    if(strcmp("exit",cmdLine)==0 || strcmp("kill",cmdLine)==0)
        return 2;

    // Parsing for cd command 
    if(cmdLine[0]=='c' && cmdLine[1]=='d' && cmdLine[2]==' ')
    {
        strncpy(cmdLine,cmdLine+3,strlen(cmdLine)-2); // Changing cmdLine to path name

        if(cmdLine[0]=='~') // Handling ~ in cd
        {
            char* homeDirectory=getenv("HOME");
            strncpy(cmdLine,cmdLine+1,strlen(cmdLine));
            strcat(homeDirectory,cmdLine);
            strcpy(cmdLine,homeDirectory);
        }

        return 3;
    }
    else if(cmdLine[0]=='c' && cmdLine[1]=='d' && strlen(cmdLine)==2)
    {
        char* homeDirectory=getenv("HOME");
        strcpy(cmdLine,homeDirectory);
        return 3;
    }

    // Parsing for history command
    if(strcmp("history",cmdLine)==0)
        return 4;

    return 0; // All other commands
}


int isBuiltInCommand(int cmd) // Checks for inbuilt command
{
    if(cmd==1)
        return 1; // pwd

    if(cmd==2)
        return 2; // exit/kill

    if(cmd==3)
        return 3; // cd

    if(cmd==4)
        return 4; // history

    return 0; // Non built in command
}


void executeBuiltInCommand(int cmd,char* cmdLine) // Executes inbuilt command
{
    if(cmd == 1) // pwd
    {
        char currentDirectory[128];

        getcwd(currentDirectory,sizeof(currentDirectory));
        printf("Current directory : %s\n",currentDirectory);
    }

    if(cmd == 2) // exit/kill
    {
        printf("Exiting from command line\n");
        exit(0);
    }

    if(cmd == 3) // cd 
    {
        if(chdir(cmdLine))
        {
            printf("This directory does not exist\n");
        }

        executeBuiltInCommand(1,cmdLine);
    }

    if(cmd == 4) // history
    {
        if(startCommand==(endCommand+1)%10)
            printf("%s",prevCommands[endCommand]);
        for(int i=(startCommand)%10;i!=endCommand;i=(i+1)%10)
        {
            printf("%s",prevCommands[i]);
        }
    }
}


char** createArgumentList(char* cmdLine) // Parses command into different strings
{
    int maxArgs = 20;
    
    char** argument_list = (char**)malloc(sizeof(char*) * (maxArgs + 1));

    char* token = strtok(cmdLine, " ");
    int argIndex = 0;

    while (token != NULL) {
        argument_list[argIndex] = strdup(token);

        token = strtok(NULL, " ");
        argIndex++;
    }

    // Set the last element of the argument list to NULL (required by execvp)
    argument_list[argIndex] = NULL;

    return argument_list;
}


void executeNonBuiltInCommand(char* cmdLine) // Executes non-built in commands
{
    char** argumentList=createArgumentList(cmdLine);

    if(execvp(argumentList[0],argumentList)==-1)
    {
        printf("Not a valid command\n");
        exit(0); // Exiting from child process
    }

}


void inputOutputRedirection() // Handles file input/output
{
    if (strcmp(inputFile,"")) 
    {
        int inputFd = open(inputFile, O_RDONLY , 0644);

        if (inputFd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        if (dup2(inputFd, STDIN_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        
        close(inputFd);
    }

    if (strcmp(outputFile,""))
    {
        int outputFd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC , 0644);

        if (outputFd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        if (dup2(outputFd, STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }

        close(outputFd);
    }
}


void executePipeCommand(char* cmdLine)
{
    char* cmd1 = strtok(cmdLine, "|"); // Get the first command
    char* cmd2 = strtok(NULL, "|");    // Get the second command

    if (!cmd1 || !cmd2)
    {
        printf("Invalid pipe command\n");
        return;
    }

    // Create two pipes for communication between the commands
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        exit(0);
    }

    // Fork the first child process to execute the first command
    int childPid1 = fork();
    if (childPid1 < 0)
    {
        perror("fork");
        exit(0);
    }

    if (childPid1 == 0)
    {
        // Child process for the first command
        close(pipefd[0]); // Close the read end of the pipe

        // Redirect stdout to the write end of the pipe
        if (dup2(pipefd[1], STDOUT_FILENO) == -1)
        {
            perror("dup2");
            exit(EXIT_FAILURE);
        }

        close(pipefd[1]); // Close the write end of the pipe

        // Execute the first command
        executeNonBuiltInCommand(cmd1);

        // Exit the child process
        exit(0);
    }

    // Fork the second child process to execute the second command
    int childPid2 = fork();
    if (childPid2 < 0)
    {
        perror("fork");
        exit(0);
    }

    if (childPid2 == 0)
    {
        // Child process for the second command
        close(pipefd[1]); // Close the write end of the pipe

        // Redirect stdin to the read end of the pipe
        if (dup2(pipefd[0], STDIN_FILENO) == -1)
        {
            perror("dup2");
            exit(EXIT_FAILURE);
        }

        close(pipefd[0]); // Close the read end of the pipe

        // Execute the second command
        executeNonBuiltInCommand(cmd2);

        // Exit the child process
        exit(0);
    }

    // Close both ends of the pipe in the parent process
    close(pipefd[0]);
    close(pipefd[1]);

    // Wait for both child processes to finish
    waitpid(childPid1, NULL, 0);
    waitpid(childPid2, NULL, 0);

    exit(0);
}


int checkPipe(char * cmdLine) // Checks for pipe operator ('|')
{
    if(strchr(cmdLine,'|')==NULL)
        return 0;
    return 1;
}


int main()
{
    while(1)
    {
        printPrompt(); // Printing prompt
        char* cmdLine;

        size_t bufsize=128;
        cmdLine = (char*)malloc(sizeof(bufsize));
        readCommandLine(cmdLine,&bufsize); // Reading user command

        int cmd=0;

        cmd=parseCommand(cmdLine); // Parsing command
        fflush(stdout);


        if(isBuiltInCommand(cmd)) // Cecking for inbuilt commands 
        {
            executeBuiltInCommand(cmd,cmdLine);
        }
        else
        {
            // File input/output
            if(strcmp(inputFile,"")!=0)
                printf("Reading input from %s\n",inputFile);
            if(strcmp(outputFile,"")!=0)
                printf("Writing output to %s\n",outputFile);

            // Fork

            int childPid = fork();

            if(childPid<0)
            {
                printf("Fork failed\n");
                return 0;
            }

            if(childPid==0)
            {
                // Child process
                fflush(stdout);

                // Changing to file input/output incase of redirection operators
                inputOutputRedirection();

                if(checkPipe(cmdLine)) // Checking for pipe operator
                    executePipeCommand(cmdLine);
                else
                    executeNonBuiltInCommand(cmdLine);
            }
            else
            {
                wait(NULL);
            }
        }

        free(cmdLine);
    }

    return 0;
}
