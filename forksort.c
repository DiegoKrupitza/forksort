#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

char *programmname = "./forksort";

/**
 * @brief Get the Content As Array object
 * @author Diego Krupitza
 * 
 * @param content the content i want to convert to a array
 * @param lines  the array to store the data in
 */
void getContentAsArray(char *content, char **lines)
{
    char *delimiter = "\n";

    char *tokpointer = strtok(content, delimiter);
    for (int i = 0; tokpointer != NULL; i++)
    {
        //printf("%s", tokpointer);
        lines[i] = tokpointer;
        tokpointer = strtok(NULL, delimiter);
    }
}

/**
 * @brief Merge from merge sort
 * @author Diego Krupitza
 * 
 * @param A  the array containing part1 and part2 that should merge
 * @param iBegin  begin of part1
 * @param iMiddle  end of part1 and start of part2
 * @param iEnd  the end of part2
 * @param B  the array with the result
 */
void topDownMergeFromWikipedia(char *A[], int iBegin, int iMiddle, int iEnd, char *B[])
{
    // thanks to the pseudo code from
    // because my impl didnt worked XD
    // https://en.wikipedia.org/wiki/Merge_sort
    int i = iBegin, j = iMiddle;

    // While there are elements in the left or right runs...
    for (int k = iBegin; k < iEnd; k++)
    {
        // If left run head exists and is <= existing right run head.
        if (i < iMiddle && (j >= iEnd || strcmp(A[i], A[j]) <= 0))
        {
            B[k] = A[i];
            i = i + 1;
        }
        else
        {
            B[k] = A[j];
            j = j + 1;
        }
    }
}

/**
 * @brief Merges two array according to merge sort and prints them
 * @author Diego Krupitza
 * 
 * @param sortedLeftPart array 1 to merge
 * @param part1Length  lenght of array1
 * @param sortedRightPart  array 2 to merge 
 * @param part2Length part of array2
 */
void mergeAndPrint(char *sortedLeftPart[], int part1Length, char *sortedRightPart[], int part2Length)
{
    // Merge the sorted parts from the two child processes and write them
    // to stdout. At each step, compare the next line of both parts and
    // write the smaller one to stdout, such that the lines are written
    // in alphabetical order. Terminate the program with exit
    //status EXIT_SUCCESS.

    int totalLength = part2Length + part1Length;
    char *aTotal[totalLength];
    for (int i = 0; i < totalLength; i++)
    {
        if (i < part1Length)
        {
            aTotal[i] = sortedLeftPart[i];
        }
        else
        {
            aTotal[i] = sortedRightPart[i - part1Length];
        }
    }

    char *sorted[totalLength];
    topDownMergeFromWikipedia(aTotal, 0, part1Length, totalLength, sorted);
    for (int i = 0; i < totalLength; i++)
    {
        printf("%s\n", sorted[i]);
        if (fflush(stdout) != 0)
        {
            //ERROR
            fprintf(stderr, "%s: Fehler beim flushen von stdout: %s", programmname, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * @brief Handles the fork process for the parrent process
 * @author Diego Krupitza
 * 
 * @param childPid the pid of the child 
 * @param lines the lines i want to send to the child over a pipe
 * @param partLength the number of lines i want to send
 * @param pipeParentToChild  a pipe to communicate from parent to child
 * @param pipeChildToParent  a pipe to communicate from child to parent 
 * @param sortedPart  the sorted restul from the child
 */
void forkParentHandler(pid_t childPid, char *lines[], int partLength, int pipeParentToChild[], int pipeChildToParent[], char *sortedPart[])
{
    // send to lines to the child process
    
    // pipe 0 - read
    // pipe 1 - write

    // closing the unused pipes
    if (close(pipeParentToChild[0]) != 0)
    {
        //ERROR
        fprintf(stderr, "%s: Error closing: %s\n", programmname, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (close(pipeChildToParent[1]) != 0)
    {
        //ERROR
        fprintf(stderr, "%s: Error closing: %s\n", programmname, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // sending the data to the child over the pipe
    FILE *parentToChild = fdopen(pipeParentToChild[1], "w");
    for (int i = 0; i < partLength; i++)
    {
        char *currentLine = lines[i];
        if (fputs(currentLine, parentToChild) == EOF)
        {
            //ERROR
            fprintf(stderr, "%s: Error putting on pipe from parent to child\n", programmname);
            exit(EXIT_FAILURE);
        }
        if (fputc('\n', parentToChild) == EOF)
        {
            //ERROR
            fprintf(stderr, "%s: Error fputc newline on pipe from parent to child\n", programmname);
            exit(EXIT_FAILURE);
        }
    }
    // flush to make sure it is really send
    if (fflush(parentToChild) != 0)
    {
        //ERROR
        fprintf(stderr, "%s: Error flushing on pipe from parent to child: %s\n", programmname, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (fclose(parentToChild) != 0)
    {
        //ERROR
        fprintf(stderr, "%s: Error closing on pipe from parent to child: %s\n", programmname, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // wait for the pid of the forked child
    // check the exit code if ERROR_FAILURE then kill the whole thing
    // get the content the child sent to stdout(aka pipe)
    // return content as array
    int status;
    waitpid(childPid, &status, 0);

    if (WEXITSTATUS(status) == EXIT_SUCCESS)
    {
        //everything was fine

        // reading the content from the child
        FILE *childToParent = fdopen(pipeChildToParent[0], "r");
        char *content = calloc(5, sizeof(char));

        char *line = NULL;
        size_t linecap = 0;
        ssize_t linelen;
        while ((linelen = getline(&line, &linecap, childToParent)) > 0)
        {
            // reading the lines and the number of lines
            int newLenght = strlen(content) + strlen(line) + 5;
            content = realloc(content, newLenght);
            strcat(content, line);
            strcat(content, "\n");
        }
        if (fclose(childToParent) != 0)
        {
            //ERROR
            fprintf(stderr, "%s: Error on closing childtoParent file: %s\n", programmname, strerror(errno));
            exit(EXIT_FAILURE);
        }
        getContentAsArray(content, sortedPart);
    }
    else
    {
        fprintf(stderr, "%s: Child stopped with an exit code not equal to EXIT_SUCCESS\n", programmname);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Handles the fork process for the child process
 * @author Diego Krupitza
 * 
 * @param pipeParentToChild a pipe to communicate from parent to child
 * @param pipeChildToParent a pipe to communicate from child to parent
 */
void forkChildHandler(int pipeParentToChild[], int pipeChildToParent[])
{
    // pipe 0 - read
    // pipe 1 - write
    if (close(pipeParentToChild[1]) != 0)
    {
        //ERROR
        fprintf(stderr, "%s: Error closing: %s\n", programmname, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (close(pipeChildToParent[0]) != 0)
    {
        //ERROR
        fprintf(stderr, "%s: Error closing: %s\n", programmname, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // map ChildToParent write to stdout
    dup2(pipeChildToParent[1], STDOUT_FILENO); // new descriptor // old descriptor
    close(pipeChildToParent[1]);

    // map parenttochild read to stdin
    dup2(pipeParentToChild[0], STDIN_FILENO); // new descriptor // old descriptor
    close(pipeParentToChild[0]);

    // execute forksort on new stdin
    char *argus[] = {"./forksort", (char *)0};
    execvp("./forksort", argus);

    fprintf(stderr, "%s: Cannot exec!\n", programmname);
    exit(EXIT_FAILURE);
}

/**
 * @brief Handles the left fork of the programm
 * @author Diego Krupitza
 * 
 * @param lines the lines to fork
 * @param part1Length  the lenght of the lines array
 * @param sortedLeftPart  the sorted result
 */
void leftForkMergeMagic(char *lines[], int part1Length, char *sortedLeftPart[])
{
    // magic fork
    // fork and then send from parent process via pipe to child process
    // the lines that is for the child.
    // this pipe uses stdin as input
    // and stdout for the communication to parent
    // then the child calls execlp on forksort with the date from parent send
    // after finishing of child it send the info to the pipe automaticly because stdout is dup2 to pipewriteend

    // pipe 0 - read
    // pipe 1 - write
    int leftPipeParentToChild[2];
    if (pipe(leftPipeParentToChild) != 0)
    {
        fprintf(stderr, "%s: Left PipeParentToChild cannot be created! %s\n", programmname, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // pipe 0 - read
    // pipe 1 - write
    int leftPipeChildToParent[2];
    if (pipe(leftPipeChildToParent) != 0)
    {
        fprintf(stderr, "%s: Left PipeChildToParent cannot be created! %s\n", programmname, strerror(errno));
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    switch (pid)
    {
    case (-1):
        fprintf(stderr, "%s: Fehler beim forken: %s\n", programmname, strerror(errno));
        exit(EXIT_FAILURE);
        break;
    case 0:
        // child part
        forkChildHandler(leftPipeParentToChild, leftPipeChildToParent);
        exit(EXIT_SUCCESS);
        break;
    default:
        // we are on the parent side
        forkParentHandler(pid, lines, part1Length, leftPipeParentToChild, leftPipeChildToParent, sortedLeftPart);
        return;
    }
}

/**
 * @brief Handles the right fork of the programm
 * @author Diego Krupitza
 * 
 * @param lines the lines to fork
 * @param part2Length  the lenght of the lines array
 * @param sortedLeftPart  the sorted result
 */
void rightForkMergeMagic(char *lines[], int part2Length, char *sortedRightPart[])
{
    // fork and then send from parent process via pipe to child process
    // the lines that is for the child.
    // this pipe uses stdin as input
    // and stdout for the communication to parent
    // then the child calls execlp on forksort with the date from parent send
    // after finishing of child it send the info to the pipe automaticly because stdout is dup2 to pipewriteend

    // pipe 0 - read
    // pipe 1 - write
    int rightPipeParentToChild[2];
    if (pipe(rightPipeParentToChild) != 0)
    {
        fprintf(stderr, "%s: Right PipeParentToChild cannot be created! %s\n", programmname, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // pipe 0 - read
    // pipe 1 - write
    int rightPipeChildToParent[2];
    if (pipe(rightPipeChildToParent) != 0)
    {
        fprintf(stderr, "%s: Right PipeChildToParent cannot be created! %s\n", programmname, strerror(errno));
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    switch (pid)
    {
    case (-1):
        fprintf(stderr, "%s: Fehler beim forken: %s\n", programmname, strerror(errno));
        exit(EXIT_FAILURE);
        break;
    case 0:
        // child part
        forkChildHandler(rightPipeParentToChild, rightPipeChildToParent);
        exit(EXIT_SUCCESS);
        break;
    default:
        // we are on the parent side
        forkParentHandler(pid, lines, part2Length, rightPipeParentToChild, rightPipeChildToParent, sortedRightPart);
        return;
    }
}

/**
 * @brief Handles the recursiv fork magic for forkmerge
 * @author Diego Krupitza
 * 
 * @param lines  the lines to sort
 * @param part1Length  length of left part
 * @param part2Length  lenght of right part
 */
void forkMagic(char *lines[], int part1Length, int part2Length)
{
    //fprintf(stderr, "Part1: %d    Part2: %d\n", part1Length, part2Length);
    // lines array for the right part
    char *rightLines[part2Length];
    for (int i = part1Length; i < part1Length + part2Length; i++)
    {
        rightLines[i - part1Length] = lines[i];
    }

    char *sortedLeftPart[part1Length];
    leftForkMergeMagic(lines, part1Length, sortedLeftPart);

    char *sortedRightPart[part2Length];
    rightForkMergeMagic(rightLines, part2Length, sortedRightPart);

    mergeAndPrint(sortedLeftPart, part1Length, sortedRightPart, part2Length);
}

/**
 * @brief Return the ocntent from StdIn and writes the number of lines into the numberOfLines param
 * @author Diego Krupitza
 * 
 * @param numberOfLines stroes the number of line into there
 * @return char* the content 
 */
char *readContentFromStdIn(int *numberOfLines)
{
    FILE *inputlines = fdopen(STDIN_FILENO, "r");

    char *content = calloc(5, sizeof(char));
    (*numberOfLines) = 0;

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    while ((linelen = getline(&line, &linecap, inputlines)) > 0)
    {
        // reading the lines and the number of lines
        (*numberOfLines)++;
        int newLenght = strlen(content) + strlen(line) + 3;
        content = realloc(content, newLenght);
        strcat(content, line);
    }

    // stop because no lines found
    if ((*numberOfLines) <= 0)
    {
        fprintf(stderr, "%s: No Lines read!\n", programmname);
        exit(EXIT_FAILURE);
    }
    return content;
}

/**
 * @brief The main programm
 * @author Diego Krupitza
 * 
 * @param argc the argument counter 
 * @param argv the argumentes
 * @return int the exitcode
 */
int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        fprintf(stderr, "%s: There are not arguments allowed! Your argcounter is: %d\n", programmname, argc);
        exit(EXIT_FAILURE);
    }

    int numberOfLines = 0;
    char *content = readContentFromStdIn(&numberOfLines);

    char *lines[numberOfLines];
    getContentAsArray(content, lines);

    if (numberOfLines == 1)
    {
        fprintf(stdout, "%s", lines[0]);
        fflush(stdout);
        exit(EXIT_SUCCESS);
    }
    else
    {
        // split them so you can mergesort correctly
        int part1Length = numberOfLines / 2;
        int part2Length = numberOfLines - part1Length;

        forkMagic(lines, part1Length, part2Length);
    }

    free(content);
    return EXIT_SUCCESS;
}
