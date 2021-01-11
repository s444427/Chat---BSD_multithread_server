#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <limits.h>
#include <sys/stat.h> //Linux/POSIX
#include <pthread.h>
#include <sys/sendfile.h>
#include <fcntl.h>

int create_file(const char *filename, int filesize)
{
    char *data = malloc((filesize + 1) * sizeof(char));
    time_t t;
    srand((unsigned)time(&t));
    FILE *fPtr;

    fPtr = fopen("dummy.txt", "w");

    if (fPtr == NULL)
    {
        perror("Unable to create file.\n");
        return -1;
    }

    for (int i = 0; i < filesize; i++)
    {
        data[i] = 'A' + (rand() % 26);
    }

    data[filesize] = '\0';

    fputs(data, fPtr);

    fclose(fPtr);

    printf("File created.\n");
}