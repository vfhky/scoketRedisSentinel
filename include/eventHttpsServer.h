/**
 * Copyright (c) 2023-07, typecodes.com (vfhky@typecodes.com)
 *
 * All rights reserved.
 *
 * Creating a https server.
 */


#ifndef __SOCKET_REDIS_SENTINEL_HTTPS_SERVER_H__
#define __SOCKET_REDIS_SENTINEL_HTTPS_SERVER_H__




#include <event2/bufferevent_ssl.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include "httpComHandle.h"






namespace socketRedisSentinel {



    #define CERTIFICATE_PATH "server.crt"
    #define PRIVATE_KEY_PATH "server.key"




    class EventHttpsServer
    {
    public:

        static EventHttpsServer& instance();

        struct evhttp * createHttpsServer(struct event_base *base);





    private:


        static struct bufferevent* bevCb(struct event_base *base, void *arg);
        static SSL_CTX *createSslContext(const char *certificatePath, const char *privatePeyPath);











    private:











    };




}







#endif

