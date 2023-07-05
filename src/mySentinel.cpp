#include "mySentinel.h"





namespace socketRedisSentinel {



    MySentinel& MySentinel::instance() {
        static MySentinel instance;
        return instance;
    }



    MySentinel::~MySentinel() {
        if( m_socket.getClientFd() ) {
            m_socket.close();
        }
    }




    bool MySentinel::init(const string &ip, uint16_t port) {
        if(!m_socket.connect(ip, port)) {
            LOG(Error, "connect sentinel failed", ip, port);
            return false;
        }

        LOG(Debug, "connect sentinel ok", ip, port);
        return true;
    }


    string MySentinel::getRedisInfo() {
        string message = "info";
        if(!m_socket.send(message)) {
            LOG(Error, "send mgs failed", message);
            return "";
        }

        // receive message
        string redisInfo;
        if(!m_socket.recv(redisInfo)) {
            LOG(Error, "recv mgs failed", redisInfo);
            return "";
        }

        return redisInfo;
    }


    // 解析所有的master ip:port
    list<RedisInfo> MySentinel::getMaster() {
        list<RedisInfo> masterList;
        string redisInfo = this->getRedisInfo();
        if (redisInfo.empty()) {
            return masterList;
        }

        map<string, string> mapInfo = this->parseRedisInfo(redisInfo);

        // 解析master数量
        uint32_t masterNums = 0;
        if (mapInfo.find("sentinel_masters") != mapInfo.end()) {
            masterNums = Utils::stringToU32(mapInfo["sentinel_masters"]);
        }

        for (uint32_t index=0; index<masterNums; index++) {
            string key = "master" + Utils::toString(index);

            // master0:name=testCache_001,status=ok,address=10.26.11.239:4038,slaves=1,sentinels=3
            if (mapInfo.find(key) == mapInfo.end()) {
                LOG(Error, "cannot find index", key, masterNums);
                masterList.clear();
                return masterList;
            }

            // IP:PORT
            vector<string> vect = Utils::splitStr(mapInfo[key], ",");
            RedisInfo redisInfo;
            __foreach(it, vect) {
                vector<string> vect2 = Utils::splitStr(*it, "=");
                if (2 == vect2.size()) {
                    if ("name" == vect2[0]) {
                        redisInfo.name = vect2[1];
                    }
                    if ("address" == vect2[0]) {
                        const string &address = vect2[1];
                        vector<string> vect3 = Utils::splitStr(address, ":");
                        if (2 == vect3.size()) {
                            redisInfo.ip = vect3[0];
                            redisInfo.port = vect3[1];
                        }
                    }
                }
            }

            if (redisInfo.bMeet()) {
                masterList.push_back(redisInfo);
            }
        }

        // 再次校验数量是否一致
        if (masterNums != masterList.size()) {
            LOG(Error, "not equal", masterNums, masterList.size());
            masterList.clear();
        }

        // convert to map and set into memory.
        map<string/*hash*/, map<uint32_t, RedisInfo> > allMasterInfo;
        __foreach(it, masterList) {
            if (allMasterInfo.find(it->name) != allMasterInfo.end()) {
                uint32_t index = allMasterInfo[it->name].size() + 1;
                allMasterInfo[it->name][index] = *it;
            } else {
                allMasterInfo[it->name][1] = *it;
            }
        }
        this->setMasterRedis(allMasterInfo);

        LOG(Info, mapInfo.size(), this->printListRedisInfo(masterList));
        return masterList;
    }



    // 根据masterName获取所有的从库
    string MySentinel::getSlaveByMasterName(const string &masterName) {
        const string message = string("sentinel slaves " ) + masterName;
        if (!m_socket.send(message)) {
            LOG(Error, "send mgs failed", message);
            return "";
        }

        // receive message
        string slavesInfo;
        if (!m_socket.recv(slavesInfo)) {
            LOG(Error, "recv mgs failed", slavesInfo);
            return "";
        }

        return slavesInfo;
    }

    // 获取所有的从库
    list<RedisInfo> MySentinel::pharseSlave() {
        list<RedisInfo> slaveList;

        list<RedisInfo> masterList = this->getMaster();
        __foreach(it, masterList) {
            const string &masterName = it->name;

            string slavesInfo = this->getSlaveByMasterName(masterName);
            list<map<string, string> > listInfo = this->parseSlaveInfo(slavesInfo);
            __foreach(it2, listInfo) {
                const map<string, string> &singleSlave = *it2;
                if (singleSlave.find("flags") == singleSlave.end() || singleSlave.at("flags") != "slave") {
                    LOG(Error, "illgeal slave", Utils::printMap(singleSlave));
                    break;
                }

                RedisInfo redisInfo;

                // ip:port
                if (singleSlave.find("name")!= singleSlave.end()) {
                    const string &address = singleSlave.at("name");
                    vector<string> vect1 = Utils::splitStr(address, ":");
                    LOG(Debug, masterName, Utils::printList(vect1));
                    if (2 == vect1.size()) {
                        redisInfo.name = masterName;
                        redisInfo.ip = vect1[0];
                        redisInfo.port = vect1[1];
                    }
                }

                string ip;
                if (singleSlave.find("ip") != singleSlave.end()) {
                    ip = singleSlave.at("ip");
                }

                string port;
                if (singleSlave.find("port") != singleSlave.end()) {
                    port = singleSlave.at("port");
                }

                // check ip and port again
                if (!redisInfo.bMeet() || redisInfo.ip != ip || redisInfo.port != port) {
                    slaveList.clear();
                    LOG(Error, "illegal slave", redisInfo.dump(), ip, port);
                    break;
                }

                slaveList.push_back(redisInfo);
            }
        }

        // convert to map and set into memory.
        map<string/*hash*/, map<uint32_t, RedisInfo> > allSlaveInfo;
        __foreach(it, slaveList) {
            if (allSlaveInfo.find(it->name) != allSlaveInfo.end()) {
                uint32_t index = allSlaveInfo[it->name].size() + 1;
                allSlaveInfo[it->name][index] = *it;
            } else {
                allSlaveInfo[it->name][1] = *it;
            }
        }
        this->setSlaveRedis(allSlaveInfo);

        LOG(Info, masterList.size(), this->printListRedisInfo(slaveList));
        return slaveList;
    }




    void MySentinel::close() {
        LOG(Debug, m_socket.getClientFd());
        if (m_socket.getClientFd()) {
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
                    LOG(Error, "unkown flag", flag);
                }
                continue;
            }

            // 结算
            if (0x0a == c) {
                if (0 == flag) {    // 获取slave数量
                    slaveNums = Utils::stringToU32(data.str());
                } else if (1 == flag) {    // 解析新的一组slave中key数量*value数量
                    if (!singleSlave.empty()) {
                        LOG(Debug, "new slave start", flag);
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
                    LOG(Error, "illegal flag", flag);
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
            LOG(Error, "not equal", slaveNums, slaveList.size());
            slaveList.clear();
        }

        LOG(Debug, slaveNums, slaveList.size(), Utils::printListOfMap(slaveList));
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
                    LOG(Debug, "filter", key.str());
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


    string MySentinel::printListRedisInfo(const list<RedisInfo> &redisInfos) {
        stringstream ss;
        ss << "[size:" << redisInfos.size();
        __foreach(it, redisInfos) {
            if (it != redisInfos.begin()) {
                ss << " | ";
            } else {
                ss << " ";
            }
            ss << it->dump();
        }
        ss << "]";

        return ss.str();
    }

    /**
     * @param type 1-slave 2-master
    */
    list<RedisInfo> MySentinel::getRedisByHash(const uint32_t &type, const string &hashStr) {
        map<string/*hash*/, map<uint32_t, RedisInfo> > allRedis = (2 == type) \
                ? this->getMasterRedis() : this->getSlaveRedis();

        list<RedisInfo> redisInfos;

        if (hashStr.empty()) {
            __foreach(it, allRedis) {
                redisInfos.push_back(it->second.begin()->second);
            }
            return redisInfos;
        }

        vector<string> allSlaves = Utils::splitStr(hashStr, "|");
        if (allSlaves.empty()) {
            allSlaves = Utils::splitStr(hashStr, ",");
        }

        __foreach(it, allSlaves) {
            if (allRedis.find(*it) != allRedis.end()) {
                // just use the first redis ip port
                redisInfos.push_back(allRedis[*it].begin()->second);
            }
        }

        return redisInfos;
    }


    uint32_t MySentinel::redisComHash(const string& key, const int32_t &redisNums) {
        if (0 >= redisNums) {
            LOG(Error, "illegal param", key, redisNums);
            return 0;
        }

        const char *keyPtr = key.c_str();
        long hashIndex = 5381;
        for (uint32_t index=0; index < key.length(); ++index) {
            hashIndex = ((hashIndex << 5) + hashIndex) + keyPtr[index];
            hashIndex = hashIndex & 0xFFFFFFFFl;
        }

        hashIndex = (int)hashIndex % redisNums;
        if (hashIndex < 0) {
            hashIndex = hashIndex * -1;
        }

        return hashIndex;
    }

    uint32_t MySentinel::redisCrc32Hash(const string &key, const int32_t &redisNum) {
        if (redisNum <= 0) {
            LOG(Error, "illegal param", key, redisNum);
            return 0;
        }
        return crc32(0L, (unsigned char *)key.c_str(), key.size()) % redisNum;
    }





    map<string/*hash*/, map<uint32_t, RedisInfo> > MySentinel::getSlaveRedis() {
        return this->m_slaveRedis;
    }

    void MySentinel::setSlaveRedis(const map<string/*hash*/, map<uint32_t, RedisInfo> > &info) {
        this->m_slaveRedis = info;
    }

    map<string/*hash*/, map<uint32_t, RedisInfo> > MySentinel::getMasterRedis() {
        return this->m_masterRedis;
    }

    void MySentinel::setMasterRedis(const map<string/*hash*/, map<uint32_t, RedisInfo> > &info) {
        this->m_masterRedis = info;
    }






}

