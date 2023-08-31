#include "eventTcpClient.h"





namespace socketRedisSentinel {

    void EventTcpClient::setTcpNoDelay(evutil_socket_t fd) {
        int one = 1;
        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
    }

    void EventTcpClient::eventCb(struct bufferevent *bev, short event, void *ctx) {
        evutil_socket_t fd = bufferevent_getfd(bev);
        int i_errCode = EVUTIL_SOCKET_ERROR();
        LOG(Debug, fd, i_errCode, evutil_socket_error_to_string(i_errCode) );

        if(event & BEV_EVENT_TIMEOUT) {
            EventTcpClient::setTcpNoDelay(fd);
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

        // break loop
        if (NULL != ctx) {
            event_base* base = static_cast<event_base*>(ctx);
            event_base_loopexit(base, NULL);
        }
    }


    void EventTcpClient::sendDataCb(evutil_socket_t fd, short events, void *arg) {
        bufferevent *bev = static_cast<bufferevent *>(arg);

        // send data to server
        string reqData = "test";
        if (0 != bufferevent_write(bev, reqData.c_str(), reqData.size())) {
            LOG(Error, "send data to server fail", reqData);
        }
    }


    bool EventTcpClient::sendData(struct bufferevent* bev, const string &reqData) {
        if (reqData.empty()) {
            LOG(Info, reqData);
            return true;
        }

        if (0 == bufferevent_write(bev, reqData.c_str(), reqData.size())) {
            LOG(Debug, "sendData ok", reqData);
            return true;
        }

        LOG(Error, "bufferevent_write fail", reqData);
        return false;
    }

    void EventTcpClient::readCb(struct bufferevent* bev, void* ctx) {
        struct evbuffer* input = bufferevent_get_input(bev);
        std::string data = "";

        char buffer[20] = {0x00};
        while (true) {
            size_t len = evbuffer_remove(input, buffer, sizeof(buffer) - 1);
            if (len <= 0) {
                break;
            }
            buffer[len] = '\0';
            data += buffer;
        }

        m_rcvData = data;
        LOG(Debug, "Received ok", m_rcvData);
    }

    void EventTcpClient::writeCb(struct bufferevent *bev, void *ctx) {
        char buf[4096] = {0x00};
        size_t readSize = bufferevent_read(bev, buf, sizeof(buf));
        if (readSize < 0) {
            LOG(Error, "bufferevent_read failed", buf, evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
            exit(1);
        }

        LOG(Info, readSize, buf);
    }

    void EventTcpClient::reqTimeoutCb(evutil_socket_t fd, short event, void* arg) {
        bool bArg = NULL != arg;
        if (bArg) {
            struct event_base* base = static_cast<struct event_base*>(arg);
            event_base_loopbreak(base);
        }

        m_rcvData.clear();
        LOG(Info, "timeout reached", fd, event, bArg, m_rcvData);
    }


    bool EventTcpClient::doRequest(const std::string &ip, u_short port, const string &reqData, int16_t timeoutMics, string &rcvData) {
        event_base* base = event_base_new();
        if (NULL == base) {
            LOG(Error, "event_base_new fail", ip, port, timeoutMics, evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()), reqData);
            return false;
        }

        bufferevent* bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

        // callback
        bufferevent_data_cb rCb = EventTcpClient::readCb;
        bufferevent_data_cb wCb = EventTcpClient::writeCb;
        bufferevent_event_cb eventCb = EventTcpClient::eventCb;
        bufferevent_setcb(bev, rCb, wCb, eventCb, NULL);
        bufferevent_enable(bev, EV_READ | EV_PERSIST | EV_WRITE);

        // total request timeout callback
        struct event* reqTimeoutEvent = evtimer_new(base, reqTimeoutCb, base);
        struct timeval reqTimeout;
        reqTimeout.tv_sec = timeoutMics / 1000;
        reqTimeout.tv_usec = timeoutMics % 1000;
        evtimer_add(reqTimeoutEvent, &reqTimeout);

        // connect server
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &(serverAddr.sin_addr));
        if (bufferevent_socket_connect(bev, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            LOG(Error, "bufferevent_socket_connect fail", ip, port, evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()), reqData);
            bufferevent_free(bev);
            event_free(reqTimeoutEvent);
            return false;
        }

        // make an event
        #if 0
        evutil_socket_t fd = bufferevent_getfd(bev);
        struct event* ev = event_new(base, fd, EV_READ | EV_PERSIST, sendDataCb, bev);
        event_add(ev, NULL);
        #endif
        // send data directly
        if (!EventTcpClient::sendData(bev, reqData)) {
            LOG(Error, "sendData fail", ip, port, evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()), reqData);
            bufferevent_free(bev);
            event_free(reqTimeoutEvent);
            return false;
        }

        // run forever
        event_base_dispatch(base);

        // release source
        bufferevent_free(bev);
        event_free(reqTimeoutEvent);
        // event_base_free(base);

        // return recieved data
        rcvData = m_rcvData;

        LOG(Debug, "done", ip, port, timeoutMics, rcvData, reqData);
        return true;
    }















}


