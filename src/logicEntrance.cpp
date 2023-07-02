#include "logicEntrance.h"





namespace socketRedisSentinel {




    LogicEntrance &LogicEntrance::instance() {
        static LogicEntrance instance;
        return instance;
    }


    string LogicEntrance::help() {
        stringstream ss;
        ss << "input format: reqType sentinelDomain/ip port bMaster [hash]" << "\n";
        ss << "===> @param reqType  :  1-get redis info.  2-modify log level." << "\n"
            << "===> @param sentinelDomain  :  sentinel doamin or ip" << "\n"
            << "===> @param port  :  sentinel port" << "\n"
            << "===> @param bMaster  :  1-means get slave redis info. "\
                    "2-means get master redis info 3-means the result equal : 1 | 2" << "\n"
            << "===> @param hash  :  not must. when you want to assign special"
                    " redis pool for the sentinelDomain." << "\n";

        ss << "===> example : " << "\n";
        ss << "=====> 1. top get all slave redis info, should input : "
            << "1 inner-sentinel.typecodes.com 3600 1" << "\n";
        ss << "=====> 2. top get special pool for slave redis info, should input : "
        << "1 inner-sentinel.typecodes.com 3600 1 test_001|test_002" << "\n";
        // ss << "=====> 3. to change log level, should input : " << "1 loglevel" << "\n";
        return ss.str();
    }


    string LogicEntrance::handleReq(const string &req) {
        vector<string> tokens = Utils::splitStr(req, " ");
        if (tokens.empty()) {
            return LogicEntrance::help();
        }

        uint32_t reqType = 0;
        try {
            reqType = Utils::stringToU32(tokens[0]);

            switch (reqType) {
                case CLIENT_REQ_TYPE_REDIS_INFO: {
                    return this->handleGetRedisInfoReq(tokens);
                }
                case CLIENT_REQ_TYPE_LOG_LEVEL: {
                    return this->handleResetLogLevelReq(tokens);
                }
                default: {
                    return LogicEntrance::help();
                }
            }
        } catch (const std::out_of_range& e) {
            LOG(Error, "out of range exception", tokens[0], e.what());
            return LogicEntrance::help();
        } catch (const std::invalid_argument& e) {
            LOG(Error, "invalid argument exception", tokens[0], e.what());
            return LogicEntrance::help();
        } catch (const std::exception& e) {
            LOG(Error, "exception", tokens[0], e.what());
            return LogicEntrance::help();
        } catch ( ... ) {
            LOG(Error, "uknown exception", tokens[0]);
            return LogicEntrance::help();
        }
    }

    string LogicEntrance::makeRspData(const CLIENT_REQ_REDIS_TYPE &type, const list<RedisInfo> &infos) {
        stringstream ss;
        if (CLIENT_REQ_REDIS_TYPE_MASTER & type) {
            ss << "==> master " << std::endl;
        } else {
            ss << "==> slave " << std::endl;
        }

        __foreach(it, infos) {
            ss << it->name << " " << it->ip << " " << it->port << std::endl;
        }

        return ss.str();
    }

    string LogicEntrance::handleGetRedisInfoReq(const vector<string> &tokens) {
        if (tokens.size() < 4) {
            return LogicEntrance::help();
        }

        try {
            string sentinelDomain = tokens[1];
            string port = tokens[2];
            uint32_t bMaster = Utils::stringToU32(tokens[3]);
            // hash : hash_001|hash_002|...
            string hash = "";
            if (tokens.size() > 4) {
                hash = tokens[4];
            }

            bool bIp = Utils::simpleCheckIpStr(sentinelDomain);
            string ip;
            if (!bIp) { // 传入域名
                list<string> ipList = Utils::domain2ip(sentinelDomain);
                if (ipList.empty()) {
                    return "invalid sentinel doamin";
                }
                ip = *ipList.begin();
            } else {    // 直接ip
                ip = sentinelDomain;
            }
            uint16_t portInt = atoi(port.c_str());

            // ======= begin main logic
            MySentinel &cmd = MySentinel::instance();
            if (!cmd.init(ip, portInt)) {
                LOG(Error, "init failed", ip, port);
                return "can not connect sentinel [" + ip + ":" + port + "]";
            }

            // set to memory
            stringstream rspData;
            if (bMaster & CLIENT_REQ_REDIS_TYPE_MASTER) {
                list<RedisInfo> master = cmd.getMaster();
                rspData << LogicEntrance::makeRspData(CLIENT_REQ_REDIS_TYPE_MASTER, master);
            }
            if (bMaster & CLIENT_REQ_REDIS_TYPE_SLVAVE) {
                list<RedisInfo> slave = cmd.pharseSlave();
                rspData << LogicEntrance::makeRspData(CLIENT_REQ_REDIS_TYPE_SLVAVE, slave);
            }

            if (hash.empty()) {
                LOG(Info, rspData.str());
                return rspData.str();
            }

            rspData.clear();
            rspData.str("");
            // master redis infos
            if (bMaster & CLIENT_REQ_REDIS_TYPE_MASTER) {
                list<RedisInfo> allMasterRedis = cmd.getRedisByHash(2, hash);
                rspData << LogicEntrance::makeRspData(CLIENT_REQ_REDIS_TYPE_MASTER, allMasterRedis);
            }

            // slave redis infos
            if (bMaster & CLIENT_REQ_REDIS_TYPE_SLVAVE) {
                list<RedisInfo> hashSlaveRedis = cmd.getRedisByHash(1, hash);
                rspData << LogicEntrance::makeRspData(CLIENT_REQ_REDIS_TYPE_SLVAVE, hashSlaveRedis);
            }

            LOG(Info, rspData.str());
            return rspData.str();
        } catch (const std::exception& e) {
            LOG(Error, "exception", Utils::printList(tokens), e.what());
            return LogicEntrance::help();
        } catch ( ... ) {
            LOG(Error, "uknown exception", Utils::printList(tokens));
            return LogicEntrance::help();
        }
    }

    string LogicEntrance::handleResetLogLevelReq(const vector<string> &tokens) {
        if (tokens.size() < 2) {
            return LogicEntrance::help();
        }

        try {
            stringstream rspData;
            LOG(Info, rspData.str());
            return rspData.str();
        } catch (const std::exception& e) {
            LOG(Error, "exception", Utils::printList(tokens), e.what());
            return LogicEntrance::help();
        } catch ( ... ) {
            LOG(Error, "uknown exception", Utils::printList(tokens));
            return LogicEntrance::help();
        }
    }

























}

