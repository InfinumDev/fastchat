#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

//NOTE(inf): THIS IS BAD CODE. JUST FIRST THOUGHT THAT CAME IN MIND

#define internal static
#define global_variable static

internal int close(int);

global_variable char inbuf[4096];
global_variable char serverOutbuf[4096];

internal void* printOutputProces(void* arg)
{
    int socket = *((int*)arg);
    for(;;)
    {
        if(recv(socket, serverOutbuf, sizeof(serverOutbuf), MSG_PEEK) > 0)
        {
            memset(serverOutbuf, '\0', sizeof(char));
            recv(socket, serverOutbuf, sizeof(serverOutbuf), 0);
            printf("%s\n", serverOutbuf);
        }
        memset(serverOutbuf, '\0', sizeof(serverOutbuf));
    }

    pthread_exit(0);
}

internal void* sendInputProces(void* arg)
{
    int socket = *((int*)arg);

    for(;;)
    {
        printf(">");
        gets(inbuf);
        if(strlen(inbuf) > 0)
        {
            send(socket, inbuf, sizeof(inbuf), 0);
            memset(inbuf, '\0', sizeof(inbuf));
        }
    }

    pthread_exit(0);
}

int main(int argc, char* argv)
{
    int socket_desc;
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    char server_reply[2000];

    if (socket_desc == -1){
        printf("Couldn't create a socket\n");
    }

    char ip[14] = "192.168.0.104\0";

    struct sockaddr_in server;
    
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(50000);

    //Connect to remote server
    if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        printf("connect error to %s\n", ip);
        return 1;
    }
    printf("Connected to %s\n", ip);

    //receive a reply from server
    if ( recv(socket_desc, server_reply, 2000, 0) < 0 )
    {
        printf("Receive failed\n");
        return 1;
    }
    printf("Reply received: <%s>\n", server_reply);

    char mes[9] = "Connected";
    send(socket_desc, mes, sizeof(mes), 0);

    //creating thread ids
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_t tid1, tid2;
    pthread_create(&tid1, &attr, sendInputProces, &socket_desc);
    pthread_create(&tid2, &attr, printOutputProces, &socket_desc);


    //lauching I/O proccess
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    //closing the socket
    close(socket_desc);

    return 0;
}