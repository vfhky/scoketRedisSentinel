#ifndef __SCOKET_REDIS_SENTINEL_CLIENT_REQ_INFO_H__
#define __SCOKET_REDIS_SENTINEL_CLIENT_REQ_INFO_H__






#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <algorithm>








namespace socketRedisSentinel {








    enum CLIENT_REQ_REDIS_TYPE {
        CLIENT_REQ_REDIS_TYPE_ILLEGAL = 0x00,
        CLIENT_REQ_REDIS_TYPE_MASTER = 0x01,
        CLIENT_REQ_REDIS_TYPE_SLAVE = 0x02,
        CLIENT_REQ_REDIS_TYPE_ALL = 0x03,
    };





    /**
     * 1-get redis info.
     * 2-get target redis info by common hash.
     * 3-get target redis info by crc32 hash.
     * 4-modify log level
    */
    enum CLIENT_REQ_TYPE {
        CLIENT_REQ_TYPE_ILLEGAL = 0,
        CLIENT_REQ_TYPE_REDIS_INFO = 1,
        CLIENT_REQ_TYPE_REDIS_INFO_BY_COM_HASH = 2,
        CLIENT_REQ_TYPE_REDIS_INFO_BY_CRC32_HASH = 3,
        CLIENT_REQ_TYPE_LOG_CFG = 4,
    };



    // http or tcp client req to inner struct
    struct ClientReqInfo {
        /**
         * 1-get redis info.  2/3-get target redis info by hash  4-modify log level/type
        */
        CLIENT_REQ_TYPE type;

        /**
         * domain or ip of sentinel
        */
        std::string ip;

        /**
        * port of sentinel
        */
        uint16_t port;

        /**
         * 1-means get slave redis info.
         * 2-means get master redis info
         * 3-means the result equal : 1 | 2
        */
        CLIENT_REQ_REDIS_TYPE redisType;

        /**
         * when you want to assign special redis poolName for the ip.
         * for example: test_001|test_002
        */
        std::string poolName;


        /**
         * used when reqType equals 2/3 to get target redis ip.
        */
        std::string hashKey;

        int32_t logLv;

        int32_t logType;


        ClientReqInfo() {
            type = CLIENT_REQ_TYPE_ILLEGAL;
            ip.clear();
            port = -1;
            redisType = CLIENT_REQ_REDIS_TYPE_ILLEGAL;
            poolName.clear();
            hashKey.clear();
            logLv = -1;
            logType = -1;
        }


        const std::string dump() const  {
            std::stringstream ss;
            ss << "ClientReqInfo - {"
                << "[type:" << type << "]"
                << "[ip:" << ip << "]"
                << "[port:" << port << "]"
                << "[redisType:" << redisType << "]"
                << "[poolName:" << poolName << "]"
                << "[hashKey:" << hashKey << "]"
                << "[logLv:" << logLv << "]"
                << "[logType:" << logType << "]"
                << "}";

            return ss.str();
        }




    };


















}

#endif

