#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <unistd.h>

#define MAXLINE 10
#define SERV_PORT 9000
#define RESEVENT_SIZ 10

int main() {
    struct sockaddr_in servaddr, clientaddr;
    socklen_t clientaddr_len;
    int listenfd, connfd;
    char buf[MAXLINE];
    char str[INET_ADDRSTRLEN];
    int efd;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    listen(listenfd, 128);

    struct epoll_event event;
    struct epoll_event resevent[RESEVENT_SIZ];
    int res, len;

    efd = epoll_create(MAXLINE);
    event.events = EPOLLIN;   // 边沿触发
    // event.events = EPOLLIN|EPOLLET; // 水平触发

    printf("accepting connection ....");
    clientaddr_len = sizeof(clientaddr);
    connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientaddr_len);
    printf("received from %s at Port %d\n",
        inet_ntop(AF_INET, &clientaddr.sin_addr, str, sizeof(str)),
        ntohs(clientaddr.sin_port));

    event.data.fd = connfd;
    epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &event);

    while(1) {
        res = epoll_wait(efd, resevent, RESEVENT_SIZ, -1);
        printf("res %d\n", res);
        if (resevent[0].data.fd == connfd) {
            len = read(connfd, buf, MAXLINE/2);
            for (int i = 0;i < len; i++) {
                buf[i] = toupper(buf[i]);
            }
            write(STDOUT_FILENO, buf, len);
            write(connfd, buf, len);
        }
        // sleep(5);
    }
    return 0;

}
