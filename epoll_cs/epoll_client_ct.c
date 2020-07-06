#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#define SERV_PORT 9000
#define SERV_HOST "127.0.0.1"
#define BUF_SIZ 1024

int main() {
    int cfd;
    struct sockaddr_in serv_addr;
    socklen_t serv_addr_len;
    char buf[BUF_SIZ];

    cfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, SERV_HOST, &serv_addr.sin_addr);
    serv_addr.sin_port = htons(SERV_PORT);

    serv_addr_len = sizeof(serv_addr);

    int res = connect(cfd, (struct sockaddr *)&serv_addr, serv_addr_len);
    if (res == -1) {
        perror("bind failed");
        exit(1);
    }

    while(1) {
        fgets(buf, sizeof(buf), stdin);
        write(cfd, buf, strlen(buf));
        int n = read(cfd, buf, BUF_SIZ);
        write(STDOUT_FILENO, buf, n);
    }
    return 0;

}