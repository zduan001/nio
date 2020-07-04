#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <stdio.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 6666

int main() {
    int lfd, cfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    char buf[1024];
    int n, res;

    lfd = socket(AF_INET, SOCK_STREAM, 0); //这里应该已经bind了。
    if (lfd == -1) {
        perror("socket error");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY会主动获取当前网卡上的IP

    res = bind(lfd, 
         (struct sockaddr *)&server_addr, 
         sizeof(server_addr));
    if (res == -1) {
        perror("bind error");
        exit(1);
    }

    res = listen(lfd, 128); // listen can set the max number of client can be connected
    if (res == -1) {
        perror("listen error");
        exit(1);
    }

    client_addr_len = sizeof(client_addr);
    cfd = accept(lfd, 
                (struct sockaddr *)&client_addr, 
                &client_addr_len); // this is block call.
    // server is blocked
    if (cfd == -1) {
        perror("accept error");
        exit(1);
    }

    char client_ip[1024];
    // socklen_t client_ip_len;
    printf("client ip and port %s:%d\n", 
            inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)),
            ntohs(client_addr.sin_port));

    while(1) {
        n = read(cfd, buf, sizeof(buf)); // 另一个blocking call.
        for (int i = 0; i < n; i++) {
            buf[i] = toupper(buf[i]);
        }
        write(STDOUT_FILENO, buf, n);
        write(cfd, buf, n);
    }

    close(lfd);
    close(cfd);

    return 0;
}