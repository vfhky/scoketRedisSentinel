#ifndef __SCOKET_REDIS_SENTINEL_CONFIG_H__
#define __SCOKET_REDIS_SENTINEL_CONFIG_H__



#include "common.h"



using namespace std;


namespace socketRedisSentinel {




    class Config
    {
    public:

        static Config& instance();

        static const LOG_LEVEL getLogLv();
        void setLogLv(const LOG_LEVEL &lv);

        static const LOG_TYPE getLogType();
        void setLogType(const LOG_TYPE &type);




    private:








    private:

        static LOG_LEVEL m_logLv;
        static LOG_TYPE m_logType;





    };



}




#endif


