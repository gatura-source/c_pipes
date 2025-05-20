
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <time.h>

#define MAX_FILES 100 // Maximum number of files to process
#define BUFFER_SIZE 64 // Buffer size for reading and writing operations

// Child process function
void child_process(int p_2_c[2], int c_2_p[2], char *filename)
{
    // Close the unused pipe ends
    close(p_2_c[1]); /*close parent to child write end*/
    close(c_2_p[0]); /*close child to parent read end */

    char buffer[BUFFER_SIZE];
    int fd_read;
    char *name = NULL;
    char *dob = NULL;
    char *result = NULL;
    char *age_str = NULL;
    int name_idx = 0;
    int dob_idx = 0;
    int current_year = 0;
    int age = 0;
    int day, month, year = 0;
    time_t now;
    char c;

    // Read the filename from the parent process
    ssize_t bytesRead = read(p_2_c[0], buffer, BUFFER_SIZE);
    if (bytesRead == -1) {
        perror("Error reading from parent pipe");
        exit(EXIT_FAILURE);
    }
    buffer[bytesRead] = '\0'; // Null-terminate the buffer

    // Open the file for reading
    fd_read = open(buffer, O_RDONLY);
    if (fd_read == -1) {
        perror("child ");
        exit(EXIT_FAILURE);
    }

    // Allocate memory for name and date of birth
    name = (char *)malloc(sizeof(char) * 100);
    if (name == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    dob = (char *)malloc(sizeof(char) * 11);
    if (dob == NULL) {
        perror("Memory allocation failed");
        free(name);
        exit(EXIT_FAILURE);
    }

    // Read the name from the file
    while (read(fd_read, &c, 1) > 0 && c != '\n') {
        name[name_idx++] = c;
    }
    name[name_idx] = '\0';

    // Read the date of birth from the file
    while (read(fd_read, &c, 1) > 0 && c != '\n') {
        dob[dob_idx++] = c;
    }
    dob[dob_idx] = '\0';

    // Parse the date of birth
    sscanf(dob, "%d-%d-%d", &day, &month, &year);

    // Get the current year
    time(&now);
    struct tm *local = localtime(&now);
    if (local == NULL) {
        perror("Error parsing time");
        exit(EXIT_FAILURE);
    }
    current_year = local->tm_year + 1900;

    // Calculate the age
    age = current_year - year;

    // Allocate memory for age string and result string
    age_str = (char *)malloc(sizeof(char) * 4);
    sprintf(age_str, "%d", age);

    result = (char *)malloc(sizeof(char) * (strlen(name) + strlen(age_str) + 2));
    if (result == NULL) {
        free(age_str);
        free(name);
        free(dob);
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Construct the result string
    strcpy(result, name);
    strcat(result, ":");
    strcat(result, age_str);
    strcat(result, "\n");

    // Write the result to the parent process
    write(c_2_p[1], result, strlen(result));

    // Clean up
    close(fd_read);
    free(name);
    free(dob);
    free(age_str);
    free(result);

    // Wait for the child process to finish
    sleep(5);

    // Close the remaining pipe ends
    close(p_2_c[0]);
    close(c_2_p[1]);

    exit(EXIT_SUCCESS);
}

// Parent process function
void parent_process(int p_2_c[2], int c_2_p[2])
{
    pid_t child_pids[MAX_FILES];
    int num_children = 0;

    // Open the current directory
    DIR *dir = opendir(".");
    if (dir == NULL) {
        perror("Error opening directory\n");
        exit(EXIT_FAILURE);
    }

    char *filenames[MAX_FILES];
    int num_files = 0;

    // Scan the directory for files with .usp extension
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        char *ext = strrchr(entry->d_name, '.');
        if (ext != NULL && strcmp(ext, ".usp") == 0) {
            filenames[num_files] = malloc(strlen(entry->d_name) + 1);
            strcpy(filenames[num_files], entry->d_name);
            num_files++;
        }
    }

    // Create a file for writing the results
    char buffer[BUFFER_SIZE];
    int fd_write = open("result.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (fd_write == -1) {
        perror("result.txt");
        exit(EXIT_FAILURE);
    }

    closedir(dir);

    // Create child processes for each file
    for (int i = 0; i < num_files; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("Error forking");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process
            child_process(p_2_c, c_2_p, filenames[i]);
            exit(0);
        } else {
            // Parent process
            child_pids[num_children++] = pid;

            // Send the filename to the child process
            write(p_2_c[1], filenames[i], strlen(filenames[i]) + 1);

            // Wait for the child process to complete
            waitpid(pid, NULL, 0);

            // Read the result from the child process
            ssize_t bytesRead = read(c_2_p[0], buffer, BUFFER_SIZE);
            if (bytesRead == -1) {
                perror("Error reading from child process");
                exit(EXIT_FAILURE);
            }
            buffer[bytesRead] = '\0';

            // Write the result to the result.txt file
            if (write(fd_write, buffer, bytesRead) == -1) {
                perror("Error writing to result.txt");
                exit(EXIT_FAILURE);
            }

            printf("Filename: %s Complete !!\n", filenames[i]);
        }
    }

    // Free memory allocated for filenames
    for (int i = 0; i < num_files; i++) {
        free(filenames[i]);
    }

    printf("Done !!\n");
    exit(EXIT_SUCCESS);
}

int main() {
    int p_2_c[2]; // Array to hold the file descriptors for the pipe(parent to child)
    int c_2_p[2];
    pid_t pid;

    // Create the pipe
    if (pipe(p_2_c) == -1 || pipe(c_2_p) == -1) {
        perror("Error Creating pipe\n");
        exit(EXIT_FAILURE);
    }
    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        parent_process(p_2_c, c_2_p);
        waitpid(pid, NULL, 0);
    }

    return 0;
}

/**
 * REFRENCES
 * https://www.tutorialspoint.com/what-is-interprocess-communication
 * https://stackoverflow.com/questions/7867014/which-end-of-a-pipe-is-for-input-and-which-for-output
 * https://www.geeksforgeeks.org/pipe-system-call/
 * https://www.programiz.com/c-programming/string-handling-functions
 * https://www.geeksforgeeks.org/string-functions-in-c/
 * https://www.w3schools.com/c/c_strings_functions.php
 * https://beginnersbook.com/2014/01/c-strings-string-functions/
 * https://www.gnu.org/software/libc/manual/html_node/Opening-and-Closing-Files.html
 * https://www.geeksforgeeks.org/time-h-header-file-in-c-with-examples/
 * https://www.tutorialspoint.com/c_standard_library/time_h.htm
 * */