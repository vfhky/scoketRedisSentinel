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

        static MySentinel& instance();


        ~MySentinel();



        bool init(const string &ip, uint16_t port);


        string getRedisInfo();


        // 解析所有的master ip:port
        list<RedisInfo> getMaster();



        // 根据masterName获取所有的从库
        string getSlaveByMasterName(const string &masterName);

        // 解析所有的从库到内存
        list<RedisInfo> pharseSlave();


        /**
         * @param type 1-master 2-slave
         * @param hashStr format：hash_001|hash_0002|....
         * @return master or slave redis infos
        */
        list<RedisInfo> getRedisByHash(const uint32_t &type, const string &hashStr);

        uint32_t getHashIndexCrc32(const string &key, const int32_t &redisNum);




        void close();



        map<string/*hash*/, map<uint32_t, RedisInfo> > getSlaveRedis();
        void setSlaveRedis(const map<string/*hash*/, map<uint32_t, RedisInfo> > &info);
        map<string/*hash*/, map<uint32_t, RedisInfo> > getMasterRedis();
        void setMasterRedis(const map<string/*hash*/, map<uint32_t, RedisInfo> > &info);



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

        map<string/*hash*/, map<uint32_t, RedisInfo> > m_slaveRedis;
        map<string/*hash*/, map<uint32_t, RedisInfo> > m_masterRedis;






    };


}




#endif


