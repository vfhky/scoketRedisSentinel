#include "mySentinel.h"
#include "utils.h"



using namespace myRedisSentinel;



int main(int argc, char const* argv[])
{
    if(argc < 4) {
        LOG(Error, argc, "usage:getRedisFromSentinel type/1-domain/2-ip "\
                "sentinelIP sentinelPort [logLv]");
        return 0;
    }


    // 1-传入域名 2-传入ip
    uint32_t type = RedisSentinelUtils::stringToU32(argv[1]);
    string ip;
    // 传入域名
    if(1 == type) {
        list<string> ipList = RedisSentinelUtils::domain2ip(argv[2]);
        if(ipList.empty()) {
            return 1;
        }
        ip = *ipList.begin();
    } else {    // 直接ip
        ip = argv[2];
    }
    uint16_t port = atoi(argv[3]);

    // 日志级别
    syslogLevel = Error;
    if (5 == argc) {
        syslogLevel = atoi(argv[4]);
    }

    MySentinel cmd;
    if(!cmd.init(ip, port)) {
        LOG(Error, "init failed", ip, port);
        return 1;
    }

    map<string, string> mapInfo = cmd.getSlave();


    return 0;
}

