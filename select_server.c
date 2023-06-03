#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/ioctl.h>

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    unsigned long ul = 1;
    ioctl(listener, FIONBIO, &ul);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9090);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    fd_set fdread;
    int clients[64];
    int num_clients = 0;

    char buf[256];
    char message[256];

    while (1)
    {
        FD_ZERO(&fdread);

        FD_SET(listener, &fdread);
        for (int i = 0; i < num_clients; i++)
            FD_SET(clients[i], &fdread);

        int ret = select(FD_SETSIZE, &fdread, NULL, NULL, NULL);

        if (ret <= 0)
        {
            perror("select() failed");
            break;
        }

        if (FD_ISSET(listener, &fdread))
        {
            int client = accept(listener, NULL, NULL);
            if (ret <= 0)
            {
                perror("accept() failed");
                break;
            }
            if (FD_ISSET(listener, &fdread))
            {
                if (num_clients < 64)
                {
                    int client = accept(listener, NULL, NULL);

                    if (client == -1)
                    {
                        if (errno == EWOULDBLOCK || errno == EAGAIN)
                        {
                            // Van dang cho ket noi
                            // Bo qua khong xu ly
                            continue;
                        }
                        else
                        {
                            perror("accept() failed");
                            break;
                        }
                    }
                    else
                    {
                        clients[num_clients] = client;
                        unsigned long ulong = 1;
                        ioctl(client, FIONBIO, &ulong);
                        printf("New client connected %d\n", client);
                        sprintf(message, sizeof(message), "Xin chào. Hiện có %d clients đang kết nối", num_clients);
                        num_clients++;
                        send(client, message, strlen(message), 0);
                    }
                }
            }
        }
    }

    close(listener);

    return 0;
}