#include "mySentinel.h"





namespace myRedisSentinel {



    MySentinel::~MySentinel() {
        if( m_socket.getClientFd() ) {
            m_socket.close();
        }
    }




    bool MySentinel::init(const string &ip, uint16_t port) {
        if(!m_socket.connect(ip, port)) {
            std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << ", connect sentinel failed ip=[" << ip << "] port=[" << port << "]" << std::endl;
            return false;
        }

        std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << ", connect sentinel ok ip=[" << ip << "] port=[" << port << "]" << std::endl;
        return true;
    }


    string MySentinel::getRedisInfo() {
        string message = "info";
        if(!m_socket.send(message)) {
            std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << ", send mgs failed message=[" << message << "]" << std::endl;
            return "";
        }

        // receive message
        string redisInfo;
        if(!m_socket.recv(redisInfo)) {
            std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << ", recv mgs failed redisInfo=[" << redisInfo << "]" << std::endl;
            return "";
        }

        return redisInfo;
    }


    // 解析所有的master ip:port
    map<string,string> MySentinel::getMaster() {
        map<string,string> masterMap;
        string redisInfo = this->getRedisInfo();
        if (redisInfo.empty()) {
            return masterMap;
        }

        map<string, string> mapInfo = this->parseRedisInfo(redisInfo);

        // 解析master数量
        uint32_t masterNums = 0;
        if (mapInfo.find("sentinel_masters") != mapInfo.end()) {
            masterNums = RedisSentinelUtils::stringToU32(mapInfo["sentinel_masters"]);
        }

        for(uint32_t index=0; index<masterNums; index++) {
            string key = "master" + RedisSentinelUtils::toString(index);

            // master0:name=meipaiCache_001,status=ok,address=10.26.11.239:4038,slaves=1,sentinels=3
            if( mapInfo.find(key) == mapInfo.end() ) {
                std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << ", masterNums=[" << masterNums << "] but cannot find index key=[" << key << "]" << std::endl;
                masterMap.clear();
                return masterMap;
            }

            // IP:PORT
            vector<string> vect = RedisSentinelUtils::splitStr(mapInfo[key], ",");
            string masterName;
            string masterIpPort;
            __foreach(it, vect) {
                vector<string> vect2 = RedisSentinelUtils::splitStr(*it, "=");
                if(2 == vect2.size() ) {
                    if("name" == vect2[0]) {
                        masterName = vect2[1];
                    }
                    if("address" == vect2[0]) {
                        masterIpPort = vect2[1];
                    }
                }
            }

            if(!masterName.empty() && !masterIpPort.empty()) { // master name
                masterMap[masterName] = masterIpPort;
            }
        }

        // 再次校验数量是否一致
        if(masterNums != masterMap.size()) {
            std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << ", masterNums=[" << masterNums << "] but masterMap.size=[" << masterMap.size() << "]" << std::endl;
            masterMap.clear();
        }

        std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << ", mapInfo.size=[" << mapInfo.size() << "] masterMap=[" << RedisSentinelUtils::printMap(masterMap) << "]" << std::endl;
        return masterMap;
    }



    // 根据masterName获取所有的从库
    string MySentinel::getSlaveByMasterName(const string &masterName) {
        const string message = string("sentinel slaves " ) + masterName;
        if( !m_socket.send(message)) {
            std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << ", send mgs failed message=[" << message << "]" << std::endl;
            return "";
        }

        // receive message
        string slavesInfo;
        if (!m_socket.recv(slavesInfo)) {
            std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << ", recv mgs failed slavesInfo=[" << slavesInfo << "]" << std::endl;
            return "";
        }

        return slavesInfo;
    }

    // 获取所有的从库
    map<string, string> MySentinel::getSlave() {
        map<string,string> slaveMap;

        // 所有主库
        map<string, string> masterMap = this->getMaster();
        __foreach(it, masterMap) {
            const string masterName = it->first;

            // 根据主播获得从库
            string slavesInfo = this->getSlaveByMasterName(masterName);
            list<map<string, string> > listInfo = this->parseSlaveInfo(slavesInfo);
            __foreach(it2, listInfo) {
                const map<string, string> &singleSlave = *it2;
                if (singleSlave.find("flags") == singleSlave.end() || singleSlave.at("flags") != "slave") {
                    std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << ", illgeal slave=[" << RedisSentinelUtils::printMap(singleSlave) << "]" << std::endl;
                    break;
                }

                // ip:port
                if (singleSlave.find("name")!= singleSlave.end()) {
                    slaveMap[masterName] = singleSlave.at("name");
                    break;
                }
            }
        }

        std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << ", masterMap.size=[" << masterMap.size() << "] slaveMap=[" << RedisSentinelUtils::printMap(slaveMap) << "]" << std::endl;
        return slaveMap;
    }




    void MySentinel::close() {
        std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << ", cloese m_client_fd=[" << m_socket.getClientFd() << "]" << std::endl;
        if( m_socket.getClientFd() ) {
            m_socket.close();
        }
    }




    /**
     * 解析 slave 命令返回的数据
     * *1
     * *40
     * $4
     * name
     * $16
     * 10.25.70.78:4033
     * $2
     * ip
     * $11
     * 10.25.70.78
     */
    list<map<string, string> > MySentinel::parseSlaveInfo(const string &slavesInfo) {
        list<map<string, string> > slaveList;
        if (slavesInfo.empty()) {
            return slaveList;
        }

        stringstream data;

        uint32_t slaveNums = 0;

        map<string, string> singleSlave;
        string key;
        string value;

        // -1-初始 0-获取slave数量 1-解析新的一组slave的变量数量 2-key的长度 3-key的值 3-value的长度 4-value的值
        int flag = -1;
        for (size_t index = 0; index < (slavesInfo.size()); index++) {
            char c = slavesInfo[index];

            // 设置当前的标志
            if (0 == index && '*' == c) {
                flag = 0;
                continue;
            }

            // 换行
            if (0x0d == c) {
                continue;
            }

            // 切换到1组新的slave
            if ('*' == c) {
                flag = 1;
                continue;
            }

            if ('$' == c) {
                if (1 == flag || 5 == flag) {    // 解析完slave变量数量后，或者解析完value的值后，开始解析key的长度
                    flag = 2;
                } else if (2 == flag) {     // 解析完key的长度，开始解析key的值
                    flag = 3;
                } else if (3 == flag) {     // 解析完key的值，开始解析value的长度
                    flag = 4;
                } else if (4 == flag) {     // 解析完value的长度，开始解析value的值
                    flag = 5;
                } else {
                    std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << ", unkown flag=[" << flag << "]" << std::endl;
                }
                continue;
            }

            // 结算
            if (0x0a == c) {
                if (0 == flag) {    // 获取slave数量
                    slaveNums = RedisSentinelUtils::stringToU32(data.str());
                } else if (1 == flag) {    // 解析新的一组slave中key数量*value数量
                    if (!singleSlave.empty()) {
                        std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << ", singleSlave=[" << RedisSentinelUtils::printMap(singleSlave) << "]" << std::endl;
                        slaveList.push_back(singleSlave);
                        singleSlave.clear();
                    }
                } else if (2 == flag) { // key的长度获取完毕，进入获取key的值
                    flag = 3;
                } else if (3 == flag) { // key的值
                    key = data.str();
                } else if (4 == flag) { // value的长度获取完毕，进入获取value的值
                    flag = 5;
                }  else if (5 == flag) { // value的值
                    value = data.str();
                    singleSlave[key] = value;
                    key.clear();
                    value.clear();
                } else {
                    slaveList.clear();
                    std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << ", illegal flag=[" << flag << "]" << std::endl;
                    return slaveList;
                }

                data.str("");
                data.clear();
                continue;
            }

            // 正常数据
            data << c;
        }

        if (!singleSlave.empty()) {
            slaveList.push_back(singleSlave);
        }

        // 校验数量是否一致
        if (slaveNums != slaveList.size()) {
            std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << ", slaveNums=[" <<slaveNums << "] not equal slaveList.size=[" << slaveList.size() << "]" << std::endl;
            slaveList.clear();
        } else {
            //std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << ", slaveNums=[" <<slaveNums << "] slaveList.size=[" << slaveList.size() << "]" << std::endl;
        }

        return slaveList;
    }

    // 解析 info 命令返回的数据
    map<string, string> MySentinel::parseRedisInfo(const string &redisInfo) {
        stringstream key;

        map<string, string> map;
        for(size_t index = 0; index < (redisInfo.size()); index++) {
            char c = redisInfo[index];

            // 过滤换行
            if(c == 0x0d) {
                continue;
            }

            if(c == 0x0a) {
                if(key.str().empty()) {
                    continue;
                }

                // 过滤以 $ 或者 # 开头的内容
                if(key.str()[0] == '$' || key.str()[0] == '#') {
                    //std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << ", key=[" << key.str() << "] filter" << std::endl;
                    key.str("");
                    key.clear();
                    continue;
                }

                // 找到第1个:作为key
                string keyStr = key.str();
                string::size_type pos = keyStr.find(":");
                if(pos!= string::npos) {
                    map[keyStr.substr(0, pos)] = keyStr.substr(pos + 1);
                }

                key.str("");
                key.clear();
                continue;
            }

            key << c;
        }

        return map;
    }


}

