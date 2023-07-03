#include "libEventServer.h"
#include "logicEntrance.h"





namespace socketRedisSentinel {


    void LibEventServer::trimCR(char *p) {
        if (NULL == p) {
            return;
        }

        size_t len = strlen(p);
        if (len <= 0) {
            return;
        }

        if (0x0A == *(p + len - 1)) {
            *(p + len - 1) = 0X00;
        }
        if (0x0D == *(p + len - 2)) {
            *(p + len - 2) = 0X00;
        }
    }


    void LibEventServer::readCb(struct bufferevent *bev, void *ctx) {
        char buf[4096] = {0x00};
        size_t readSize = bufferevent_read(bev, buf, sizeof(buf) - 1);
        if (readSize < 0) {
            LOG(Error, "bufferevent_read failed", buf, evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
            exit(1);
        }

        buf[readSize] = '\0';
        LibEventServer::trimCR(buf);
        LOG(Info, readSize, buf);

        string rsp = LogicEntrance::instance().handleReq(buf);
        bufferevent_write(bev, rsp.c_str(), rsp.size());
        LOG(Info, rsp);
    }

    void LibEventServer::writeCb(struct bufferevent *bev, void *ctx) {
        char buf[4096] = {0x00};
        size_t readSize = bufferevent_read(bev, buf, sizeof(buf));
        if (readSize < 0) {
            LOG(Error, "bufferevent_read failed", buf, evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
            exit(1);
        }

        LOG(Info, readSize, buf);
    }

    void LibEventServer::eventCb(struct bufferevent *bev, short event, void *ctx) {
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



    void LibEventServer::listenerCb(struct evconnlistener *listener, evutil_socket_t fd,
                struct sockaddr *address, int socklen, void *ctx)
    {
        LOG(Debug, "accept client", fd);

        struct event_base *base = evconnlistener_get_base(listener);
        struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
        if (NULL == bev) {
            LOG(Error, "bufferevent_socket_new failed", fd, evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
            return;
        }


        bufferevent_data_cb rCb = LibEventServer::readCb;
        bufferevent_data_cb wCb = NULL;
        bufferevent_event_cb eventCb = LibEventServer::eventCb;

        bufferevent_setcb(bev, rCb, wCb, eventCb, &fd);
        bufferevent_enable(bev, EV_READ | EV_PERSIST);
    }

    int LibEventServer::init() {
        struct event_base *base = event_base_new();
        if (NULL == base) {
            LOG(Error, "event_base_new failed", evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
            return -1;
        }

        struct sockaddr_in sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_port = htons(10086);
        sin.sin_addr.s_addr = htonl(INADDR_ANY);


        struct evconnlistener *listener = evconnlistener_new_bind(base, listenerCb, NULL,
                                                                LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
                                                                (struct sockaddr *)&sin, sizeof(sin));
        if (NULL == listener) {
            LOG(Error, "evconnlistener_new_bind failed", evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
            return -2;
        }

        event_base_dispatch(base);


        evconnlistener_free(listener);
        event_base_free(base);
        return 0;
    }










}


