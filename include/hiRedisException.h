/**
 * Copyright (c) 2023-07, typecodes.com (vfhky@typecodes.com)
 *
 * All rights reserved.
 *
 * Exception on handling RESP of hiRedis.
 */


#ifndef __SCOKET_REDIS_SENTINEL_HI_REDIS_EXCEPTION_H__
#define __SCOKET_REDIS_SENTINEL_HI_REDIS_EXCEPTION_H__


#include <string>
#include "myException.h"





namespace socketRedisSentinel {


    class HiRedisException : public MyException {

    public:
        HiRedisException();

        HiRedisException(const std::string &errMsg);

        ~HiRedisException() throw ();

        virtual const char* what() const throw ();

    private:

        std::string mErrMsg;

    };


}




#endif
