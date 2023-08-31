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



using namespace std;

namespace socketRedisSentinel {





    class EventTcpClient
    {
    public:


        bool doRequest(const std::string &ip, u_short port, const string &reqData, int16_t timeoutMics, string &rcvData);





    private:

        event_base* getEventBase() const;
        bool setEventBase(const event_base* base);


        static void setTcpNoDelay(evutil_socket_t fd);

        static void eventCb(struct bufferevent *bev, short event, void *ctx);
        static void readCb(struct bufferevent* bev, void* ctx);
        static void writeCb(struct bufferevent *bev, void *ctx);
        static void reqTimeoutCb(evutil_socket_t fd, short event, void* arg);

        void sendDataCb(evutil_socket_t fd, short events, void *arg);
        bool sendData(struct bufferevent *bev, const string &reqData);






    private:

        static string m_rcvData;











    };

    string EventTcpClient::m_rcvData = "";




}







#endif

