/**
 * Copyright (c) 2023-07, typecodes.com (vfhky@typecodes.com)
 *
 * All rights reserved.
 *
 * Tools for tcp client.
 */


#include "eventTcpClient.h"
#include "hiRedisRespUtils.h"
#include "hiRedisException.h"





namespace socketRedisSentinel {


    EventTcpClient::EventTcpClient() {
        this->freeRcvData();
        this->m_base = NULL;
        this->m_localIp = "";
        this->m_localPort = 0;
    }

    EventTcpClient::~EventTcpClient() {
        this->freeRcvData();
        this->freeEventBase();
    }


    // local ip and port
    void EventTcpClient::setLocalIp(const char *localIp) {
        if (NULL != localIp) {
            this->m_localIp = localIp;
        }
    }

    std::string EventTcpClient::getLocalIp() const {
        return this->m_localIp;
    }

    void EventTcpClient::setLocalPort(const uint16_t &localPort) {
        this->m_localPort = localPort;
    }

    uint16_t EventTcpClient::getLocalPort() const {
        return this->m_localPort;
    }


    void EventTcpClient::setTcpNoDelay(evutil_socket_t fd) {
        int one = 1;
        int ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
        LOG(Debug, "setTcpNoDelay done", fd, ret);
    }

    void EventTcpClient::fillLocalIpPort(evutil_socket_t fd, void *ctx) {
        if (NULL == ctx) {
            LOG(Debug, "illegal", fd, ctx);
            return;
        }

        EventTcpClient* eventTcpClient = static_cast<EventTcpClient*>(ctx);
        struct sockaddr_storage ss;
        socklen_t len = sizeof(ss);
        if (getsockname(fd, (struct sockaddr*)&ss, &len) == 0) {
            if (ss.ss_family == AF_INET) {
                struct sockaddr_in* sin = (struct sockaddr_in*)&ss;
                char addr[INET_ADDRSTRLEN] = {0x00};
                const char* str = inet_ntop(AF_INET, &(sin->sin_addr), addr, INET_ADDRSTRLEN);
                if (str) {
                    eventTcpClient->setLocalIp(str);
                    eventTcpClient->setLocalPort(ntohs(sin->sin_port));
                }
            } else if (ss.ss_family == AF_INET6) {
                struct sockaddr_in6* sin6 = (struct sockaddr_in6*)&ss;
                char addr[INET6_ADDRSTRLEN] = {0x00};
                const char* str = inet_ntop(AF_INET6, &(sin6->sin6_addr), addr, INET6_ADDRSTRLEN);
                if (str) {
                    eventTcpClient->setLocalIp(addr);
                    eventTcpClient->setLocalPort(ntohs(sin6->sin6_port));
                }
            }
        }
    }

    void EventTcpClient::eventCb(struct bufferevent *bev, short event, void *ctx) {
        evutil_socket_t fd = 0;
        if (NULL != bev) {
            fd = bufferevent_getfd(bev);
        }
        int i_errCode = EVUTIL_SOCKET_ERROR();
        LOG(Debug, fd, bev, i_errCode, event, evutil_socket_error_to_string(i_errCode));

        bool bLoopExit = true;
        if(event & BEV_EVENT_TIMEOUT) {
            LOG(Debug, "eventCb timeout reached");
        } else if (event & BEV_EVENT_CONNECTED) {
            EventTcpClient::setTcpNoDelay(fd);
            bLoopExit = false;
            EventTcpClient::fillLocalIpPort(fd, ctx);
            LOG(Debug, "eventCb connection success");
        } else if (event & BEV_EVENT_EOF) {
            LOG(Debug, "eventCb connection closed");
        } else if (event & BEV_EVENT_ERROR) {
            LOG(Error, "eventCb some other error");
        } else if(event & BEV_EVENT_READING) {
            bLoopExit = false;
            LOG(Debug, "eventCb read data is ready");
        } else if(event & BEV_EVENT_WRITING) {
            bLoopExit = false;
            LOG(Debug, "eventCb write data is ready");
        } else {
            LOG(Debug, "eventCb unkown event callback", event);
        }

        // break loop
        if (bLoopExit && NULL != ctx) {
            EventTcpClient* eventTcpClient = static_cast<EventTcpClient*>(ctx);
            eventTcpClient->loopExitEventBase();
            LOG(Debug, "eventCb event_base_loopexit");
        }
    }


    void EventTcpClient::sendDataCb(evutil_socket_t fd, short events, void *arg) {
        bufferevent *bev = static_cast<bufferevent *>(arg);

        // send data to server
        std::string reqData = "test";
        if (0 != bufferevent_write(bev, reqData.c_str(), reqData.size())) {
            LOG(Error, "send data to server fail", reqData);
        }
    }


    bool EventTcpClient::sendData(struct bufferevent* bev, const std::string &data) {
        if (data.empty()) {
            LOG(Info, data);
            return true;
        }

        string reqData = data + "\n";
        if (0 == bufferevent_write(bev, reqData.c_str(), reqData.size())) {
            LOG(Debug, "===> sendData ok , wait callback", reqData);
            return true;
        }

        LOG(Error, "===> sendData fail", reqData, evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
        return false;
    }

    // check the data received from server entirely.
    bool EventTcpClient::checkReadDataCompleted(const char *receivedData, const size_t &totalReadSize) {
        const std::string receivedDataStr(receivedData, totalReadSize);

        HiRedisReplyData hiRedisRespValue;
        try {
            hiRedisRespValue = HiRedisRespUtils::processItem(receivedDataStr);
            LOG(Debug, totalReadSize, hiRedisRespValue.dump());
        } catch (const HiRedisException &e) {
            LOG(Debug, "received data not completed", totalReadSize, hiRedisRespValue.dump(), e.what());
            return false;
        } catch (const MyException &e) {
            LOG(Error, "illegal data", totalReadSize, hiRedisRespValue.dump(), e.what(), receivedData);
        } catch (...) {
            LOG(Error, "unknown error", totalReadSize, hiRedisRespValue.dump(), receivedData);
        }

        return true;
    }

    // callback function when recieved data from server.
    void EventTcpClient::readCb(struct bufferevent* bev, void* ctx) {
        EventTcpClient* eventTcpClient = static_cast<EventTcpClient*>(ctx);
        if (NULL == eventTcpClient) {
            LOG(Error, "readCb ctx is NULL", evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
            return;
        }

        struct evbuffer* input = bufferevent_get_input(bev);
        size_t receivedDataSize = evbuffer_get_length(input);
        if (receivedDataSize > 0) {
            char *receivedData = new char[receivedDataSize + 1];
            evbuffer_copyout(input, receivedData, receivedDataSize);
            receivedData[receivedDataSize] = '\0';
            // set the recieved data to memeber.
            eventTcpClient->setRcvData(receivedData, receivedDataSize);
            // reset data readed.
            evbuffer_drain(input, receivedDataSize);
        }

        // break loop
        bool bReadDataCompleted = EventTcpClient::checkReadDataCompleted(eventTcpClient->getRcvData().c_str(), \
                eventTcpClient->getRcvDataSize());
        LOG(Debug, "===> read success", ctx, bReadDataCompleted, receivedDataSize, \
                eventTcpClient->getRcvDataSize(), eventTcpClient->getRcvData());
        if (bReadDataCompleted) {
            eventTcpClient->loopExitEventBase();
        }
    }

    // callback function when data was send to server.
    void EventTcpClient::writeCb(struct bufferevent *bev, void *ctx) {
        LOG(Info, "writeCb done");
    }

    void EventTcpClient::reqTimeoutCb(evutil_socket_t fd, short event, void* arg) {
        bool bArg = NULL != arg;
        if (bArg) {
            EventTcpClient * eventTcpClient = static_cast<EventTcpClient*>(arg);
            eventTcpClient->loopExitEventBase();
        }

        LOG(Info, "timeout reached", fd, event, bArg);
    }

    bool EventTcpClient::createEventBase() {
        this->m_base = event_base_new();
        if (NULL == this->m_base) {
            LOG(Error, "event_base_new fail", \
                evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
            return false;
        }

        return true;
    }

    event_base* EventTcpClient::getEventBase() const {
        return this->m_base;
    }

    void EventTcpClient::freeEventBase() {
        if (NULL != this->m_base) {
            event_base_free(this->m_base);
            this->m_base = NULL;
        }
    }

    void EventTcpClient::loopExitEventBase() {
        if (NULL != this->m_base) {
            event_base_loopexit(this->m_base, NULL);
        }
    }


    bool EventTcpClient::doRequest(const std::string &ip, u_short port, const std::string &reqData,
            int16_t timeoutMics, std::string &rcvData) {
        int64_t bTime = Utils::getMilliSecond();

        if (!this->createEventBase()) {
            return false;
        }

        event_base* base = this->getEventBase();
        bufferevent* bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

        // callback
        bufferevent_data_cb rCb = EventTcpClient::readCb;
        bufferevent_data_cb wCb = EventTcpClient::writeCb;;
        bufferevent_event_cb eventCb = EventTcpClient::eventCb;
        bufferevent_setcb(bev, rCb, wCb, eventCb, this);
        bufferevent_enable(bev, EV_READ | EV_WRITE | EV_PERSIST);

        // total request timeout callback
        struct event* reqTimeoutEvent = evtimer_new(base, reqTimeoutCb, this);
        struct timeval reqTimeout;
        reqTimeout.tv_sec = timeoutMics / 1000;
        reqTimeout.tv_usec = timeoutMics % 1000;
        evtimer_add(reqTimeoutEvent, &reqTimeout);

        // connect server
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        // Convert IPv4 and IPv6 addresses from text to binary
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
        if (NULL != bev) {
            bufferevent_free(bev);
            bev = NULL;
        }
        if (NULL != reqTimeoutEvent) {
            event_free(reqTimeoutEvent);
            reqTimeoutEvent = NULL;
        }

        // return recieved data
        rcvData = this->getRcvData();
        int64_t eTime = Utils::getMilliSecond();
        int64_t timeMics = eTime - bTime;
        LOG(Info, "doRequest done", ip, port, this->getLocalIp(), this->getLocalPort(), timeoutMics, timeMics, this->getRcvDataSize(), rcvData, reqData);
        return true;
    }

    void EventTcpClient::setRcvData(char * const rcvData, const size_t &rcvDataSize) {
        if (NULL != rcvData && rcvDataSize > 0) {
            this->m_rcvData.append(rcvData, rcvDataSize);
            this->m_rcvDataSize += rcvDataSize;
            delete [] rcvData;
        }
    }

    std::string EventTcpClient::getRcvData() const {
        return this->m_rcvData;
    }

    size_t EventTcpClient::getRcvDataSize() const {
        return this->m_rcvDataSize;
    }

    void EventTcpClient::freeRcvData() {
        this->m_rcvData.clear();
        this->m_rcvDataSize = 0;
    }















}


