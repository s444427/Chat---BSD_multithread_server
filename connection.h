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

#define BUFSIZ 4096

void *handle_connection(void *p_client_socket)
{

    int client_socket = *((int *)p_client_socket);
    free(p_client_socket);
    char message[BUFSIZ];
    char delim[] = " ";
    long offset;
    int remain_data;
    ssize_t len;
    struct stat file_stat;
    int fd;
    int sent_bytes = 0;

    bool cclient = false;
    bool cSharpClient = false;

    bzero(message, BUFSIZ);
    recv(client_socket, message, BUFSIZ, 0);

    if (strcmp(message, "c") == 0)
    {
        cclient = true;
        printf("working with c client\n");
    }
    else if (strcmp(message, "csharp") == 0)
    {
        cSharpClient = true;
        printf("working with csharp client\n");
    }
    bzero(message, BUFSIZ);

    recv(client_socket, message, BUFSIZ, 0);

    char *arg1 = malloc(128 * sizeof(char));

    char *arg3 = malloc(128 * sizeof(char));
    char actualpath[PATH_MAX + 1];
    size_t bytes_read = 0;

    char *ptr = strtok(message, delim);

    strcpy(arg1, ptr);

    if (strncmp(arg1, "exit", 4) == 0)
    {
        printf("client exit\n");
        close(client_socket);
        free(arg1);
        free(arg3);
        return NULL;
    }

    if (!(strcmp(arg1, "dummy") == 0))
    {
        printf("unknown command\n");
        close(client_socket);
        free(arg1);
        free(arg3);
        return NULL;
    }

    ptr = strtok(NULL, delim);
    strcpy(arg3, ptr);

    if (strcmp(arg3, "") == 0 || atoi(arg3) == 0)
    {
        printf("no file size provided\n");
        close(client_socket);
        free(arg1);
        free(arg3);
        return NULL;
    }

    bzero(message, BUFSIZ);

    int n;
    char data[BUFSIZ] = {0};

    int filesize = atoi(arg3);
    create_file("please", filesize);

    long fileLength, sent, sentTotal, read;
    struct stat fileinfo;
    FILE *fp;

    if (stat("dummy.txt", &fileinfo) < 0)
    {
        printf("File does not exist\n");
        fclose(fp);
        close(client_socket);

        free(arg1);
        free(arg3);
        return NULL;
    }

    if (fileinfo.st_size == 0)
    {
        printf("File size is 0\n");
        fclose(fp);
        close(client_socket);

        free(arg1);
        free(arg3);
        return NULL;
    }

    printf("File length: %ld\n", fileinfo.st_size);

    fileLength = htonl((long)fileinfo.st_size);
    char fileLenString[255];
    sprintf(fileLenString, "%ld", fileinfo.st_size);

    if (cclient)
    {
        if (send(client_socket, &fileLength, sizeof(long), 0) != sizeof(long))
        {
            printf("Error sending file size (C client)\n");
            fclose(fp);
            close(client_socket);

            free(arg1);
            free(arg3);
            return NULL;
        }
    }
    if (cSharpClient)
    {
        if (send(client_socket, fileLenString, sizeof(fileLenString), 0) != sizeof(fileLenString))
        {
            printf("Error sending file size (C# client)\n");
            fclose(fp);
            close(client_socket);

            free(arg1);
            free(arg3);
            return NULL;
        }
    }


    fileLength = fileinfo.st_size;
    sentTotal = 0;
    fp = fopen("dummy.txt", "r");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        fclose(fp);
        close(client_socket);

        free(arg1);
        free(arg3);
        return NULL;
    }

    while (sentTotal < fileLength)
    {
        read = fread(data, 1, BUFSIZ, fp);
        sent = send(client_socket, data, read, 0);
        if (read != sent)
            break;
        sentTotal += sent;
        printf("sent %ld bites\n", sentTotal);
    }

    if (sentTotal == fileLength)
    {
        printf("File sent correctly\n");
    }
    else
    {
        printf("Error sending file\n");
    }

    fclose(fp);
    close(client_socket);

    free(arg1);
    free(arg3);

    return NULL;
}