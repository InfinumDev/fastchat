#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>

//NOTE(inf): THIS IS BAD CODE. JUST FIRST THOUGHT THAT CAME IN MIND

#define internal static

int close(int);

struct sockets
{
    int client1;
    int client2;
};

internal void statePrinter(int recvSocketID, char* buffer)
{
    printf("<client %d>%s", recvSocketID, buffer);
}

internal void* client1Thread(void *arg)
{   
    char buf[4096];
    struct sockets* temp = (struct sockets*)arg;
    int recvSocket = temp->client1;
    int sendSocket = temp->client2;

    for(;;)
    {
        memset(buf, '\0', sizeof(buf));
        if(recv(recvSocket, buf, sizeof(buf), 0) > 0)
        {
            statePrinter(recvSocket, buf);
            send(sendSocket, buf, sizeof(buf), 0);
        }
    }
}

internal void* client2Thread(void *arg)
{   
    char buf[4096];
    struct sockets* temp = (struct sockets*)arg;
    int recvSocket = temp->client2;
    int sendSocket = temp->client1;

    for(;;)
    {
        memset(buf, '\0', sizeof(buf));
        if(recv(recvSocket, buf, sizeof(buf), 0) > 0)
        {
            statePrinter(recvSocket, buf);
            send(sendSocket, buf, sizeof(buf), 0);
        }
    }
}

//function takes a socket descriptor as an argument and writes IPv4 to ARR array
internal void cpyIpToArrFromFd(int newfd, char* arr) 
{
    struct sockaddr_in addr;

    socklen_t addr_size = sizeof(struct sockaddr_in);
    int res = getpeername(newfd, (struct sockaddr *)&addr, &addr_size);

    strcpy(arr, inet_ntoa(addr.sin_addr));
    printf("%s saved\n", inet_ntoa(addr.sin_addr));
}

int main()
{
    char server_message[256] = "You are connected";

    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server;

    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(50000);

    if(bind(server_socket, (struct sockaddr*)&server, sizeof(server)) != 0)
    {
        printf("Couldnt bind a socket to a server\n");
    }

    listen(server_socket, 3);

    //connecting client sockets
    int client_socket1, client_socket2;
    client_socket1 = accept(server_socket, NULL, NULL);
    char client1ip[15];
    client_socket2 = accept(server_socket, NULL, NULL);
    char client2ip[15];

    //saving ip addresses
    cpyIpToArrFromFd(client_socket1, client1ip);
    cpyIpToArrFromFd(client_socket2, client2ip);

    //sending confirmation that evrthing is ok
    send(client_socket1, server_message, sizeof(server_message), 0);
    send(client_socket2, server_message, sizeof(server_message), 0);    

    //retreiving "Connected\0" message 
    char bufClient1[10];
    char bufClient2[10];
    if(recv(client_socket1, bufClient1, sizeof(bufClient1), 0) > 0)
        printf("<client1> %s\n", bufClient1);
    if(recv(client_socket2, bufClient2, sizeof(bufClient2), 0) > 0)
        printf("<client2> %s\n", bufClient2);

    struct sockets ids;
    
    ids.client1 = client_socket1;
    ids.client2 = client_socket2;

    //creating thread id from proccessing messages
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_t tid1, tid2;
    pthread_create(&tid1, &attr, client1Thread, &ids);
    pthread_create(&tid2, &attr, client2Thread, &ids);

    //lauching I/O proccess
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    close(server_socket);

    return 0;
}