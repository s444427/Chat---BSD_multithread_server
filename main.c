#include "create_file.h"
#include "connection.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/sendfile.h>

#define SERVERPORT 8989
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



