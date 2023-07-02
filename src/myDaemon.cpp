#include "myDaemon.h"
#include "utils.h"




namespace socketRedisSentinel {





bool MyDaeMon::createDaemon() {
    pid_t pid = fork();

    if (pid < 0) {
        LOG(Error, "fork failed", pid);
        return false;
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    if (setsid() < 0) {
        LOG(Error, "setsid failed", pid);
        return false;
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    chdir("/");

    umask(0);

    return true;
}















}

