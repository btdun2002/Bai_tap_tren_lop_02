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
    if (argc < 4)
    {
        fprintf(stderr, "Usage: %s <server_ip> <sender_port> <receiver_port>\n", argv[0]);
        return 1;
    }

    int sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    int receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sender == -1 || receiver == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr_s;
    addr_s.sin_family = AF_INET;
    addr_s.sin_addr.s_addr = inet_addr(argv[1]);
    addr_s.sin_port = htons(atoi(argv[2]));

    struct sockaddr_in addr_r;
    addr_r.sin_family = AF_INET;
    addr_r.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_r.sin_port = htons(atoi(argv[3]));

    if (bind(receiver, (struct sockaddr *)&addr_r, sizeof(addr_r)) == -1)
    {
        perror("bind() failed");
        return 1;
    }

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
            if (ret == -1)
            {
                perror("recvfrom() failed");
                break;
            }
            buf[ret] = 0;
            printf("Received: %s", buf);
        }
    }
    close(sender);
    close(receiver);

    return 0;
}
