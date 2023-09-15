/**
 * Copyright (c) 2023-07, typecodes.com (vfhky@typecodes.com)
 *
 * All rights reserved.
 *
 * Creating a http server.
 */


#include "eventHttpServer.h"




/**
 * GET ： $ curl "typecodes.com:10087?type=1&ip=sentinel.typecodes.com&port=20066&redisType=1&poolName=revenueGameTest_001&hashKey=1111"
 * POST post data：
 * $ curl -X POST -d 'type=1&ip=sentinel.typecodes.com&port=20066&redisType=1&poolName=revenueGameTest_001&hashKey=1111' typecodes.com:10087
 * $ curl -s -H "Content-Type: application/json" -X POST -d '{"type":1,"ip":"sentinel.typecodes.com","port":"20066","redisType":1,"poolName":"revenueGameTest_001","hashKey":"1111"}' typecodes.com:10087
 * $ data.json file： $ curl -H "Content-Type: application/json" -X POST -d @data.json  http://typecodes.com:8090

 * request command not support will send 400 status code:
 * root@4d1ef2bcb408:~/testwork/testLibEvent# curl -XDELETE "http://typecodes.com:8090?type=1&ip=typecodes&port=20066"
    <HTML><HEAD>
    <TITLE>400 Bad Request</TITLE>
    </HEAD><BODY>
    <H1>Bad Request</H1>
    </BODY></HTML>
    root@4d1ef2bcb408:~/testwork/testLibEvent#
 */




namespace socketRedisSentinel {



    EventHttpServer &EventHttpServer::instance() {
        static EventHttpServer instance;
        return instance;
    }




    struct evhttp * EventHttpServer::createHttpServer(struct event_base *base) {
        if (NULL == base) {
            LOG(Error, "createHttpServer failed due to NULL event_base", \
                    evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
            return NULL;
        }


        // Create a new evhttp object to handle requests.
        struct evhttp *http = evhttp_new(base);
        if (!http) {
            LOG(Error, "evhttp_new failed", evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
            return NULL;
        }

        HttpComHandle::instance().httpRouter(http);

        struct evhttp_bound_socket *handle = evhttp_bind_socket_with_handle(http, SERVER_LISTEN_IP, HTTP_LISTEN_PORT);
        if (!handle) {
            LOG(Error, "couldn't bind to port ", HTTP_LISTEN_PORT, evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
            return NULL;
        }

        LOG(Info, "http server listening on ", SERVER_LISTEN_IP, ":", HTTP_LISTEN_PORT);
        return http;
    }








}


