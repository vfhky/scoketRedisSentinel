#ifndef __SCOKET_REDIS_SENTINEL_LOGGER_H__
#define __SCOKET_REDIS_SENTINEL_LOGGER_H__



#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <string>
#include <errno.h>



using namespace std;


namespace socketRedisSentinel {


    static const int LOG_FILE_NAME_LEN = 256;
    static const char LOG_DIR[LOG_FILE_NAME_LEN] = "./log";
    static const int LOG_FILE_MAX_BYTES = 200 * 1024 * 1024;




    class Logger
    {
    public:

        static FILE *openFile(char *fileName);
        static int openDir(char *dirName);
        static void logToFile(const string &content);
        static int mkdirP(const char *path, mode_t mode);




    private:








    private:




    };



}




#endif


