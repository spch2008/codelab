/**
* @file   echo_client.c
* @author sunpengcheng(spch2008@foxmail.com)
* @date   2016-06-06 11:38:31
* @brief
**/

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

int main(int argc, char *argv[])
{
    const char *ip = "127.0.0.1";
    const char *port = "2016";

    int res;
    struct addrinfo hint, *result;

    memset(&hint, 0, sizeof(struct addrinfo)); 
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;

    res = getaddrinfo(ip, port, &hint, &result);

    if (res != 0)
    {
        printf("get addr error\n");
        return -1;
    }

    int fd = -1;
    fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (fd < 0)
    {
        printf("get socket error\n");
        return -1;
    }

    res = connect(fd, result->ai_addr, result->ai_addrlen);
    if (res != 0)
    {
        return -1;
    }

    char buff[50] = {"Hello Server!"};
    
    int write_cnt = write(fd, buff, strlen(buff));
    if (write_cnt == strlen(buff))
    {
        printf("write to server: %s\n", buff);
    }
    else
    {
        printf("write error!\n");
    }

    int read_cnt = read(fd, buff, sizeof(buff));
    if (read_cnt > 0)
    {
        printf("recv from server: %s\n", buff);
    }
    else
    {
        printf("read error\n");
    }

    freeaddrinfo(result);
    return 0;
}
