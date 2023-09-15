/**
 * Copyright (c) 2023-07, typecodes.com (vfhky@typecodes.com)
 *
 * All rights reserved.
 *
 * Server demonize.
 */


#ifndef __SCOKET_REDIS_SENTINEL_DEMONIZE_H__
#define __SCOKET_REDIS_SENTINEL_DEMONIZE_H__




#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>


#include "utils.h"
#include "eventServer.h"






namespace socketRedisSentinel {




    class Demonize {


    public:


        static void initSrv(const LOG_LEVEL &sysLogLv, const LOG_TYPE &sysLogType);



    private:




    };













}


















#endif

