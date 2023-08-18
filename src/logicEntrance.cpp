#include "logicEntrance.h"





namespace socketRedisSentinel {




    LogicEntrance &LogicEntrance::instance() {
        static LogicEntrance instance;
        return instance;
    }


    string LogicEntrance::help() {
        stringstream ss;
        ss << "input format: -type 1 -ip a -port b -redisType c [-poolName d] [-hashKey e] [-logType f -logLv g]" << "\n";
        ss << "===> @param type :  1-get redis info.  2/3-get redis ip by common/crc32 hash  4-modify log type/lv." << "\n"
            << "===> @param ip  :  sentinel doamin or ip" << "\n"
            << "===> @param port  :  sentinel port" << "\n"
            << "===> @param redisType  :  1-get master redis info. 2-get slave redis info 3-both 1 and 2" << "\n"
            << "===> @param -poolName  :  not must. when you want to assign special"
                    " redis poolName for the sentinelDomain." << "\n"
            << "===> @param -hashKey  :  not must, used when type equals 2 to get target redis ip." << "\n"
            << "===> @param -logType  :  not must, used when type equals 3. enum: 1-console 2-log 3-both" << "\n"
            << "===> @param -logLv  :  not must, used when type equals 3. enum: 1-7." << "\n";

        ss << "===> example : " << "\n";
        ss << "=====> 1. to get all slave redis info, should input : "
            << "-type 1 -ip sentinel.typecodes.com -port 3600 -redisType 2" << "\n";
        ss << "=====> 2. to get special poolName for slave redis info, should input : "
            << "-type 1 -ip sentinel.typecodes.com -port 3600 -redisType 2 -poolName test_001|test_002" << "\n";
        ss << "=====> 3. to get target master redis ip by common hash key: "
            << "-type 2 -ip sentinel.typecodes.com -port 3600 -redisType 1 -poolName test_001 -hashKey key" << "\n";
        ss << "=====> 4. to change log type and level, should input : " << "-type 3 -logLv 1" << "\n";
        return ss.str();
    }


    string LogicEntrance::handleReq(ClientReqInfo &reqInfo) {
        try {
            if (CLIENT_REQ_TYPE_ILLEGAL == reqInfo.type) {
                LOG(Info, "illegal req param", reqInfo.dump());
                return LogicEntrance::help();
            }

            switch (reqInfo.type) {
                case CLIENT_REQ_TYPE_REDIS_INFO: {
                    return this->handleGetRedisInfoReq(reqInfo);
                }
                case CLIENT_REQ_TYPE_REDIS_INFO_BY_COM_HASH:
                case CLIENT_REQ_TYPE_REDIS_INFO_BY_CRC32_HASH: {
                    return this->handleRedisInfoByHashReq(reqInfo);
                }
                case CLIENT_REQ_TYPE_LOG_CFG: {
                    return this->handleResetLogLevelReq(reqInfo);
                }
                default: {
                    return LogicEntrance::help();
                }
            }
        } catch (const std::out_of_range& e) {
            LOG(Error, "out of range exception", reqInfo.dump(), e.what());
            return LogicEntrance::help();
        } catch (const std::invalid_argument& e) {
            LOG(Error, "invalid argument exception", reqInfo.dump(), e.what());
            return LogicEntrance::help();
        } catch (const std::exception& e) {
            LOG(Error, "exception", reqInfo.dump(), e.what());
            return LogicEntrance::help();
        } catch ( ... ) {
            LOG(Error, "uknown exception", reqInfo.dump());
            return LogicEntrance::help();
        }
    }

    string LogicEntrance::makeRspData(const CLIENT_REQ_REDIS_TYPE &type, const list<RedisInfo> &infos) {
        const string bMasterDesc = (CLIENT_REQ_REDIS_TYPE_MASTER & type) ? "master " : "slave ";
        stringstream ss;
        __foreach(it, infos) {
            ss << bMasterDesc << it->name << " " << it->ip << " " << it->port << std::endl;
        }

        return ss.str();
    }

    string LogicEntrance::makeRspData(const CLIENT_REQ_REDIS_TYPE &type, const RedisInfo &info) {
        const string bMasterDesc = (CLIENT_REQ_REDIS_TYPE_MASTER & type) ? "master " : "slave ";
        stringstream ss;
        ss << bMasterDesc << info.name << " " << info.ip << " " << info.port << std::endl;

        return ss.str();
    }

    /**
     * get master/slave redis info by sentinel.
    */
    string LogicEntrance::handleGetRedisInfoReq(const ClientReqInfo &req) {
        if (req.ip.empty() || -1 == req.port ||
            !(CLIENT_REQ_REDIS_TYPE_MASTER <= req.redisType && req.redisType <= CLIENT_REQ_REDIS_TYPE_ALL))
        {
            LOG(Error, "illegal req param", req.dump());
            return LogicEntrance::help();
        }

        try {
            bool bIp = Utils::simpleCheckIpStr(req.ip);
            string ip;
            if (!bIp) { // 传入域名
                list<string> ipList = Utils::domain2ip(req.ip);
                if (ipList.empty()) {
                    LOG(Info, "invalid sentinel doamin", req.dump());
                    return "invalid sentinel doamin";
                }
                ip = *ipList.begin();
            } else {    // 直接ip
                ip = req.ip;
            }

            // ======= begin main logic
            Sentinel &cmd = Sentinel::instance();
            if (!cmd.init(ip, req.port)) {
                stringstream ss;
                ss << "can not connect sentinel [" << ip << ":" << req.port << "]";
                LOG(Error, "init failed", ss.str());
                return ss.str();
            }

            // set to memory
            stringstream rspData;
            if (req.redisType & CLIENT_REQ_REDIS_TYPE_MASTER) {
                list<RedisInfo> master = cmd.getMaster();
                if (req.poolName.empty()) {
                    rspData << LogicEntrance::makeRspData(CLIENT_REQ_REDIS_TYPE_MASTER, master);
                }
            }
            if (req.redisType & CLIENT_REQ_REDIS_TYPE_SLAVE) {
                list<RedisInfo> slave = cmd.pharseSlave();
                if (req.poolName.empty()) {
                    rspData << LogicEntrance::makeRspData(CLIENT_REQ_REDIS_TYPE_SLAVE, slave);
                }
            }

            if (req.poolName.empty()) {
                LOG(Info, rspData.str());
                return rspData.str();
            }

            rspData.clear();
            rspData.str("");
            // master redis infos
            if (req.redisType & CLIENT_REQ_REDIS_TYPE_MASTER) {
                list<RedisInfo> allMasterRedis = cmd.getRedisByType(CLIENT_REQ_REDIS_TYPE_MASTER, req.poolName);
                rspData << LogicEntrance::makeRspData(CLIENT_REQ_REDIS_TYPE_MASTER, allMasterRedis);
            }

            // slave redis infos
            if (req.redisType & CLIENT_REQ_REDIS_TYPE_SLAVE) {
                list<RedisInfo> hashSlaveRedis = cmd.getRedisByType(CLIENT_REQ_REDIS_TYPE_SLAVE, req.poolName);
                rspData << LogicEntrance::makeRspData(CLIENT_REQ_REDIS_TYPE_SLAVE, hashSlaveRedis);
            }

            LOG(Info, rspData.str());
            return rspData.str();
        } catch (const std::exception& e) {
            LOG(Error, "exception", req.dump(), e.what());
            return LogicEntrance::help();
        } catch ( ... ) {
            LOG(Error, "uknown exception", req.dump());
            return LogicEntrance::help();
        }
    }

    /**
     * get target master/slave redis info by sentinel and hashkey.
    */
    string LogicEntrance::handleRedisInfoByHashReq(const ClientReqInfo &req) {
        if (req.ip.empty() || -1 == req.port ||
            !(CLIENT_REQ_REDIS_TYPE_MASTER <= req.redisType && req.redisType <= CLIENT_REQ_REDIS_TYPE_ALL)
            || req.hashKey.empty())
        {
            LOG(Error, "illegal req param", req.dump());
            return LogicEntrance::help();
        }

        try {
            bool bIp = Utils::simpleCheckIpStr(req.ip);
            string ip;
            if (!bIp) { // 传入域名
                list<string> ipList = Utils::domain2ip(req.ip);
                if (ipList.empty()) {
                    LOG(Info, "invalid sentinel doamin", req.dump());
                    return "invalid sentinel doamin";
                }
                ip = *ipList.begin();
            } else {    // 直接ip
                ip = req.ip;
            }

            // ======= begin main logic
            Sentinel &cmd = Sentinel::instance();
            if (!cmd.init(ip, req.port)) {
                stringstream ss;
                ss << "can not connect sentinel [" << ip << ":" << req.port << "]";
                LOG(Error, "init failed", ip, req.port, ss.str());
                return ss.str();
            }

            // set to memory
            stringstream rspData;

            // master redis infos
            list<RedisInfo> redisList;
            if (CLIENT_REQ_REDIS_TYPE_MASTER == req.redisType) {
                cmd.getMaster();
                redisList = cmd.getRedisByType(CLIENT_REQ_REDIS_TYPE_MASTER, req.poolName);
            } else if (CLIENT_REQ_REDIS_TYPE_SLAVE == req.redisType) {
                cmd.pharseSlave();
                redisList = cmd.getRedisByType(CLIENT_REQ_REDIS_TYPE_SLAVE, req.poolName);
            } else {
                rspData << "illegal req redisType=[" << req.redisType << "]";
                LOG(Warn, "illegal req redisType", rspData.str(), req.dump());
                return rspData.str();
            }

            if (redisList.empty()) {
                rspData << "# can not get redis infos";
                LOG(Error, "can not get redis infos", rspData.str(), req.dump());
                return rspData.str();
            }

            uint32_t hashIndex = 0;
            if (CLIENT_REQ_TYPE_REDIS_INFO_BY_COM_HASH == req.type) {
                hashIndex = cmd.redisComHash(req.hashKey, redisList.size());
            } else {
                hashIndex = cmd.redisCrc32Hash(req.hashKey, redisList.size());
            }

            uint32_t index = 0;
            __foreach(it, redisList) {
                if (hashIndex == index) {
                    rspData << LogicEntrance::makeRspData(req.redisType, *it);
                    break;
                }

                ++index;
            }

            LOG(Info, req.type, req.redisType, hashIndex, redisList.size(), rspData.str());
            return rspData.str();
        } catch (const std::exception& e) {
            LOG(Error, "exception", req.dump(), e.what());
            return LogicEntrance::help();
        } catch ( ... ) {
            LOG(Error, "uknown exception", req.dump());
            return LogicEntrance::help();
        }
    }

    string LogicEntrance::handleResetLogLevelReq(const ClientReqInfo &req) {
        if (-1 != req.logLv && !(Fatal <= req.logLv && req.logLv <= Debug) ) {
            LOG(Info, "req param illegal", req.dump());
            return LogicEntrance::help();
        }
        if (-1 != req.logType && !(LOG_TYPE_STDOUT <= req.logType && req.logType <= LOG_TYPE_ALL)) {
            LOG(Info, "req param illegal", req.dump());
            return LogicEntrance::help();
        }

        try {
            stringstream rspData;
            if (-1 != req.logLv) {
                Config::instance().setLogLv(static_cast<LOG_LEVEL>(req.logLv));
                rspData << "# set log level ok " << req.logLv << std::endl;
            } else {
                LOG_LEVEL lv = Config::instance().getLogLv();
                rspData << "# not set log level , so keep old lv ==>" << lv << std::endl;
            }
            if (-1 != req.logType) {
                Config::instance().setLogType(static_cast<LOG_TYPE>(req.logType));
                rspData << "# set log type ok " << req.logType << std::endl;
            } else {
                LOG_TYPE type = Config::instance().getLogType();
                rspData << "# not set log type , so keep old type ==>" << type << std::endl;
            }

            LOG(Info, rspData.str());
            return rspData.str();
        } catch (const std::exception& e) {
            LOG(Error, "exception", req.dump(), e.what());
            return LogicEntrance::help();
        } catch ( ... ) {
            LOG(Error, "uknown exception", req.dump());
            return LogicEntrance::help();
        }
    }

























}

