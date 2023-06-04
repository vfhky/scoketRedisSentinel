#include "mySentinel.h"
#include "utils.h"



using namespace myRedisSentinel;


int main(int argc, char const* argv[])
{
    if(argc != 4) {
        std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << " " <<"argc=[" << argc << "] getRedisFromSentinel type/1-domain/2-ip sentinelIP sentinelPort" << std::endl;
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



    MySentinel cmd;
    if(!cmd.init(ip, port)) {
        std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << ", init failed ip=[" << ip << "] port=[" << port << "]" << std::endl;
        return 1;
    }

    map<string, string> mapInfo = cmd.getSlave();


    return 0;
}

