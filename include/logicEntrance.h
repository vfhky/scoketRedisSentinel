/**
 * Copyright (c) 2023-07, typecodes.com (vfhky@typecodes.com)
 *
 * All rights reserved.
 *
 * Entrance of logic handling.
 */


#ifndef __SOCKET_REDIS_SENTINEL_LOGIC_ENTRANCE_H__
#define __SOCKET_REDIS_SENTINEL_LOGIC_ENTRANCE_H__




#include "common.h"
#include "utils.h"
#include "sentinel.h"
#include "clientReqInfo.h"


using namespace std;

namespace socketRedisSentinel {





    class LogicEntrance
    {
    public:

        static LogicEntrance& instance();


        string handleReq(const ClientReqInfo &reqInfo);





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

