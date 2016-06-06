/**
* @file   echo_server.c
* @author sunpengcheng(spch2008@foxmail.com)
* @date   2016-06-01 16:30:30
* @brief
**/

#include <event.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
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

void del_client(struct client *c)
{
    if (c->revent != NULL)
    {
        event_del(c->revent);
    }

    if (c->wevent != NULL)
    {
        event_del(c->wevent);
    }

    free(c);
}

void write_cb(int fd, short ev_kind, void *arg)
{
    char buff[50] = {"Hello Client!"};
    ssize_t write_cnt;

    write_cnt = write(fd, buff, strlen(buff));
    if (write_cnt != strlen(buff))
    {
        printf("write error\n");
    }
    else
    {
        printf("write to client: %s\n", buff);
    }

    del_client((struct client*)arg);
}

void read_cb(int fd, short ev_kind, void *arg)
{
    char buff[50]; 
    ssize_t read_cnt;

    struct client *c = (struct client*)arg;

    read_cnt = read(fd, buff, sizeof(buff));
    if (read_cnt <= 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            del_client(c);
        }
    }
    else
    {
        c->wevent = (struct event*)calloc(1, sizeof(struct event));
        if (c->wevent == NULL)
        {
            del_client(c);
        }

        event_set(c->wevent, fd, EV_WRITE, write_cb, c);
        event_add(c->wevent, NULL);

        printf("recv from client: %s\n", buff);
    }
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
            event_set(c->revent, client_fd, EV_READ, read_cb, c);
            event_add(c->revent, NULL);
        }
    }
}

int main(int argc, char argv[])
{
    const char *ip = "127.0.0.1";
    const char *port = "2016";

    int sfd = create_and_bind(ip, port);
    if (sfd == -1)
    {
        return -1;
    }

    struct event_base *base = event_init();
    if (base == NULL)
    {
        return -1;
    }


    struct event ev;
    event_set(&ev, sfd, EV_READ|EV_PERSIST, accept_cb, NULL);
    event_add(&ev, NULL);

    event_base_dispatch(base);
    return 0;
}
