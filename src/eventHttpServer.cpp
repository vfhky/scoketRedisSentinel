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




    const bool EventHttpServer::fillGetParams(HttpReqInfo &httpReqInfo) {
        // /?id=001&name=typecodes&phone=1111
        struct evhttp_uri* uri = evhttp_uri_parse(httpReqInfo.requestUri.c_str());
        if (!uri) {
            LOG(Warn, "Failed to parse URI.");
            return false;
        }

        // do not use evhttp_parse_query_str(requestUri, &params); for it will return key of "/?id"
        struct evkeyvalq params;
        evhttp_parse_query_str(evhttp_uri_get_query(uri), &params);
        struct evkeyval* kv;
        TAILQ_FOREACH(kv, &params, next) {
            httpReqInfo.body[kv->key] = kv->value;
        }

        /**
         * get special param :
         *   const char* id = evhttp_find_header(&params, "id");
        */

        evhttp_clear_headers(&params);
        return true;
    }


    // handle get command
    void EventHttpServer::handleGetReq(struct evhttp_request *req, HttpReqInfo &httpReqInfo, void *arg) {
        if (!EventHttpServer::fillGetParams(httpReqInfo)) {
            evhttp_send_reply(req, HTTP_BADREQUEST, "Bad Request", NULL);
            LOG(Info, "Bad Request", httpReqInfo.dump());
            return;
        }

        LOG(Info, "end", httpReqInfo.dump());

        EventHttpServer::doHttpRsp(req, httpReqInfo.requestUri, HTTP_OK, "OK");
    }

    const std::map<std::string, std::string> EventHttpServer::parseFormData(const std::string& postData) {
        std::map<std::string, std::string> params;
        size_t pos = 0;
        while (pos < postData.length()) {
            size_t delimiterPos = postData.find('&', pos);
            if (delimiterPos == std::string::npos) {
                delimiterPos = postData.length();
            }

            size_t equalPos = postData.find('=', pos);
            if (equalPos != std::string::npos && equalPos < delimiterPos) {
                std::string key = postData.substr(pos, equalPos - pos);
                std::string value = postData.substr(equalPos + 1, delimiterPos - equalPos - 1);
                params[key] = value;
            }

            pos = delimiterPos + 1;
        }

        return params;
    }

    const bool EventHttpServer::fillPostParams(struct evhttp_request *req, HttpReqInfo &httpReqInfo) {
        struct evbuffer* inbuf = evhttp_request_get_input_buffer(req);
        std::stringstream dataStream;
        while (evbuffer_get_length(inbuf)) {
            int n = 0;
            char cbuf[128] = {0x00};
            n = evbuffer_remove(inbuf, cbuf, sizeof(cbuf));
            if (n > 0) {
                dataStream.write(cbuf, n);
            }
        }
        LOG(Debug, dataStream.str());

        std::map<std::string, std::string> params;
        std::string contentType = "";
        if (httpReqInfo.headers.find("Content-Type") != httpReqInfo.headers.end()) {
            contentType = httpReqInfo.headers["Content-Type"];
        }
        if (contentType.find("application/x-www-form-urlencoded") != std::string::npos) {
            httpReqInfo.body = EventHttpServer::parseFormData(dataStream.str());
        } else if (contentType.find("application/json") != std::string::npos) {
            //httpReqInfo.body = EventHttpServer::parseJsonData(dataStream.str());
        } else {
            LOG(Warn, "Unsupported Content-Type", contentType, httpReqInfo.dump());
            return false;
        }

        return true;
    }

    // handle post command
    void EventHttpServer::handlePostReq(struct evhttp_request *req, HttpReqInfo &httpReqInfo, void *arg) {
        // all post params
        if (!EventHttpServer::fillPostParams(req, httpReqInfo)) {
            evhttp_send_reply(req, HTTP_BADREQUEST, "Bad Request", NULL);
            LOG(Info, "Bad Request", httpReqInfo.dump());
            return;
        }

        LOG(Info, "end", httpReqInfo.dump());

        EventHttpServer::doHttpRsp(req, Utils::printMap(httpReqInfo.body), HTTP_OK, "OK");
    }


    void EventHttpServer::doHttpRsp(struct evhttp_request *req, const std::string &rspData,
                const int &rspCode, const std::string &rspReason) {
        struct evbuffer *returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "%s\n", rspData.c_str());
        evhttp_send_reply(req, rspCode, rspReason.c_str(), returnbuffer);
        evbuffer_free(returnbuffer);
        LOG(Info, "response end", rspData, rspCode, rspReason);
    }

    const void EventHttpServer::fillHeaders(struct evhttp_request *req, HttpReqInfo &httpReqInfo) {
        struct evkeyvalq* headers = evhttp_request_get_input_headers(req);
        struct evkeyval* kv;
        TAILQ_FOREACH(kv, headers, next) {
            httpReqInfo.headers[kv->key] = kv->value;
        }

        // map<string, string> headersMap;
        // for (struct evkeyval* header = headers->tqh_first; header; header = header->next.tqe_next) {
        //     headersMap[header->key] = header->value;
        // }
        evhttp_clear_headers(headers);
    }

    const void EventHttpServer::fillClientIpPort(struct evhttp_request *req, HttpReqInfo &httpReqInfo) {
        struct evhttp_connection* conn = evhttp_request_get_connection(req);
        char * clientIP = NULL;
        ev_uint16_t clientPort = 0;
        evhttp_connection_get_peer(conn, &clientIP, &clientPort);
        httpReqInfo.ip = clientIP;
        httpReqInfo.port = clientPort;
    }

    const void EventHttpServer::fillCmdType(struct evhttp_request *req, HttpReqInfo &httpReqInfo) {
        httpReqInfo.cmdType = evhttp_request_get_command(req);
    }

    const void EventHttpServer::fillRequestUri(struct evhttp_request *req, HttpReqInfo &httpReqInfo) {
        httpReqInfo.requestUri = evhttp_request_get_uri(req);
    }

    const void EventHttpServer::fillHttpReqInfo(struct evhttp_request *req, HttpReqInfo &httpReqInfo) {
        EventHttpServer::fillClientIpPort(req, httpReqInfo);
        EventHttpServer::fillCmdType(req, httpReqInfo);
        EventHttpServer::fillHeaders(req, httpReqInfo);
        EventHttpServer::fillRequestUri(req, httpReqInfo);
    }

    // http request handle entrance
    void EventHttpServer::httpReqEntrance(struct evhttp_request *req, void *arg) {
        HttpReqInfo httpReqInfo;
        EventHttpServer::fillHttpReqInfo(req, httpReqInfo);


        LOG(Info, httpReqInfo.dump());

        switch (httpReqInfo.cmdType) {
            case EVHTTP_REQ_GET:
                EventHttpServer::handleGetReq(req, httpReqInfo, arg);
                break;
            case EVHTTP_REQ_POST:
                EventHttpServer::handlePostReq(req, httpReqInfo, arg);
                break;
            default:
                evhttp_send_error(req, HTTP_BADREQUEST, 0);
                LOG(Warn, httpReqInfo.dump());
        }
    }

    void EventHttpServer::httpRouter(struct evhttp *http) {
        if (NULL != http) {
            // default router
            evhttp_set_gencb(http, EventHttpServer::httpReqEntrance, NULL);
            //evhttp_set_cb(http, "/help", EventHttpServer::helpEntrance, NULL);
            //evhttp_set_cb(http, "/sentinel", EventHttpServer::httpReqEntrance, NULL);

            LOG(Info, "set http router success");
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

        EventHttpServer::httpRouter(http);

        struct evhttp_bound_socket *handle = evhttp_bind_socket_with_handle(http, SERVER_LISTEN_IP, HTTP_LISTEN_PORT);
        if (!handle) {
            LOG(Error, "couldn't bind to port ", HTTP_LISTEN_PORT, evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
            return NULL;
        }

        LOG(Info, "http server listening on ", SERVER_LISTEN_IP, ":", HTTP_LISTEN_PORT);
        return http;
    }








}


