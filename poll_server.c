#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>

#define MAXLINE 80
#define SERV_PORT 8000
#define OPEN_MAX 1024

int main() {
    int i, j, maxi, listenfd, connfd, sockfd;
    int nready;
    ssize_t n;

    char buf[MAXLINE], str[INET_ADDRSTRLEN];
    socklen_t client_len;
    struct pollfd client[OPEN_MAX];
    struct sockaddr_in client_addr, serv_addr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    // error check listenfd

    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_PORT);

    bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    listen(listenfd, 128);

    client[0].fd = listenfd;
    client[0].events = POLLIN;

    for (i = 1; i < OPEN_MAX; i++) client[i].fd = -1; // use -1 because 0 is STDOUT

    maxi = 0;

    while(1) {
        nready = poll(client, maxi+1, -1);
        
        // 专门处理 listenfd, 来 accept connections
        if (client[0].revents & POLLIN) {
            client_len = sizeof(client_addr);
            connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_len);
            write(STDOUT_FILENO, "abc", sizeof("abc"));

            // why printf doesn't work?
            printf("received connect from %s:%d",
                    inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, str, sizeof(str)),
                    ntohs(client_addr.sin_port));
                    
            for(i = 1; i < OPEN_MAX; i++) {
                if (client[i].fd < 0) {
                    client[i].fd = connfd;
                    break;
                }
            }

            if (i == OPEN_MAX) perror("too many client");
            client[i].events = POLLIN;
            if (i > maxi) maxi = i;
            if (--nready <= 0) continue;
        }

        // 然后处理所有 client connection 的 fd. 
        for (i = 1; i <= maxi; i++) {
            if ((sockfd = client[i].fd) < 0) continue;
            
            if (client[i].revents & POLLIN) {
                if ((n = read(sockfd, buf, MAXLINE)) < 0) {
                    /* connect reset by client*/
                    if (errno == ECONNRESET) {
                        printf("client [%d] aborted connection\n", i);
                        close(sockfd);
                        client[i].fd = -1;
                    } else {
                        perror("read error");
                        exit(1);
                    }
                } else if (n ==0) {
                    printf("client [%d] closed connection.", i);
                    close(sockfd);
                    client[i].fd = -1;
                } else {
                    for (j = 0; j < n; j++) {
                        buf[j] = toupper(buf[j]);    
                    }
                    write(STDOUT_FILENO, buf, n);
                    write(sockfd, buf, n);
                }
                if (--nready <= 0) break;

            }
        }
    }
    return 0;
}