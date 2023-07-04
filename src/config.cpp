#include "config.h"





namespace socketRedisSentinel {




    LOG_LEVEL Config::m_logLv   = Debug;
    LOG_TYPE  Config::m_logType = LOG_TYPE_STDOUT;



    Config& Config::instance() {
        static Config instance;
        return instance;
    }



    const LOG_LEVEL Config::getLogLv() {
        return m_logLv;
    }

    void Config::setLogLv(const LOG_LEVEL &lv) {
        this->m_logLv = lv;
    }




    const LOG_TYPE Config::getLogType() {
        return m_logType;
    }

    void Config::setLogType(const LOG_TYPE &type) {
        this->m_logType = type;
    }





}

