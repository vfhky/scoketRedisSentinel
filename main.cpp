#include "mySentinel.h"
#include "utils.h"



using namespace scoketRedisSentinel;



int main(int argc, char const* argv[])
{
    if(argc < 3) {
        LOG(Error, argc, "usage:getRedisFromSentinel "\
                "sentinelIP sentinelPort [logLv]");
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
    syslogLevel = Error;
    if (4 == argc) {
        syslogLevel = atoi(argv[3]);
    }

    MySentinel cmd;
    if (!cmd.init(ip, port)) {
        LOG(Error, "init failed", ip, port);
        return 1;
    }

    list<RedisInfo> mapInfo = cmd.getSlave();
    __foreach(it, mapInfo) {
        LOG(Info, "get slave info from sentinel", it.dump(), it.2dump());
    }


    return 0;
}

