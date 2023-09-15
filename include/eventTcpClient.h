/**
 * Copyright (c) 2023-07, typecodes.com (vfhky@typecodes.com)
 *
 * All rights reserved.
 *
 * Tools for tcp client.
 */


#ifndef __SOCKET_REDIS_SENTINEL_EVENT_TCP_CLIENT_H__
#define __SOCKET_REDIS_SENTINEL_EVENT_TCP_CLIENT_H__




#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "utils.h"
#include "common.h"




namespace socketRedisSentinel {




    class EventTcpClient
    {
    public:
        EventTcpClient();
        ~EventTcpClient();


        // make an tcp request
        bool doRequest(const std::string &ip, u_short port, const std::string &reqData, int16_t timeoutMics, std::string &rcvData);





    private:


        static void setTcpNoDelay(evutil_socket_t fd);

        // callback
        static void eventCb(struct bufferevent *bev, short event, void *ctx);
        static void readCb(struct bufferevent* bev, void* ctx);
        static void writeCb(struct bufferevent *bev, void *ctx);
        static void reqTimeoutCb(evutil_socket_t fd, short event, void* arg);

        // add an event of sending data
        void sendDataCb(evutil_socket_t fd, short events, void *arg);
        // send data directly
        bool sendData(struct bufferevent *bev, const std::string &reqData);

        // handle event base
        bool createEventBase();
        event_base* getEventBase() const;
        void freeEventBase();
        void loopExitEventBase();

        // received data
        void setRcvData(char * const rcvData, const size_t &rcvDataSize);
        std::string getRcvData() const;
        size_t getRcvDataSize() const;
        void freeRcvData();

        // check whether the server have send data completely and client recevied completely
        static bool checkReadDataCompleted(const char *receivedData, const size_t &totalReadSize);

        // local ip and port
        void setLocalIp(const char *localIp);
        std::string getLocalIp() const;
        void setLocalPort(const uint16_t &localPort);
        uint16_t getLocalPort() const;
        static void fillLocalIpPort(evutil_socket_t fd, void *ctx);




    private:

        // local ip and port
        std::string m_localIp;
        uint16_t m_localPort;

        // tcp received data
        std::string m_rcvData;
        size_t m_rcvDataSize;

        event_base* m_base;











    };




}







#endif

