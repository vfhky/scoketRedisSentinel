#include "httpComHandle.h"
#include "logicEntrance.h"




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



    HttpComHandle &HttpComHandle::instance() {
        static HttpComHandle instance;
        return instance;
    }




    /**
     * convert http req data to inner data struct
     */
    ClientReqInfo HttpComHandle::httpBodyToClientReqInfo(const std::map<std::string, std::string> &httpBody) {
        ClientReqInfo clientReqInfo;

        __foreach(it, httpBody) {
            std::string key = it->first;
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            const std::string &value = it->second;
            if (key == "type") {
                clientReqInfo.type = static_cast<CLIENT_REQ_TYPE>(Utils::stringToU32(value));
            } else if (key == "ip") {
                clientReqInfo.ip = value;
            } else if (key == "port") {
                clientReqInfo.port = (uint16_t)Utils::stringToU32(value);
            } else if (key == "redistype") {
                clientReqInfo.redisType = static_cast<CLIENT_REQ_REDIS_TYPE>(Utils::stringToU32(value));
            } else if (key == "poolname") {
                clientReqInfo.poolName = value;
            } else if (key == "hashkey") {
                clientReqInfo.hashKey = value;
            } else if (key == "loglv") {
                clientReqInfo.logLv = Utils::stringToI64(value);
            } else if (key == "logtype") {
                clientReqInfo.logType = Utils::stringToI64(value);
            } else {
                //
            }
        }
        return clientReqInfo;
    }


    const bool HttpComHandle::fillGetParams(HttpReqInfo &httpReqInfo) {
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

        if (NULL != uri) {
            evhttp_uri_free(uri);
        }
        evhttp_clear_headers(&params);
        return true;
    }


    // handle get command
    void HttpComHandle::handleGetReq(struct evhttp_request *req, HttpReqInfo &httpReqInfo, void *arg) {
        if (!HttpComHandle::fillGetParams(httpReqInfo)) {
            evhttp_send_reply(req, HTTP_BADREQUEST, "Bad Request", NULL);
            LOG(Info, "Bad Request", httpReqInfo.dump());
            return;
        }

        LOG(Info, "fill param end", httpReqInfo.dump());
    }

    const std::map<std::string, std::string> HttpComHandle::parseFormData(const std::string& postData) {
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

    const bool HttpComHandle::fillPostParams(struct evhttp_request *req, HttpReqInfo &httpReqInfo) {
        struct evbuffer* input = evhttp_request_get_input_buffer(req);

        char* receivedPostData = NULL;
        size_t readSize = 0;
        size_t totalReadSize = 0;

        while (NULL != input && (readSize = evbuffer_get_length(input)) > 0) {
            char* buffer = new char[readSize+1];
            memset(buffer, 0x00, readSize+1);
            evbuffer_copyout(input, buffer, readSize);

            char* resizedData = new char[totalReadSize + readSize + 1];
            if (receivedPostData != NULL) {
                memcpy(resizedData, receivedPostData, totalReadSize);
                delete[] receivedPostData;
            }
            memcpy(resizedData + totalReadSize, buffer, readSize);
            resizedData[totalReadSize + readSize] = '\0';
            receivedPostData = resizedData;
            totalReadSize += readSize;

            delete[] buffer;
            evbuffer_drain(input, readSize);
        }
        LOG(Debug, receivedPostData);

        std::map<std::string, std::string> params;
        std::string contentType = "";
        if (httpReqInfo.headers.find("Content-Type") != httpReqInfo.headers.end()) {
            contentType = httpReqInfo.headers["Content-Type"];
        }
        if (contentType.find("application/x-www-form-urlencoded") != std::string::npos) {
            httpReqInfo.body = HttpComHandle::parseFormData(receivedPostData);
        } else if (contentType.find("application/json") != std::string::npos) {
            httpReqInfo.body = Utils::simpleJsonToMap(receivedPostData);
        } else {
            LOG(Warn, "Unsupported Content-Type", contentType, httpReqInfo.dump());
            return false;
        }

        return true;
    }

    // handle post command
    void HttpComHandle::handlePostReq(struct evhttp_request *req, HttpReqInfo &httpReqInfo, void *arg) {
        // all post params
        if (!HttpComHandle::fillPostParams(req, httpReqInfo)) {
            evhttp_send_reply(req, HTTP_BADREQUEST, "Bad Request", NULL);
            LOG(Info, "Bad Request", httpReqInfo.dump());
            return;
        }

        LOG(Info, "fill param end", httpReqInfo.dump());
    }

    // make http response to client
    void HttpComHandle::doHttpRsp(struct evhttp_request *req, const std::string &rspData,
                const int &rspCode, const std::string &rspReason) {
        struct evbuffer *returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "%s\n", rspData.c_str());
        evhttp_send_reply(req, rspCode, rspReason.c_str(), returnbuffer);
        evbuffer_free(returnbuffer);
        LOG(Info, "response end", rspCode, rspReason, rspData);
    }

    const void HttpComHandle::fillHeaders(struct evhttp_request *req, HttpReqInfo &httpReqInfo) {
        struct evkeyvalq* headers = evhttp_request_get_input_headers(req);
        struct evkeyval* kv;
        TAILQ_FOREACH(kv, headers, next) {
            httpReqInfo.headers[kv->key] = kv->value;
        }

        evhttp_clear_headers(headers);
    }

    const void HttpComHandle::fillClientIpPort(struct evhttp_request *req, HttpReqInfo &httpReqInfo) {
        struct evhttp_connection* conn = evhttp_request_get_connection(req);
        char * peerIp = NULL;
        ev_uint16_t peerPort = 0;
        evhttp_connection_get_peer(conn, &peerIp, &peerPort);
        httpReqInfo.peerIp = peerIp;
        httpReqInfo.peerPort = peerPort;
    }

    const void HttpComHandle::fillCmdType(struct evhttp_request *req, HttpReqInfo &httpReqInfo) {
        httpReqInfo.cmdType = evhttp_request_get_command(req);
    }

    const void HttpComHandle::fillRequestUri(struct evhttp_request *req, HttpReqInfo &httpReqInfo) {
        httpReqInfo.requestUri = evhttp_request_get_uri(req);

        struct evhttp_uri* httpUri = evhttp_uri_parse(httpReqInfo.requestUri.c_str());
        if (NULL != httpUri) {
            const char *uriPath = evhttp_uri_get_path(httpUri);
            httpReqInfo.uriPath = (NULL != uriPath) ? uriPath : "";

            const char *uriHost = evhttp_uri_get_host(httpUri);
            httpReqInfo.uriHost = (NULL != uriHost) ? uriHost : "";

            httpReqInfo.uriPort = evhttp_uri_get_port(httpUri);

            const char *userInfo = evhttp_uri_get_userinfo(httpUri);
            httpReqInfo.userInfo = (NULL != userInfo) ? userInfo : "";
        }
    }

    const void HttpComHandle::fillHttpReqInfo(struct evhttp_request *req, HttpReqInfo &httpReqInfo) {
        httpReqInfo.clear();
        HttpComHandle::fillClientIpPort(req, httpReqInfo);
        HttpComHandle::fillCmdType(req, httpReqInfo);
        HttpComHandle::fillHeaders(req, httpReqInfo);
        HttpComHandle::fillRequestUri(req, httpReqInfo);
    }

    void HttpComHandle::handleSentinelUri(struct evhttp_request *req, const map<string, string> &httpBody) {
        ClientReqInfo clientReqInfo = HttpComHandle::httpBodyToClientReqInfo(httpBody);
        string rsp = LogicEntrance::instance().handleReq(clientReqInfo);
        HttpComHandle::doHttpRsp(req, rsp, HTTP_OK, "OK");
    }

    // http request handle entrance
    void HttpComHandle::httpReqEntrance(struct evhttp_request *req, void *arg) {
        HttpReqInfo httpReqInfo;
        HttpComHandle::fillHttpReqInfo(req, httpReqInfo);

        switch (httpReqInfo.cmdType) {
            case EVHTTP_REQ_GET:
                HttpComHandle::handleGetReq(req, httpReqInfo, arg);
                break;
            case EVHTTP_REQ_POST:
                HttpComHandle::handlePostReq(req, httpReqInfo, arg);
                break;
            default:
                evhttp_send_error(req, HTTP_BADREQUEST, 0);
                LOG(Warn, "not supported req type.", httpReqInfo.dump());
                return;
        }

        // router
        if (HTTP_URI_SENTINEL == httpReqInfo.uriPath) {
            LOG(Info, "find router.", HTTP_URI_SENTINEL);
            HttpComHandle::handleSentinelUri(req, httpReqInfo.body);
        } else {
            LOG(Info, "not find router , redirect to sentinel help");
            HttpComHandle::handleSentinelUri(req, httpReqInfo.body);
        }
    }

    void HttpComHandle::httpRouter(struct evhttp *http) {
        if (NULL != http) {
            // default router
            evhttp_set_gencb(http, HttpComHandle::httpReqEntrance, NULL);
            //evhttp_set_cb(http, "/help", HttpComHandle::helpEntrance, NULL);
            //evhttp_set_cb(http, "/sentinel", HttpComHandle::httpReqEntrance, NULL);

            LOG(Info, "set http router success");
        }
    }








}


