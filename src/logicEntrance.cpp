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
            << "===> @param redisType  :  1-get slave redis info. 2-get master redis info 3-both 1 and 2" << "\n"
            << "===> @param -poolName  :  not must. when you want to assign special"
                    " redis poolName for the sentinelDomain." << "\n"
            << "===> @param -hashKey  :  not must, used when type equals 2 to get target redis ip." << "\n";

        ss << "===> example : " << "\n";
        ss << "=====> 1. to get all slave redis info, should input : "
            << "1 inner-sentinel.typecodes.com 3600 1" << "\n";
        ss << "=====> 2. to get special poolName for slave redis info, should input : "
        << "1 inner-sentinel.typecodes.com 3600 2 test_001|test_002" << "\n";
        ss << "=====> 3. to get target master redis ip by common hash key: "
        << "2 inner-sentinel.typecodes.com 3600 1 -p test_001 -h key" << "\n";
        ss << "=====> 4. to change log level, should input : " << "3 loglevel" << "\n";
        return ss.str();
    }


    ClientReqInfo LogicEntrance::pharseReq(const string &req) {
        ClientReqInfo reqInfo;
        if (req.empty()) {
            LOG(Info, reqInfo.dump());
            return reqInfo;
        }

        /**
         * 0-begin parse
         * 1-get value
         * 2-get flag
        */
        int32_t flag = 0;

        int32_t length = req.size();
        stringstream ss;
        string key;
        for (int32_t index = 0; index < length; index++) {
            char c = (char)req[index];

            // begin to parse a flag
            if ('-' == c) {
                if (0 == index || ' ' == (char)req[index-1]) {
                    flag = 2;
                    continue;
                }
            }

            // settle when reach blank or come to the end char
            if (' ' == c || index == length - 1) {
                // reached end
                if (index == length - 1) {
                    ss << c;
                }

                // settle
                if (1 == flag || index == length - 1) {
                    string value = ss.str();
                    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                    if (key == "type") {
                        reqInfo.type = static_cast<CLIENT_REQ_TYPE>(Utils::stringToU32(value));
                    } else if (key == "ip") {
                        reqInfo.ip = value;
                    } else if (key == "port") {
                        reqInfo.port = (uint16_t)Utils::stringToU32(value);
                    } else if (key == "redistype") {
                        reqInfo.redisType = static_cast<CLIENT_REQ_REDIS_TYPE>(Utils::stringToU32(value));
                    } else if (key == "poolname") {
                        reqInfo.poolName = value;
                    } else if (key == "hashkey") {
                        reqInfo.hashKey = value;
                    } else if (key == "loglv") {
                        reqInfo.logLv = Utils::stringToI64(value);
                    } else if (key == "logtype") {
                        reqInfo.logType = Utils::stringToI64(value);
                    } else {
                        //
                    }

                    key.clear();
                    value.clear();
                    ss.clear();
                    ss.str("");
                    flag = 0;
                } else if (2 == flag) {
                    key = ss.str();
                    ss.clear();
                    ss.str("");
                    flag = 1;
                } else {
                }
                continue;
            }

            if (0 != flag) {
                ss << c;
            }
        }

        LOG(Info, reqInfo.dump());
        return reqInfo;
    }

    string LogicEntrance::handleReq(const string &req) {
        try {
            ClientReqInfo reqInfo = this->pharseReq(req);
            if (CLIENT_REQ_TYPE_ILLEGAL == reqInfo.type) {
                LOG(Info, "illegal req param", req, reqInfo.dump());
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
            LOG(Error, "out of range exception", req, e.what());
            return LogicEntrance::help();
        } catch (const std::invalid_argument& e) {
            LOG(Error, "invalid argument exception", req, e.what());
            return LogicEntrance::help();
        } catch (const std::exception& e) {
            LOG(Error, "exception", req, e.what());
            return LogicEntrance::help();
        } catch ( ... ) {
            LOG(Error, "uknown exception", req);
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
                rspData << LogicEntrance::makeRspData(CLIENT_REQ_REDIS_TYPE_MASTER, master);
            }
            if (req.redisType & CLIENT_REQ_REDIS_TYPE_SLVAVE) {
                list<RedisInfo> slave = cmd.pharseSlave();
                rspData << LogicEntrance::makeRspData(CLIENT_REQ_REDIS_TYPE_SLVAVE, slave);
            }

            if (req.poolName.empty()) {
                LOG(Info, rspData.str());
                return rspData.str();
            }

            rspData.clear();
            rspData.str("");
            // master redis infos
            if (req.redisType & CLIENT_REQ_REDIS_TYPE_MASTER) {
                list<RedisInfo> allMasterRedis = cmd.getRedisByHash(2, req.poolName);
                rspData << LogicEntrance::makeRspData(CLIENT_REQ_REDIS_TYPE_MASTER, allMasterRedis);
            }

            // slave redis infos
            if (req.redisType & CLIENT_REQ_REDIS_TYPE_SLVAVE) {
                list<RedisInfo> hashSlaveRedis = cmd.getRedisByHash(1, req.poolName);
                rspData << LogicEntrance::makeRspData(CLIENT_REQ_REDIS_TYPE_SLVAVE, hashSlaveRedis);
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
                LOG(Error, "init failed", ip, req.port);
                return ss.str();
            }

            // set to memory
            stringstream rspData;

            // master redis infos
            if (req.redisType & CLIENT_REQ_REDIS_TYPE_MASTER) {
                cmd.getMaster();
                list<RedisInfo> allMasterRedis = cmd.getRedisByHash(2, req.poolName);

                if (allMasterRedis.empty()) {
                    rspData << "# no master redis infos";
                    LOG(Info, "no redis infos", req.dump());
                } else {
                    uint32_t hashIndex = cmd.redisComHash(req.hashKey, allMasterRedis.size());
                    uint32_t index = 0;
                    __foreach(it, allMasterRedis) {
                        if (hashIndex == index) {
                            const RedisInfo &info = *it;
                            rspData << LogicEntrance::makeRspData(CLIENT_REQ_REDIS_TYPE_MASTER, info);
                            break;
                        }

                        ++index;
                    }
                }
            }

            // slave redis infos
            if (req.redisType & CLIENT_REQ_REDIS_TYPE_SLVAVE) {
                list<RedisInfo> slave = cmd.pharseSlave();
                list<RedisInfo> hashSlaveRedis = cmd.getRedisByHash(1, req.poolName);

                if (hashSlaveRedis.empty()) {
                    rspData << "# no master redis infos";
                    LOG(Info, "no redis infos", req.dump());
                } else {
                    uint32_t hashIndex = cmd.redisCrc32Hash(req.hashKey, hashSlaveRedis.size());
                    uint32_t index = 0;
                    __foreach(it, hashSlaveRedis) {
                        if (hashIndex == index) {
                            const RedisInfo &info = *it;
                            rspData << LogicEntrance::makeRspData(CLIENT_REQ_REDIS_TYPE_SLVAVE, info);
                            break;
                        }

                        ++index;
                    }
                }
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

