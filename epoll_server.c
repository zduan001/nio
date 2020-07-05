#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include <ctype.h>

#define MAX_LINE 8192
#define SERV_PORT 8000
#define OPEN_MAX 1024

void perr_exit(const char* str) {
    perror(str);
    exit(1);
}

int main() {
    int i, listenfd, connfd, sockfd;
    int n;
    ssize_t nready, efd, res;
    char buf[MAX_LINE], str[INET_ADDRSTRLEN];
    socklen_t client_len;

    struct sockaddr_in client_addr, serv_addr;
    struct epoll_event tep, ep[OPEN_MAX];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    // 设置端口复用，after server close, the TCP statu is time_wait. it can be reused.
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_PORT);

    bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    listen(listenfd, 128);

    // epoll related
    efd = epoll_create(OPEN_MAX);
    if (efd == -1) {
        perr_exit("epoll creation error");
    }
    tep.events = EPOLLIN, tep.data.fd = listenfd;
    res = epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &tep);
    if (res == -1) {
        perr_exit("epoll_ctl error");
    }

    while(1) {
        // 个block的call。
        nready = epoll_wait(efd, ep, OPEN_MAX, -1);
        if (nready == -1) perr_exit("epoll_wait error");

        for (i = 0; i < nready; i++) {
            if (!(ep[i].events & EPOLLIN))  continue; // don't care non POLLIN

            if (ep[i].data.fd == listenfd) { // server fd, do accept
                client_len = sizeof(client_addr);
                connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_len);

                printf("received connection from %s: %d\n",
                    inet_ntop(AF_INET, &client_addr.sin_addr, str, sizeof(str)),
                    ntohs(client_addr.sin_port));

                tep.events = EPOLLIN, tep.data.fd = connfd;
                res = epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &tep);
                if (res == -1) perr_exit("epoll_ctl connfd error");
            } else {// 这是client connection， 应该读了。
                sockfd = ep[i].data.fd;
                n = read(sockfd, buf, MAX_LINE);
                if (n == 0) { // client closed. 
                    res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL); // 删除
                    if (res == -1) perr_exit("remove sockfd error");
                    close(sockfd);
                    printf("client [%d] closed.\n", sockfd);
                } else if (n < 0) { // read error
                    res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL); // 删除
                    close(sockfd);
                    printf("client [%d] error and closed", sockfd);
                } else {
                    for (i = 0;i < n; i++) {
                        buf[i] = toupper(buf[i]);
                    }
                    write(sockfd, buf, n);
                    write(STDOUT_FILENO, buf, n);
                }
            }
        }
        
    }
    return 0;
}