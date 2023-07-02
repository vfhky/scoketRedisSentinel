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


#if 0
    if(argc < 3) {
        LOG(Error, argc, "usage: socketRedisSentinel sentinelIP/domain sentinelPort [logLv] [type:1-slave 2-master] [hash]");
        return 0;
    }

    bool bIp = RedisSentinelUtils::simpleCheckIpStr(argv[1]);
    string ip;
    if (!bIp) { // 传入域名
        list<string> ipList = RedisSentinelUtils::domain2ip(argv[1]);
        if (ipList.empty()) {
            return 1;
        }
        ip = *ipList.begin();
    } else {    // 直接ip
        ip = argv[1];
    }
    uint16_t port = atoi(argv[2]);

    // 日志级别
    if (argc > 3) {
        syslogLevel = atoi(argv[3]);
    }

    // hash : hash_001|hash_002|...
    string hash = "";
    if (argc > 4) {
        hash = argv[4];
    }

    MySentinel cmd;
    if (!cmd.init(ip, port)) {
        LOG(Error, "init failed", ip, port);
        return 1;
    }

    cmd.pharseSlave();


    // master redis infos
    list<RedisInfo> allMasterRedis = cmd.getRedisByHash(2, hash);
    __foreach(it, allMasterRedis) {
        LOG(Info, it->dump());
    }

    // slave redis infos
    list<RedisInfo> hashSlaveRedis = cmd.getRedisByHash(1, hash);
    __foreach(it, hashSlaveRedis) {
        LOG(Info, it->dump());
    }
#endif


    return 0;
}

