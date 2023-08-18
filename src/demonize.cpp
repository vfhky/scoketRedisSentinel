#include "demonize.h"




namespace socketRedisSentinel {




    void Demonize::initSrv(const LOG_LEVEL &sysLogLv, const LOG_TYPE &sysLogType) {
        LOG(Info, sysLogLv, sysLogType);

        pid_t pid = fork();
        if (pid < 0) {
            LOG(Error, "first fork failed", sysLogLv, sysLogType, strerror(errno));
            exit(-1);
        } else if (pid != 0) {
            exit(0);
        }

        setsid();

        if ((pid = fork()) < 0) {
            LOG(Error, "second fork failed", sysLogLv, sysLogType, strerror(errno));
            exit(-1);
        } else if (pid != 0) {
            exit(0);
        }

        const char *dir = getcwd(NULL, 0);
        if (chdir(dir) < 0) {
            LOG(Error, "chdir failed", sysLogLv, sysLogType, strerror(errno));
            exit(1);
        }
        LOG(Info, "chdir to ", dir);

        // to support console output , do not remove this depress.
        #if 0
            close(STDIN_FILENO);
            close(STDOUT_FILENO);
            close(STDERR_FILENO);
        #endif

        Config::instance().setLogLv(sysLogLv);
        Config::instance().setLogType(sysLogType);

        // init tcp server
        EventServer server;
        if (0 != server.init()) {
            LOG(Error, 0, "EventServer init tcp server failed", sysLogLv, sysLogType);
            exit(2);
        }

        LOG(Error, "server exit ", sysLogLv, sysLogType);
    }



















}

