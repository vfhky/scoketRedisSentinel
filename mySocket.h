#ifndef __MY_REDIS_SENTINEL_SOCKET_H__
#define __MY_REDIS_SENTINEL_SOCKET_H__




#include "common.h"


using namespace std;

namespace myRedisSentinel {
    class MySocket
    {
    public:
        MySocket();


        bool connect(const string &ip, const uint16_t &port);

        void setSocketOpt(const int64_t &rcvSec, const int64_t &sendSec);

        bool send(const string &message);


        bool recv(string &message);

        int selectTimeOut(const int &sec, const int &usec);

        void close();

        int getClientFd();


    private:


        int m_client_fd;
    };




}







#endif

