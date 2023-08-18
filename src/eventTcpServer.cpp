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


    void EventTcpServer::readCb(struct bufferevent *bev, void *ctx) {
        char buf[4096] = {0x00};
        size_t readSize = bufferevent_read(bev, buf, sizeof(buf) - 1);
        if (readSize < 0) {
            LOG(Error, "bufferevent_read failed", buf, evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
            exit(1);
        }

        buf[readSize] = '\0';
        EventTcpServer::trimCR(buf);
        LOG(Info, readSize, buf);

        string rsp = LogicEntrance::instance().handleReq(buf);
        bufferevent_write(bev, rsp.c_str(), rsp.size());
        LOG(Info, rsp);
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
        LOG(Debug, fd, i_errCode, evutil_socket_error_to_string(i_errCode) );

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


