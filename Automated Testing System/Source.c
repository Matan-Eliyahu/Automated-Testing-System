#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <stdio.h>

int readLine(int, char*);
void createStudentsList(char*);
void getInputParameters(char*, char[][50]);
void writeGrades(char[], char[], char[][50]);
void runStudentProgram(char[], char[], char[][50]);
char* checkStudentProgram(char[], int);
void writeStudentGrade(int, char[], char*);

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        _exit(1);
    }

    int configFd = open(argv[1], O_RDONLY);
    if (configFd < 0)
    {
        _exit(1);
    }

    if (lseek(configFd, 0, SEEK_SET) != 0 || lseek(configFd, 0, SEEK_SET) != 0)
    {
        close(configFd);
        _exit(1);
    }

    char studentsPath[50];
    char inputPath[50];
    char expectedOutputPath[50];

    readLine(configFd, studentsPath);
    readLine(configFd, inputPath);
    readLine(configFd, expectedOutputPath);
    close(configFd);

    createStudentsList(studentsPath);

    char inputParameters[50][50] = { 0 };
    getInputParameters(inputPath, inputParameters);

    writeGrades(expectedOutputPath, studentsPath, inputParameters);
    return 0;
}

int readLine(int fd, char* buffer)
{
    int i = 0;
    int countReaded = read(fd, &buffer[i], 1);

    while (countReaded == 1)
    {
        if (buffer[i] == '\n')
        {
            buffer[i] = '\0';
            return 0;
        }

        i++;
        countReaded = read(fd, &buffer[i], 1);
    }

    if (countReaded == -1)
    {
        _exit(1);
    }

    //Reached EOF
    buffer[i] = '\0';
    return 1;
}

void createStudentsList(char* studentsPath)
{
    int studentsListFd = open("/home/matan.eliyahu6/studentsList.txt", O_WRONLY);
    if (studentsListFd < 0)
    {
        _exit(1);
    }

    int pid = fork();
    if (pid == 0)
    {
        close(1);
        dup(studentsListFd);
        close(studentsListFd);
        char* lsParameters[3] = { "ls", studentsPath, NULL };
        execvp("ls", lsParameters);
        _exit(1);
    }
    else
    {
        int stat;
        wait(&stat);

        if (stat != 0)
        {
            _exit(1);
        }

        lseek(studentsListFd, -1, SEEK_END);
        write(studentsListFd, "\0", 1);
        close(studentsListFd);
    }
}

void getInputParameters(char* inputPath, char inputParameters[][50])
{
    int inputFd = open(inputPath, O_RDONLY);
    if (inputFd < 0)
    {
        _exit(1);
    }

    int  reachedEOF;
    int i = 0;

    do {
        reachedEOF = readLine(inputFd, inputParameters[i]);
        i++;
    } while (!reachedEOF);
    inputParameters[i][0] = '\0';
    close(inputFd);
}

void writeGrades(char expectedOutputPath[], char studentsPath[], char inputParameters[][50])
{
    int studentsListFd = open("/home/matan.eliyahu6/studentsList.txt", O_RDONLY);
    if (studentsListFd < 0)
    {
        _exit(1);
    }

    int resultsFd = open("/home/matan.eliyahu6/results.csv", O_WRONLY);
    if (resultsFd < 0)
    {
        close(studentsListFd);
        _exit(1);
    }

    char currentStudent[50];
    int reachedEOF;
    do {
        memset(currentStudent, 0, 50);
        reachedEOF = readLine(studentsListFd, currentStudent);

        int pid = fork();
        if (pid == 0)
        {
            runStudentProgram(currentStudent, studentsPath, inputParameters);
        }
        else
        {
            int stat;
            wait(&stat);

            char* grade = checkStudentProgram(expectedOutputPath, stat);
            writeStudentGrade(resultsFd, currentStudent, grade);
        }
    } while (!reachedEOF);

    close(studentsListFd);
    close(resultsFd);
}

void runStudentProgram(char currentStudent[], char studentsPath[], char inputParameters[][50])
{
    int outputFd = open("/home/matan.eliyahu6/output.txt", (O_WRONLY | O_CREAT | O_TRUNC), 0666);
    if (outputFd < 0)
    {
        _exit(-1);
    }

    char* iP[50] = { 0 };
    iP[0] = "main.exe";

    for (int i = 1; i < 50; i++)
    {
        if (inputParameters[i - 1][0] == '\0')
        {
            iP[i] = NULL;
            break;
        }

        iP[i] = inputParameters[i - 1];
    }

    close(1);
    dup(outputFd);
    close(outputFd);

    char* runFilePath;
    runFilePath = strcat(studentsPath, "/");
    strcat(runFilePath, currentStudent);
    strcat(runFilePath, "/main.exe");

    _exit(execvp(runFilePath, iP));
}

char* checkStudentProgram(char expectedOutputPath[], int stat)
{
    if (stat == -1)
    {
        return "0";
    }

    int pid = fork();

    if (pid == 0)
    {
        char* parameters[4] = { "comp.out", expectedOutputPath, "/home/matan.eliyahu6/output.txt", NULL };
        _exit(execvp("/home/matan.eliyahu6/comp.out", parameters));
    }
    else
    {
        int newStat;
        wait(&newStat);

        if (WEXITSTATUS(newStat) == 2)
        {
            return "100";
        }

        return "0";
    }
}

void writeStudentGrade(int resultsFd, char currentStudent[], char* grade)
{
    strcat(currentStudent, ",");
    strcat(currentStudent, grade);
    strcat(currentStudent, "\n");

    if (write(resultsFd, currentStudent, strlen(currentStudent)) == -1)
    {
        exit(1); //instead of getting the studentListFd by a parameter, closing it and then _exit(1)
    }
}