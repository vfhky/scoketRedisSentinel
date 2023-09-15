/**
 * Copyright (c) 2023-07, typecodes.com (vfhky@typecodes.com)
 *
 * All rights reserved.
 *
 * Exception on handling RESP of hiRedis.
 */


#include "hiRedisException.h"




namespace socketRedisSentinel {




    HiRedisException::HiRedisException() : mErrMsg("Error.") {
    }

    HiRedisException::HiRedisException(const std::string &errMsg)
            : mErrMsg("Error : " + errMsg) {
    }


    HiRedisException::~HiRedisException() throw () {
        this->mErrMsg.clear();
    }

    const char* HiRedisException::what() const throw () {
        return this->mErrMsg.c_str();
    }








}

