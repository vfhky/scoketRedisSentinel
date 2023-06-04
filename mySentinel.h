#ifndef __MY_REDIS_SENTINEL_SENTINEL_H__
#define __MY_REDIS_SENTINEL_SENTINEL_H__




#include "mySocket.h"
#include "utils.h"


using namespace std;

namespace myRedisSentinel {



    class MySentinel
    {
    public:

        ~MySentinel();



        bool init(const string &ip, uint16_t port);


        string getRedisInfo();


        // 解析所有的master ip:port
        map<string,string> getMaster();



        // 根据masterName获取所有的从库
        string getSlaveByMasterName(const string &masterName);

        // 获取所有的从库
        map<string, string> getSlave();




        void close();




    private:

        /**
         * 解析 slave 命令返回的数据
         * *1
         * *40
         * $4
         * name
         * $16
         * 10.25.70.78:4033
         * $2
         * ip
         * $11
         * 10.25.70.78
         */
        list<map<string, string> > parseSlaveInfo(const string &slavesInfo);

        // 解析 info 命令返回的数据
        map<string, string> parseRedisInfo(const string &redisInfo);


    private:
        MySocket m_socket;






    };


}




#endif


