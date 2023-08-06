#ifndef __SCOKET_REDIS_SENTINEL_HTTP_SERVER_H__
#define __SCOKET_REDIS_SENTINEL_HTTP_SERVER_H__




#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/http_compat.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/keyvalq_struct.h>



#include "utils.h"



using namespace std;

namespace socketRedisSentinel {





    class EventHttpServer
    {
    public:

        static EventHttpServer& instance();

        static void httpReqEntrance(struct evhttp_request *req, void *arg);

        struct evhttp * createHttpServer(struct event_base *base);





    private:

        static void handleGetReq(struct evhttp_request *req, void *arg);
        static void handlePostReq(struct evhttp_request *req, void *arg);

        static const void dumpHeaders(const struct evhttp_request *req);
        static const map<string, string> dumpGetParams(struct evhttp_request *req);
        static const map<string, string> dumpPostParams(struct evhttp_request *req);









    private:











    };




}







#endif

