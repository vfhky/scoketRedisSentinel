#ifndef __SCOKET_REDIS_SENTINEL_EVENT_TCP_CLIENT_H__
#define __SCOKET_REDIS_SENTINEL_EVENT_TCP_CLIENT_H__




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






    private:

        // tcp recivied data
        static std::string m_rcvData;











    };




}







#endif

