#include "eventTcpServer.h"
#include "logicEntrance.h"





namespace socketRedisSentinel {

    EventTcpServer &EventTcpServer::instance() {
        static EventTcpServer instance;
        return instance;
    }


    void EventTcpServer::trimCR(char *p) {
        if (NULL == p) {
            return;
        }

        size_t len = strlen(p);
        if (len <= 0) {
            return;
        }

        if (0x0A == *(p + len - 1)) {
            *(p + len - 1) = 0x00;
        }
        if (0x0D == *(p + len - 2)) {
            *(p + len - 2) = 0x00;
        }
    }

    void EventTcpServer::trimCR(std::string &p) {
        if (p.empty()) {
            return;
        }

        size_t len = p.size();

        if (0x0A == p[len - 1]) {
            p[len - 1] = 0x00;
        }
        if (0x0D == p[len - 2]) {
            p[len - 2] = 0x00;
        }
    }



    ClientReqInfo EventTcpServer::pharseReq(const std::string &req) {
        ClientReqInfo reqInfo;
        if (req.empty()) {
            LOG(Info, reqInfo.dump());
            return reqInfo;
        }

        /**
         * 0-begin parse
         * 1-get value
         * 2-get flag
        */
        int32_t flag = 0;

        int32_t length = req.size();
        std::stringstream ss;
        std::string key;
        for (int32_t index = 0; index < length; index++) {
            char c = (char)req[index];

            // begin to parse a flag
            if ('-' == c) {
                if (0 == index || ' ' == (char)req[index-1]) {
                    flag = 2;
                    continue;
                }
            }

            // settle when reach blank or come to the end char
            if (' ' == c || index == length - 1) {
                // reached end
                if (index == length - 1) {
                    ss << c;
                }

                // settle
                if (1 == flag || index == length - 1) {
                    std::string value = ss.str();
                    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                    if (key == "type") {
                        reqInfo.type = static_cast<CLIENT_REQ_TYPE>(Utils::stringToU32(value));
                    } else if (key == "ip") {
                        reqInfo.ip = value;
                    } else if (key == "port") {
                        reqInfo.port = (uint16_t)Utils::stringToU32(value);
                    } else if (key == "redistype") {
                        reqInfo.redisType = static_cast<CLIENT_REQ_REDIS_TYPE>(Utils::stringToU32(value));
                    } else if (key == "poolname") {
                        reqInfo.poolName = value;
                    } else if (key == "hashkey") {
                        reqInfo.hashKey = value;
                    } else if (key == "loglv") {
                        reqInfo.logLv = Utils::stringToI64(value);
                    } else if (key == "logtype") {
                        reqInfo.logType = Utils::stringToI64(value);
                    } else {
                        //
                    }

                    key.clear();
                    value.clear();
                    ss.clear();
                    ss.str("");
                    flag = 0;
                } else if (2 == flag) {
                    key = ss.str();
                    ss.clear();
                    ss.str("");
                    flag = 1;
                } else {
                }
                continue;
            }

            if (0 != flag) {
                ss << c;
            }
        }

        LOG(Info, reqInfo.dump());
        return reqInfo;
    }


    void EventTcpServer::readCb(struct bufferevent *bev, void *ctx) {
        struct evbuffer* input = bufferevent_get_input(bev);
        std::string data = "";

        char *buffer = new char[SOCKET_DATA_BATCH_SIZE];
        size_t readSize = 0;
        while (NULL != input) {
            memset(buffer, 0x00, sizeof(buffer));
            size_t len = bufferevent_read(bev, buffer, sizeof(buffer) - 1);
            if (len <= 0) {
                break;
            }
            // buffer[len] = '\0';
            data += buffer;
            readSize += len;
        }
        delete []buffer;
        EventTcpServer::trimCR(data);
        LOG(Info, "readCb done", readSize, data);

        // parse req data and handle it and make a respone to client.
        ClientReqInfo clientReqInfo = EventTcpServer::pharseReq(data);
        std::string rsp = LogicEntrance::instance().handleReq(clientReqInfo);
        bufferevent_write(bev, rsp.c_str(), rsp.size());
        LOG(Info, clientReqInfo.dump(), rsp);
    }

    void EventTcpServer::writeCb(struct bufferevent *bev, void *ctx) {
        char buf[4096] = {0x00};
        size_t readSize = bufferevent_read(bev, buf, sizeof(buf));
        if (readSize < 0) {
            LOG(Error, "bufferevent_read failed", buf, evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
            exit(1);
        }

        LOG(Info, readSize, buf);
    }

    void EventTcpServer::eventCb(struct bufferevent *bev, short event, void *ctx) {
        evutil_socket_t fd = bufferevent_getfd(bev);
        int i_errCode = EVUTIL_SOCKET_ERROR();
        LOG(Debug, fd, i_errCode, event, evutil_socket_error_to_string(i_errCode) );

        if(event & BEV_EVENT_TIMEOUT) {
            LOG(Debug, "timeout reached");
        } else if (event & BEV_EVENT_CONNECTED) {
            LOG(Debug, "connection success");
        } else if (event & BEV_EVENT_EOF) {
            LOG(Debug, "connection closed");
        } else if (event & BEV_EVENT_ERROR) {
            LOG(Error, "some other error");
        } else if(event & BEV_EVENT_READING) {
            LOG(Debug, "read data is ready");
        } else if(event & BEV_EVENT_WRITING ) {
            LOG(Debug, "write data is ready");
        } else {
            LOG(Debug, "unkown event callback", event);
        }

        bufferevent_free(bev);
    }



    void EventTcpServer::listenerCb(struct evconnlistener *listener, evutil_socket_t fd,
                struct sockaddr *address, int socklen, void *ctx)
    {
        char client_ip[INET6_ADDRSTRLEN];
        struct sockaddr_in *addr = (struct sockaddr_in*)address;
        const char *ip = evutil_inet_ntop(AF_INET, &(addr->sin_addr), client_ip, INET6_ADDRSTRLEN);
        int port = ntohs(addr->sin_port);
        LOG(Debug, "accept client", fd, ip, port);

        struct event_base *base = evconnlistener_get_base(listener);
        struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
        if (NULL == bev) {
            LOG(Error, "bufferevent_socket_new failed", fd, evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
            return;
        }

        // callback
        bufferevent_data_cb rCb = EventTcpServer::readCb;
        bufferevent_data_cb wCb = NULL;
        bufferevent_event_cb eventCb = EventTcpServer::eventCb;

        bufferevent_setcb(bev, rCb, wCb, eventCb, &fd);
        bufferevent_enable(bev, EV_READ | EV_PERSIST);
    }

    struct evconnlistener * EventTcpServer::createTcpServer(struct event_base *base) {
        struct sockaddr_in sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_port = htons(TCP_LISTEN_PORT);
        sin.sin_addr.s_addr = htonl(INADDR_ANY);

        struct evconnlistener *listener = evconnlistener_new_bind(base, listenerCb, NULL,
                                                                LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
                                                                (struct sockaddr *)&sin, sizeof(sin));
        if (NULL == listener) {
            LOG(Error, "evconnlistener_new_bind tcp server failed", evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
            return NULL;
        }

        LOG(Info, "init tcp server success", TCP_LISTEN_PORT);
        return listener;
    }












}


