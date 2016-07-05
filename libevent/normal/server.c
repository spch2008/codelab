/**
* @file   server.c
* @author sunpengcheng(spch2008@foxmail.com)
* @date   2016-07-05 09:19:12
* @brief
**/

int create_and_bind(const char *ip, const char *port)
{
    struct addrinfo hint, *result = NULL;
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
