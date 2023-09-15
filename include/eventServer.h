/**
 * Copyright (c) 2023-07, typecodes.com (vfhky@typecodes.com)
 *
 * All rights reserved.
 *
 * The entrance of creating an server like http/https/tcp.
 */


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

        int init();





    private:


        static void signalCb(evutil_socket_t sig, short event, void *arg);
        static void registerSignalCb(struct event_base *base);







    private:











    };




}







#endif

