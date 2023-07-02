#ifndef __LIB_EVENT_SERVER_H__
#define __LIB_EVENT_SERVER_H__




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>



#include "utils.h"



using namespace std;

namespace scoketRedisSentinel {
    class LibEventServer
    {
    public:


        static void readCb(struct bufferevent *bev, void *ctx);

        static void writeCb(struct bufferevent *bev, void *ctx);

        static void eventCb(struct bufferevent *bev, short events, void *ctx);

        static void listenerCb(struct evconnlistener *listener, evutil_socket_t fd,
                struct sockaddr *address, int socklen, void *ctx);

        int init();





    private:

    };




}







#endif

