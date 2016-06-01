/**
* @file   echo_server.c
* @author sunpengcheng(spch2008@foxmail.com)
* @date   2016-06-01 16:30:30
* @brief
**/

#include <event.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>

int create_and_bind(const char *ip, const char *port)
{
    struct addrinfo hint, *result;
    int res, sfd = -1;

    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family   = AF_INET;
    hint.ai_socktype = SOCK_STREAM;

    do
    {
        res = getaddrinfo(ip, port, &hint, &result); 
        if (res != 0)
        {
            break;
        }

        sfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (sfd == -1)
        {
            break;
        }

        res = bind(sfd, result->ai_addr, result->ai_addrlen);
        if (res != 0)
        {
            break;
        }

        res = listen(sfd, SOMAXCONN);
        if (res != 0)
        {
            break;
        }

    } while (0);

    freeaddrinfo(result);

    if (res == 0 && evutil_make_socket_nonblocking(sfd) == 0)
    {
        return sfd;
    }
    else
    {
        return -1;
    }
}

int main(int argc, char argv[])
{
    return 0;
}
