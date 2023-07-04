#ifndef __SCOKET_REDIS_SENTINEL_LOGIC_ENTRANCE_H__
#define __SCOKET_REDIS_SENTINEL_LOGIC_ENTRANCE_H__




#include "common.h"
#include "utils.h"
#include "mySentinel.h"


using namespace std;

namespace socketRedisSentinel {




    enum CLIENT_REQ_TYPE{
        CLIENT_REQ_TYPE_REDIS_INFO = 1,
        CLIENT_REQ_TYPE_LOG_LEVEL = 2,
    };


    enum CLIENT_REQ_REDIS_TYPE {
        CLIENT_REQ_REDIS_TYPE_MASTER = 0x01,
        CLIENT_REQ_REDIS_TYPE_SLVAVE = 0x02,
        CLIENT_REQ_REDIS_TYPE_ALL = 0x03,
    };







    class LogicEntrance
    {
    public:

        static LogicEntrance& instance();


        string handleReq(const string &req);





    private:
        static string help();
        static string makeRspData(const CLIENT_REQ_REDIS_TYPE &type, const list<RedisInfo> &infos);

        string handleGetRedisInfoReq(const vector<string> &tokens);
        string handleResetLogLevelReq(const vector<string> &tokens);




    private:





    };




}







#endif

