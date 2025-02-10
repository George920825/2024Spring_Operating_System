#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define serverPort 48763
#define serverIP "127.0.0.1"

int main() 
{
    // message buffer
    char buf[1024] = {0};
    char recvbuf[1024] = {0};
    
    // Create socket
    int socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        printf("Create socket fail!\n");
        return -1;
    }

    // Define server address
    struct sockaddr_in serverAddr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr(serverIP),
        .sin_port = htons(serverPort)
    };
    int len = sizeof(serverAddr);

    // try to connect to server, launch tcp connection
    // return -1 for server not start listening yet
    if (connect(socket_fd, (struct sockaddr *)&serverAddr, len) == -1) {
        printf("Connect server failed!\n");
        close(socket_fd);
        exit(0);
    }

    printf("Connect server [%s:%d] success\n",
            inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

    while (1) {
        // input data to buffer
        printf("Please input your message: ");
        scanf("%s", buf);

        // send to server
        if (send(socket_fd, buf, sizeof(buf), 0) < 0) {
            printf("send data to %s:%d, failed!\n", 
                    inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
            memset(buf, 0, sizeof(buf));
            break;
        }
        
        // break if receive "exit"
        if (strcmp(buf, "exit") == 0)
            break;

        // clear message buffer
        memset(buf, 0, sizeof(buf));

        // wait response from server
        if (recv(socket_fd, recvbuf, sizeof(recvbuf), 0) < 0) {
            printf("recv data from %s:%d, failed!\n", 
                    inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
            break;
        }

        // show server address and the data receive
        printf("get receive message from [%s:%d]: %s\n", 
                inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port), recvbuf);
        memset(recvbuf, 0, sizeof(recvbuf));
    }

    // close socket and check if it is closed successfully
    if (close(socket_fd) < 0) {
        perror("close socket failed!");
    }
    return 0;
}
