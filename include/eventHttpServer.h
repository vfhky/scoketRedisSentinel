/**
 * Copyright (c) 2023-07, typecodes.com (vfhky@typecodes.com)
 *
 * All rights reserved.
 *
 * Creating a http server.
 */


#ifndef __SOCKET_REDIS_SENTINEL_HTTP_SERVER_H__
#define __SOCKET_REDIS_SENTINEL_HTTP_SERVER_H__





#include "httpComHandle.h"





namespace socketRedisSentinel {




    class EventHttpServer
    {
    public:

        static EventHttpServer& instance();

        struct evhttp * createHttpServer(struct event_base *base);





    public:










    private:











    };




}







#endif

