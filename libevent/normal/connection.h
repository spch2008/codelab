/**
* @file   connection.h
* @author sunpengcheng(spch2008@foxmail.com)
* @date   2016-07-05 09:12:07
* @brief
**/

#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <event.h>

enum ConnStatus
{
    
};

struct Connection
{
    int fd;
    int left_size;
    int conn_status;

    char *read_buf;
    int   read_size;

    char *write_buf;
    int   write_size;

    struct event *revent;
    struct event *wevent;
};

#endif
