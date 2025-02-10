#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>

#define serverPort 48763
#define BACKLOG 5

pthread_t clients[BACKLOG];
pthread_t tid;

// converts all its characters to uppercase
char *convert(char *src) {
    char *iter = src;
    char *result = malloc(sizeof(src));
    char *it = result;
    if (iter == NULL) return iter;

    while (*iter) {
        *it++ = toupper(*iter++);
    }
    *it = '\0';
    return result;
}

struct client_info{
    int sockfd;
    struct sockaddr_in clientAddr;
};

void* clientSocket(void *param){
    char buf[1024];
    struct client_info * info = param;
    // continuously receives data from the client
    while (recv(info->sockfd, buf, sizeof(buf), 0)) {
        
        // break if receive "exit"
        if (strcmp(buf, "exit") == 0) {
            memset(buf, 0, sizeof(buf));
            break;
        }

        // converts to uppercase
        char *conv = malloc(sizeof(buf));
        memcpy(conv, convert(buf), sizeof(buf));

        // show data
        printf("get message from [%s:%d]: ",
                inet_ntoa(info->clientAddr.sin_addr), ntohs(info->clientAddr.sin_port));
        printf("%s -> %s\n", buf, conv);

        // send back to client
        if (send(info->sockfd, conv, sizeof(conv), 0) < 0) {
            printf("send data to %s:%d, failed!\n", 
                    inet_ntoa(info->clientAddr.sin_addr), ntohs(info->clientAddr.sin_port));
            memset(buf, 0, sizeof(buf));
            free(conv);
            break;
        }

        // clear message buffer
        memset(buf, 0, sizeof(buf));
        free(conv);
    }

    // close reply socket and check if it is closed successfully
    if (close(info->sockfd) < 0) {
        perror("close socket failed!");
    }else{
        printf("Socket closed from %s:%d success!\n", 
            inet_ntoa(info->clientAddr.sin_addr), ntohs(info->clientAddr.sin_port));
    }
    free(info);
    return NULL;

}

static void skeleton_daemon()
{
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    /* Catch, ignore and handle signals */
    //TODO: Implement a working signal handler */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/");

    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
        close (x);
    }

    /* Open the log file */
    openlog ("firstdaemon", LOG_PID, LOG_DAEMON);
}

int main() 
{
    skeleton_daemon();

    // message buffer
    char buf[1024] = {0};

    int client_index = 0;

    // create socket
    int socket_fd = socket(PF_INET , SOCK_STREAM , 0);
    if (socket_fd < 0){
        syslog(LOG_ERR, "fail to create");
        exit(EXIT_FAILURE);
    }
    
    // server address
    struct sockaddr_in serverAddr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(serverPort)
    };
    
    //  binds serverAddr to the specified port
    if (bind(socket_fd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind socket failed!");
        close(socket_fd);
        exit(0);
    }

    // initialize and wait for connect
    // backlog = 5
    // return -1 for listen error
    if (listen(socket_fd, BACKLOG) == -1) {
        printf("socket %d listen failed!\n", socket_fd);
        close(socket_fd);
        exit(0);
    }

    printf("server [%s:%d] --- ready\n", 
            inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

    while(1) {

        // create new socket attribute
        int reply_sockfd;
        struct sockaddr_in clientAddr;
        int client_len = sizeof(clientAddr);

        // get connected socket from complete connection queue
        // use reply_sockfd to communicate with client
	
        reply_sockfd = accept(socket_fd, (struct sockaddr *)&clientAddr, &client_len);
        printf("Accept connect request from [%s:%d]\n", 
                inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        // create thread attribute between each other
        struct client_info *newClientinfo = malloc(sizeof(struct client_info));
        memcpy(&newClientinfo->sockfd, &reply_sockfd, sizeof(int));
        memcpy(&newClientinfo->clientAddr, &clientAddr, sizeof(struct sockaddr_in));

        if (pthread_create(&clients[client_index++], NULL, clientSocket, newClientinfo) != 0){
            printf("Failed to create new Thread!");
        }

    }

    // close socket and check if it is closed successfully
    if (close(socket_fd) < 0) {
        perror("close socket failed!");
    }
    return 0;
}
