#ifndef __SCOKET_REDIS_SENTINEL_HTTP_SERVER_H__
#define __SCOKET_REDIS_SENTINEL_HTTP_SERVER_H__





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

