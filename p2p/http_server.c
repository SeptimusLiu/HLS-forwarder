//
//  http_server.c
//  p2p
//
//  Created by septimus on 15/6/2.
//  Copyright (c) 2015年 SEPTIMUS. All rights reserved.
//

#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

int httpserver_bindsocket(int port, int backlog)
{
    int r;
    int nfd;
    nfd = socket(AF_INET, SOCK_STREAM, 0);
    if (nfd < 0)
        return -1;
    
    int one = 1;
    r = setsockopt(nfd, SOL_SOCKET, SO_REUSEADDR, (char*)&one, sizeof(int));
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    r = bind(nfd, (struct sockaddr*)&addr, sizeof(addr));
    if (r < 0)
        return -1;
    r = listen(nfd, backlog);
    if (r < 0)
        return -1;
    
    int flags;
    if ((flags = fcntl(nfd, F_GETFL, 0)) < 0
        || fcntl(nfd, F_SETFL, flags | O_NONBLOCK) < 0)
        return -1;
    
    return nfd;
}

