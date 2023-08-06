#ifndef __SCOKET_REDIS_SENTINEL_EVENT_SERVER_H__
#define __SCOKET_REDIS_SENTINEL_EVENT_SERVER_H__




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>



#include "utils.h"



using namespace std;

namespace socketRedisSentinel {





    class EventServer
    {
    public:


        static void readCb(struct bufferevent *bev, void *ctx);

        static void writeCb(struct bufferevent *bev, void *ctx);

        static void eventCb(struct bufferevent *bev, short event, void *ctx);

        static void listenerCb(struct evconnlistener *listener, evutil_socket_t fd,
                struct sockaddr *address, int socklen, void *ctx);

        int init();





    private:

        static void trimCR(char *buf);

        struct evconnlistener * createTcpServer(struct event_base *base);







    private:











    };




}







#endif

