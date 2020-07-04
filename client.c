#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#define SERV_PORT 6666
#define SERV_HOST "127.0.0.1"
#define BUFSIZE 1024

int main() {
    int cfd;
    struct sockaddr_in serv_addr;
    socklen_t serv_addr_len;
    char buf[BUFSIZE];

    cfd = socket(AF_INET, SOCK_STREAM, 0); // unix can auto bind.

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, SERV_HOST, &serv_addr.sin_addr);

    serv_addr_len = sizeof(serv_addr);
    connect(cfd, (struct sockaddr *)&serv_addr, serv_addr_len);

    while(1) {
        fgets(buf, sizeof(buf), stdin);
        write(cfd, buf, strlen(buf));
        int n = read(cfd, buf, sizeof(buf));
        write(STDOUT_FILENO, buf, n);
    }

    close(cfd);

    return 0;
}