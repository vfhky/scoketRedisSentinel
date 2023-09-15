/**
 * Copyright (c) 2023-07, typecodes.com (vfhky@typecodes.com)
 *
 * All rights reserved.
 *
 * Simply define an exception class for project.
 */


#ifndef __SOCKET_REDIS_SENTINEL_MY_EXCPTION_H__
#define __SOCKET_REDIS_SENTINEL_MY_EXCPTION_H__





#include <iostream>
#include <vector>
#include <string>




namespace socketRedisSentinel {


    class MyException {

    public:

        MyException();

        MyException(const std::string &errMsg);

        virtual ~MyException();


        virtual const char* what() const;



    protected:

        std::string mErrMsg;




    };





}




#endif
