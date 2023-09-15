/**
 * Copyright (c) 2023-07, typecodes.com (vfhky@typecodes.com)
 *
 * All rights reserved.
 *
 * Simply define an exception class for project.
 */


#include "myException.h"





namespace socketRedisSentinel {



    MyException::MyException() {

    }

    MyException::MyException(const std::string &errMsg) : mErrMsg(errMsg) {

    }

    MyException::~MyException() {
        this->mErrMsg.clear();
    }


    const char* MyException::what() const {
        return this->mErrMsg.c_str();
    }


}