#include <limits.h>
#include <iostream>
#include "logger.h"




namespace socketRedisSentinel {









    FILE *Logger::openFile(char *fileName) {
        FILE *fp;
        struct stat tStat;
        char logFile[LOG_FILE_NAME_LEN] = {0x00};
        char oldFile[LOG_FILE_NAME_LEN] = {0x00};
        int iResult;

        sprintf(logFile, "%s.log", fileName);

        memset(&tStat, 0x00, sizeof(tStat));
        iResult = stat(logFile, &tStat);

        if (iResult == 0 && tStat.st_size > LOG_FILE_MAX_BYTES) {
            struct timeval tv;
            ::gettimeofday(&tv,NULL);
            // 2023-06-05 11:11:11
            char buf[64] = {0x00};
            struct tm *t = ::localtime(&tv.tv_sec);
            strftime(buf, sizeof(buf), "%H:%M:%S", t);
            sprintf(oldFile, "%s.%s.log", fileName, buf);
            if (rename(logFile, oldFile) != 0) {
                fprintf(stderr, "rename old:[%s] to new:[%s] err:[%s]\n", \
                        logFile, oldFile, strerror(errno));
            }
        }

        if ((fp = fopen(logFile, "a+")) == NULL) {
            fprintf(stderr, "openFile [%s] failed, errMsg:[%s]\n", logFile, strerror(errno));
            return NULL;
        }
        return fp;
    }

    int Logger::openDir(char *dirName) {
        static char last_dir[100] = {0x00};
        int result = 0;
        int dirLen = strlen(dirName);
        if (dirLen>0 && memcmp(dirName, last_dir, dirLen)) {
            struct stat fileStat;

            if (stat(dirName, &fileStat) != 0) {
                result = mkdir(dirName, 0770);
                if (result==0) {
                    memcpy(last_dir,dirName,dirLen);
                } else {
                    fprintf(stderr, "mkdir failed %s error msg:%d, %s\n", dirName, result, strerror(errno));
                }
                return result;
            }

            memcpy(last_dir,dirName,dirLen);
            return 0;
        } else if (dirLen==0) {
            fprintf(stderr, "dirName:%s is illegal error msg:%d, %s\n", dirName, result, strerror(errno));
            return -1;
        } else {
            return 0;
        }
    }


    void Logger::logToFile(const string &content) {
        FILE       *fp;
        char       logFile[LOG_FILE_NAME_LEN] = {0x00};
        char       dirName[LOG_FILE_NAME_LEN] = {0x00};

        struct timeval tv;
        ::gettimeofday(&tv,NULL);
        // 2023-06-05 11:11:11
        char buf[64] = {0x00};
        struct tm *t = ::localtime(&tv.tv_sec);
        strftime(buf, sizeof(buf), "%Y%m%d", t);

        sprintf(dirName, "%s/%s", LOG_DIR, buf);
        if (0 !=Logger::mkdirP(dirName, S_IRWXU|S_IRWXG|S_IROTH)) {

                std::cout << __FILE__ << ":" << __LINE__ << " exit " << strerror(errno) << std::endl;
            return;
        }

        sprintf(logFile, "%s/%s", dirName, buf);
        if ((fp = Logger::openFile(logFile))== NULL) {
            return;
        }

        fprintf(fp, "%s%s", content.c_str(), "\n");
        fclose(fp);
    }

    int Logger::mkdirP(const char *path, mode_t mode) {
        const size_t len = strlen(path);
        char _path[PATH_MAX] = {0x00};
        char *p;

        errno = 0;

        if (len > sizeof(_path)-1) {
            errno = ENAMETOOLONG;
            std::cout << __FILE__ << ":" << __LINE__ << " exit " << strerror(errno)<< std::endl;
            return -1;
        }
        strcpy(_path, path);

        for (p = _path + 1; *p; p++) {
            if (*p == '/') {
                *p = '\0';
                if (mkdir(_path, mode) != 0) {
                    if (errno != EEXIST) {
                        std::cout << __FILE__ << ":" << __LINE__ << " exit " << strerror(errno) << std::endl;

                        return -1;
                        }
                }
                *p = '/';
            }
        }

        if (mkdir(_path, mode) != 0) {
            if (errno != EEXIST) {
                std::cout << __FILE__ << ":" << __LINE__ << " exit" << std::endl;
                return -1;
            }
        }

        return 0;
    }







}



