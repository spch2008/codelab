/**
* @file   echo_server.c
* @author sunpengcheng(spch2008@foxmail.com)
* @date   2016-06-01 16:30:30
* @brief
**/

#include <event.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>

struct client
{
    struct event *revent;
    struct event *wevent;
};

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

void read_cb(int fd, short ev_kind, void *arg)
{
}

void accept_cb(int fd, short ev_kind, void *arg)
{
    socklen_t addrlen;
    struct sockaddr addr;

    while (1)
    {
        int client_fd = accept(fd, &addr, &addrlen);
        if (client_fd < 0)
        {
            if (errno == EAGAIN)
            {
                continue;
            }
            else
            {
                break;
            }
        }
        else
        {
            struct client *c = (struct client*)calloc(1, sizeof(struct client));
            if (c == NULL)
            {
                continue;
            }

            struct event *ev = (struct event*)calloc(1, sizeof(struct event));
            if (ev == NULL)
            {
                free(c);
                continue;
            }

            c->revent = ev;
            event_set(ev, client_fd, EV_READ, read_cb, c);
            event_add(ev, NULL);
        }
    }
}

int main(int argc, char argv[])
{
    return 0;
}
