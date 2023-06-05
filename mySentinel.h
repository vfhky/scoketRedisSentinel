#ifndef __SCOKET_REDIS_SENTINEL_MY_SENTINEL_H__
#define __SCOKET_REDIS_SENTINEL_MY_SENTINEL_H__




#include "mySocket.h"
#include "utils.h"


using namespace std;

namespace scoketRedisSentinel {

    struct RedisInfo {
        // eg. testCache_001
        string name;
        string ip;
        string port;

        bool bMeet() {
            if (!name.empty() && !ip.empty() && !port.empty()) {
                return true;
            }

            return false;
        }


        const string dump() const {
            stringstream ss;
            ss << "RedisInfo - {"
                << "[name:" << name << "]"
                << "[ip:" << ip << "]"
                << "[port:" << port << "]"
                << "}";
            return ss.str();
        }
    };



    class MySentinel
    {
    public:

        ~MySentinel();



        bool init(const string &ip, uint16_t port);


        string getRedisInfo();


        // 解析所有的master ip:port
        list<RedisInfo> getMaster();



        // 根据masterName获取所有的从库
        string getSlaveByMasterName(const string &masterName);

        // 获取所有的从库
        list<RedisInfo> getSlave();




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

        string printListRedisInfo(const list<RedisInfo> &redisInfos);


        uint32_t getHashIndex(const string& key, const int32_t &redisNums);



    private:
        MySocket m_socket;






    };


}




#endif


