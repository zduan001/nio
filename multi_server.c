#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <strings.h>
#include <sys/wait.h>

#define SERV_PORT 1234

void wait_child(int signo) {
    while(waitpid(0, NULL, WNOHANG) > 0);
    return;
}

int main() {
    pid_t pid;
    int lfd, cfd;
    struct sockaddr_in serv_addr, client_addr;
    socklen_t client_addr_len;
    char buf[1024];
    int n, res;

    lfd = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    res = bind(lfd, 
               (struct sockaddr *)&serv_addr,
               sizeof(serv_addr));

    if (res == -1) {
        perror("bind error");
        exit(1);
    }

    res = listen(lfd, 128);
    if (res == -1) {
        perror("listen error");
        exit(1);
    }

    while(1) {
        client_addr_len = sizeof(client_addr);
        cfd = accept(lfd, 
                    (struct sockaddr *)&client_addr, 
                    &client_addr_len);
        if (cfd == -1) {
            perror("accept error");
            exit(1);
        }

        char client_ip[1024];
        printf("client ip and port %s:%d\n", 
                inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)),
                ntohs(client_addr.sin_port));

        pid = fork();
        if (pid < 0) {
            perror("fork error");
            exit(1);
        } else if (pid == 0) {
            break;
        } else {
            close(cfd);
            signal(SIGCHLD, wait_child);
        }
    }

    if (pid == 0) {
        close(lfd);
        while(1) {
            n = read(cfd, buf, sizeof(buf));
            if (n == 0) {
                close(cfd);
                exit(0);
            } else if (n== -1) {
                perror("read error");
                exit(1);
            } else {
                for (int i = 0;i < n; i++) {
                    buf[i] = toupper(buf[i]);
                }
                write(STDOUT_FILENO, buf, n);
                write(cfd, buf, n);
            }
        }
    }
    return 0;
}
