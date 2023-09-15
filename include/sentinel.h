/**
 * Copyright (c) 2023-07, typecodes.com (vfhky@typecodes.com)
 *
 * All rights reserved.
 *
 * Tools for hanlding sentinel.
 */


#ifndef __SCOKET_REDIS_SENTINEL_SENTINEL_H__
#define __SCOKET_REDIS_SENTINEL_SENTINEL_H__




#include "utils.h"
#include "clientReqInfo.h"




namespace socketRedisSentinel {

    struct RedisInfo {
        // eg. testCache_001
        std::string name;
        std::string ip;
        std::string port;

        bool bMeet() {
            if (!name.empty() && !ip.empty() && !port.empty()) {
                return true;
            }

            return false;
        }


        const std::string dump() const {
            std::stringstream ss;
            ss << "RedisInfo - {"
                << "[name:" << name << "]"
                << "[ip:" << ip << "]"
                << "[port:" << port << "]"
                << "}";
            return ss.str();
        }
    };



    class Sentinel
    {
    public:

        Sentinel(const std::string &ip, const uint16_t &port);


        std::string getRedisInfo();


        // 解析所有的master ip:port
        std::list<RedisInfo> getMaster();



        // 根据masterName获取所有的从库
        std::string getSlaveByMasterName(const std::string &masterName);

        // 解析所有的从库到内存
        std::list<RedisInfo> pharseSlave();


        /**
         * @param type 1-master 2-slave
         * @param hashStr format：hash_001|hash_0002|....
         * @return master or slave redis infos
        */
        std::list<RedisInfo> getRedisByType(const uint32_t &type, const std::string &hashStr);



        uint32_t redisComHash(const std::string& key, const int32_t &redisNums);
        uint32_t redisCrc32Hash(const std::string &key, const int32_t &redisNum);



        std::map<std::string/*hash*/, std::map<uint32_t, RedisInfo> > getSlaveRedis();
        void setSlaveRedis(const std::map<std::string/*hash*/, std::map<uint32_t, RedisInfo> > &info);
        std::map<std::string/*hash*/, std::map<uint32_t, RedisInfo> > getMasterRedis();
        void setMasterRedis(const std::map<std::string/*hash*/, std::map<uint32_t, RedisInfo> > &info);



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
        std::list<std::map<std::string, std::string> > parseSlaveInfo(const std::string &slavesInfo);

        // 解析 info 命令返回的数据
        std::map<std::string, std::string> parseRedisInfo(const std::string &redisInfo);

        std::string printListRedisInfo(const std::list<RedisInfo> &redisInfos);



    private:

        // ip and port of the tcp server.
        std::string m_srvIp;
        uint16_t m_srvPort;

        std::map<std::string/*hash*/, std::map<uint32_t, RedisInfo> > m_slaveRedis;
        std::map<std::string/*hash*/, std::map<uint32_t, RedisInfo> > m_masterRedis;






    };


}




#endif


