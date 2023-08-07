#include "eventHttpServer.h"




/**
 * GET ： curl "http://typecodes.com:8090?id=001&name=typecodes&phone=1111"
 * POST post data：
 * 1. curl  -X POST -d 'id=1111&name=typecodes&phone=1111'  http://typecodes.com:8090
 * 2. curl -H "Content-Type: application/json" -X POST -d '{"id": "1111", "name":"typecodes", "phone":"1111"}'  http://typecodes.com:8090
 * 3. data.json file： $ curl -H "Content-Type: application/json" -X POST -d @data.json  http://typecodes.com:8090

 * request command not support will send 400 status code:
 * root@4d1ef2bcb408:~/testwork/testLibEvent# curl -XDELETE "http://typecodes.com:8090?id=1111&name=typecodes&phone=1111"
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




    const map<string, string> EventHttpServer::dumpGetParams(struct evhttp_request *req) {
        map<string, string> getParams;

        const char* requestUri = evhttp_request_get_uri(req);
        struct evhttp_uri* uri = evhttp_uri_parse(requestUri);
        if (!uri) {
            LOG(Warn, "Failed to parse URI.");
            evhttp_send_reply(req, HTTP_BADREQUEST, "Bad Request", NULL);
            return getParams;
        }

        // do not use evhttp_parse_query_str(requestUri, &params); for it will return key of "/?id"
        struct evkeyvalq params;
        evhttp_parse_query_str(evhttp_uri_get_query(uri), &params);
        for (struct evkeyval* header = params.tqh_first; header; header = header->next.tqe_next) {
            getParams[header->key] = header->value;
        }

        /**
         * get special param :
         *   const char* id = evhttp_find_header(&params, "id");
        */

        evhttp_clear_headers(&params);

        LOG(Info, Utils::printMap(getParams));
        return getParams;
    }


    // handle get command
    void EventHttpServer::handleGetReq(struct evhttp_request *req, void *arg) {


        struct evbuffer *returnbuffer = evbuffer_new();

        // /?id=001&name=typecodes&phone=1111
        const char* requestUri = evhttp_request_get_uri(req);
        LOG(Info, requestUri);

        EventHttpServer::dumpGetParams(req);

        evbuffer_add_printf(returnbuffer, "Received a GET request for %s\n", requestUri);
        evhttp_send_reply(req, HTTP_OK, "OK", returnbuffer);
        evbuffer_free(returnbuffer);
    }

    const map<string, string> EventHttpServer::dumpPostParams(struct evhttp_request *req) {
        struct evkeyvalq params;
        struct evbuffer* requestBuffer = evhttp_request_get_input_buffer(req);
        evhttp_parse_query_str((char*)evbuffer_pullup(requestBuffer, -1), &params);
        map<string, string> postParams;
        for (struct evkeyval* header = params.tqh_first; header; header = header->next.tqe_next) {
            postParams[header->key] = header->value;
        }
        /**
         * get special param
            const char* name = evhttp_find_header(&params, "name");
            const char* phone = evhttp_find_header(&params, "phone");
        */

        evhttp_clear_headers(&params);

        LOG(Info, Utils::printMap(postParams));
        return postParams;
    }

    // handle post command
    void EventHttpServer::handlePostReq(struct evhttp_request *req, void *arg) {
        const char* requestUri = evhttp_request_get_uri(req);
        LOG(Info, requestUri);

        // all post params
        EventHttpServer::dumpPostParams(req);


        size_t len = evbuffer_get_length(req->input_buffer);
        char *post_data = (char *)malloc(len + 1);
        memcpy(post_data, evbuffer_pullup(req->input_buffer, -1), len);
        post_data[len] = '\0';

        struct evbuffer *returnbuffer = evbuffer_new();

        evbuffer_add_printf(returnbuffer, "Received a POST request with data: %s\n", post_data);
        evhttp_send_reply(req, HTTP_OK, "OK", returnbuffer);
        evbuffer_free(returnbuffer);
        free(post_data);
    }

    const void EventHttpServer::dumpHeaders(const struct evhttp_request *req) {
        struct evkeyvalq* headers = req->input_headers;
        map<string, string> headersMap;
        for (struct evkeyval* header = headers->tqh_first; header; header = header->next.tqe_next) {
            headersMap[header->key] = header->value;
        }
        evhttp_clear_headers(headers);

        LOG(Info, Utils::printMap(headersMap));
    }

    // http request handle entrance
    void EventHttpServer::httpReqEntrance(struct evhttp_request *req, void *arg) {
        struct evhttp_connection* conn = evhttp_request_get_connection(req);
        const char* requestUri = evhttp_request_get_uri(req);

        char * clientIP = NULL;
        ev_uint16_t clientPort = 0;
        evhttp_connection_get_peer(conn, &clientIP, &clientPort);

        EventHttpServer::dumpHeaders(req);

        evhttp_cmd_type cmdType = evhttp_request_get_command(req);
        LOG(Info, clientIP, clientPort, cmdType, requestUri);

        switch (cmdType) {
            case EVHTTP_REQ_GET:
                handleGetReq(req, arg);
                break;
            case EVHTTP_REQ_POST:
                handlePostReq(req, arg);
                break;
            default:
                evhttp_send_error(req, HTTP_BADREQUEST, 0);
                LOG(Warn, clientIP, clientPort, cmdType, requestUri);
        }
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

        evhttp_set_gencb(http, EventHttpServer::httpReqEntrance, NULL);

        struct evhttp_bound_socket *handle = evhttp_bind_socket_with_handle(http, SERVER_LISTEN_IP, HTTP_LISTEN_PORT);
        if (!handle) {
            LOG(Error, "couldn't bind to port ", HTTP_LISTEN_PORT, evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
            return NULL;
        }

        LOG(Info, "http server listening on ", SERVER_LISTEN_IP, ":", HTTP_LISTEN_PORT);
        return http;
    }








}


