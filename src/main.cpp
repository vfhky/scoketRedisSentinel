#include "mySentinel.h"
#include "utils.h"
#include "libEventServer.h"



using namespace scoketRedisSentinel;



int main(int argc, char const* argv[])
{
    // syslogLevel = Error;
    syslogLevel = Info;

    // init tcp server
    LibEventServer server;
    if (0 != server.init()) {
        LOG(Error, 0, "LibEventServer init tcp server failed");
        return 1;
    }


    return 0;
}

