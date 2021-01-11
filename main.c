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

#define SERVERPORT 8989
#define BUFSIZE 4096
#define SOCKETERROR (-1)
#define SERVER_BACKLOG 1

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

void *handle_connection(void *p_client_socket);
int create_file(const char *filename, int filesize);
int check(int exp, const char *msg);

int main(int argc, char **argv)
{
    int server_socket, client_socket, addr_size;
    SA_IN server_addr, client_addr;

    check((server_socket = socket(AF_INET, SOCK_STREAM, 0)),
          "Failed to create socket");

    //initialize the address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVERPORT);

    check(bind(server_socket, (SA *)&server_addr, sizeof(server_addr)),
          "Bind Failed!");
    check(listen(server_socket, SERVER_BACKLOG),
          "Listen Failed");

    while (true)
    {
        printf("Waiting for connections...\n");
        //wait for, and eventually accept an incoming connection
        addr_size = sizeof(SA_IN);
        check(client_socket =
                      accept(server_socket, (SA *)&client_addr, (socklen_t *)&addr_size),
              "accept failed");
        printf("Connected!\n");

        //do whatever we do with connections.
        pthread_t t;
        int *pclient = malloc(sizeof(int));
        *pclient = client_socket;
        pthread_create(&t, NULL, handle_connection, pclient);

    } //while

    return 0;
}

int check(int exp, const char *msg)
{
    if (exp == SOCKETERROR)
    {
        perror(msg);
        exit(1);
    }
    return exp;
}

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