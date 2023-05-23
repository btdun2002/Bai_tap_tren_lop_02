#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/ioctl.h>

int main(int argc, char *argv[])
{
    int sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    int receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in addr_s;
    addr_s.sin_family = AF_INET;
    addr_s.sin_addr.s_addr = inet_addr(argv[1]);
    addr_s.sin_port = htons(atoi(argv[2]));

    struct sockaddr_in addr_r;
    addr_r.sin_family = AF_INET;
    addr_r.sin_addr.s_addr = inet_addr(INADDR_ANY);
    addr_r.sin_port = htons(atoi(argv[3]));

    bind(receiver, (struct sockaddr *)&addr_r, sizeof(addr_r));

    fd_set fdread, fdtest;
    FD_ZERO(&fdread);
    FD_SET(STDIN_FILENO, &fdread);
    FD_SET(receiver, &fdread);

    char buf[256];

    while (1)
    {
        fdtest = fdread;
        int ret = select(receiver + 1, &fdtest, NULL, NULL, NULL);
        if (ret < 0)
        {
            perror("select() failed");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &fdtest))
        {
            fgets(buf, sizeof(buf), stdin);
            sendto(sender, buf, strlen(buf), 0, (struct sockaddr *)&addr_s, sizeof(addr_s));
        }

        if (FD_ISSET(receiver, &fdtest))
        {
            ret = recvfrom(receiver, buf, sizeof(buf), 0, NULL, NULL);
            buf[ret] = 0;
            printf("Received: %d", ret);
        }
    }
    close(sender);
    close(receiver);
}