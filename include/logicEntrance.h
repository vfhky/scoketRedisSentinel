#ifndef __SCOKET_REDIS_SENTINEL_LOGIC_ENTRANCE_H__
#define __SCOKET_REDIS_SENTINEL_LOGIC_ENTRANCE_H__




#include "common.h"
#include "utils.h"
#include "sentinel.h"


using namespace std;

namespace socketRedisSentinel {




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



    struct ClientReqInfo {
        /**
         * 1-get redis info.  2/3-get target redis info by hash  4-modify log level/type
        */
        CLIENT_REQ_TYPE type;

        /**
         * domain or ip of sentinel
        */
        string ip;

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
        string poolName;


        /**
         * used when reqType equals 2/3 to get target redis ip.
        */
        string hashKey;

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


        const string dump() const  {
            stringstream ss;
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





    class LogicEntrance
    {
    public:

        static LogicEntrance& instance();


        string handleReq(const string &req);

        ClientReqInfo pharseReq(const string &req);





    private:
        static string help();
        static string makeRspData(const CLIENT_REQ_REDIS_TYPE &type, const list<RedisInfo> &infos);
        static string makeRspData(const CLIENT_REQ_REDIS_TYPE &type, const RedisInfo &info);

        string handleGetRedisInfoReq(const ClientReqInfo &req);
        string handleRedisInfoByHashReq(const ClientReqInfo &req) ;
        string handleResetLogLevelReq(const ClientReqInfo &req);




    private:





    };




}







#endif

