/**
 * Copyright (c) 2023-07, typecodes.com (vfhky@typecodes.com)
 *
 * All rights reserved.
 *
 * Creating a http server.
 */


#include "eventHttpsServer.h"





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



    EventHttpsServer &EventHttpsServer::instance() {
        static EventHttpsServer instance;
        return instance;
    }

    /**
     * This callback is responsible for creating a new SSL connection
     * and wrapping it in an OpenSSL bufferevent.  This is the way
     * we implement an https server instead of a plain old http server.
     */
    struct bufferevent* EventHttpsServer::bevCb(struct event_base *base, void *arg) {
        struct bufferevent* r;
        SSL_CTX *ctx = static_cast<SSL_CTX *>(arg);

        r = bufferevent_openssl_socket_new (base, -1, SSL_new (ctx), BUFFEREVENT_SSL_ACCEPTING, BEV_OPT_CLOSE_ON_FREE);
        return r;
    }

    SSL_CTX *EventHttpsServer::createSslContext(const char *certificatePath, const char *privatePeyPath) {
        // https://wiki.openssl.org/index.php/Library_Initialization
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        SSL_library_init();
#else
        OPENSSL_init_ssl(0, NULL);
#endif

        SSL_CTX *ctx;
        ctx = SSL_CTX_new(SSLv23_server_method());

        if (!ctx) {
            unsigned long err = ERR_get_error();
            char errMsg[256] = {0x00};
            ERR_error_string_n(err, errMsg, sizeof(errMsg));
            LOG(Error, "SSL_CTX_new failed", errMsg);
            return NULL;
        }

        if (!SSL_CTX_use_certificate_file(ctx, certificatePath, SSL_FILETYPE_PEM)) {
            unsigned long err = ERR_get_error();
            char errMsg[256] = {0x00};
            ERR_error_string_n(err, errMsg, sizeof(errMsg));
            LOG(Error, "SSL_CTX_use_certificate_file failed", certificatePath, errMsg);
            SSL_CTX_free(ctx);
            return NULL;
        }

        if (!SSL_CTX_use_PrivateKey_file(ctx, privatePeyPath, SSL_FILETYPE_PEM)) {
            unsigned long err = ERR_get_error();
            char errMsg[256] = {0x00};
            ERR_error_string_n(err, errMsg, sizeof(errMsg));
            LOG(Error, "SSL_CTX_use_PrivateKey_file failed", privatePeyPath, errMsg);
            SSL_CTX_free(ctx);
            return NULL;
        }

        return ctx;
    }




    struct evhttp * EventHttpsServer::createHttpsServer(struct event_base *base) {
        if (NULL == base) {
            LOG(Error, "createHttpsServer failed due to NULL event_base", \
                    evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
            return NULL;
        }

        SSL_CTX *ctx = createSslContext(CERTIFICATE_PATH, PRIVATE_KEY_PATH);
        if (!ctx) {
            LOG(Error, "createSslContext failed", \
                    evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
            return NULL;
        }


        // Create a new evhttp object to handle requests.
        struct evhttp *https = evhttp_new(base);
        if (!https) {
            LOG(Error, "evhttp_new failed", evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
            return NULL;
        }

        // This is the magic that lets evhttp use SSL.
        evhttp_set_bevcb(https, EventHttpsServer::bevCb, ctx);

        // make router
        HttpComHandle::instance().httpRouter(https);

        struct evhttp_bound_socket *handle = evhttp_bind_socket_with_handle(https, SERVER_LISTEN_IP, HTTPS_LISTEN_PORT);
        if (!handle) {
            LOG(Error, "couldn't bind to port ", HTTPS_LISTEN_PORT, evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
            return NULL;
        }

        LOG(Info, "https server listening on ", SERVER_LISTEN_IP, ":", HTTPS_LISTEN_PORT);
        return https;
    }








}


