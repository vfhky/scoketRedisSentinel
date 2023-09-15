/**
 * Copyright (c) 2023-07, typecodes.com (vfhky@typecodes.com)
 *
 * All rights reserved.
 *
 * Creating a tcp server.
 */


#ifndef __SOCKET_REDIS_SENTINEL_EVENT_TCP_SERVER_H__
#define __SOCKET_REDIS_SENTINEL_EVENT_TCP_SERVER_H__




#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include "clientReqInfo.h"




namespace socketRedisSentinel {





    class EventTcpServer
    {
    public:

        static EventTcpServer& instance();


        static void readCb(struct bufferevent *bev, void *ctx);

        static void writeCb(struct bufferevent *bev, void *ctx);

        static void eventCb(struct bufferevent *bev, short event, void *ctx);

        static void listenerCb(struct evconnlistener *listener, evutil_socket_t fd,
                struct sockaddr *address, int socklen, void *ctx);

        int init();

        struct evconnlistener * createTcpServer(struct event_base *base);





    private:

        static void trimCR(char *buf);
        static void trimCR(std::string &buf);

        static ClientReqInfo pharseReq(const std::string &req);







    private:











    };




}







#endif

