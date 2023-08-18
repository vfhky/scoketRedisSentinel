#include "utils.h"




namespace socketRedisSentinel {




    uint32_t Utils::atou32(const char *nptr)
    {
        return strtoul(nptr,0,10);
    }

    uint32_t Utils::atoul(const char *nptr)
    {
        return strtoul(nptr,0,10);
    }

    uint64_t Utils::atoull(const char *nptr)
    {
        return strtoull(nptr,0,10);
    }

    uint64_t Utils::atou64(const char *nptr)
    {
        return strtoull(nptr,0,10);
    }

    int64_t Utils::atoi64(const char *nptr)
    {
        return strtoll(nptr,0,10);
    }

    uint32_t Utils::stringToU32(const string & str)
    {
        return atou32(str.c_str());
    }


    int64_t Utils::stringToI64(const string & str)
    {
        return atoi64(str.c_str());
    }



    vector<string> Utils::splitStr(const string & str,
                                const string & delims)
    {
    // Skip delims at beginning, find start of first token
    string::size_type lastPos = str.find_first_not_of(delims, 0);
    // Find next delimiter @ end of token
    string::size_type pos     = str.find_first_of(delims, lastPos);

    // output vector
    vector<string> tokens;

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delims.  Note the "not_of". this is beginning of token
        lastPos = str.find_first_not_of(delims, pos);
        // Find next delimiter at end of token.
        pos     = str.find_first_of(delims, lastPos);
    }

    return tokens;
    }

    //take out the delim  from string
    string Utils::takeout_delims_str(const string & str,
                                const string & delims)
    {
        // Skip delims at beginning, find start of first token
        string::size_type lastPos = str.find_first_not_of(delims, 0);
        // Find next delimiter @ end of token
        string::size_type pos     = str.find_first_of(delims, lastPos);

        // output vector
        string takeout_str = "";

        while (string::npos != pos || string::npos != lastPos)
        {
            // Found a string between two delims
            takeout_str += str.substr(lastPos, pos - lastPos);
            // Skip delims.  Note the "not_of". this is beginning of token
            lastPos = str.find_first_not_of(delims, pos);
            // Find next delimiter at end of takeout
            pos     = str.find_first_of(delims, lastPos);
        }

        return takeout_str;
    }


    list<string> Utils::domain2ip(const string &domain)
    {
        list<string> ipList;


        struct addrinfo hints, *res;
        int status;
        char ipstr[INET6_ADDRSTRLEN];

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
        hints.ai_socktype = SOCK_STREAM;

        if ((status = getaddrinfo(domain.c_str(), NULL, &hints, &res)) != 0) {
            LOG(Debug, status, gai_strerror(status));
            return ipList;
        }

        for (struct addrinfo *p = res; p != NULL; p = p->ai_next) {
            void *addr;
            const char *ipver;

            if (p->ai_family == AF_INET) {
                struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
                addr = &(ipv4->sin_addr);
                ipver = "IPv4";
            } else {
                struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
                addr = &(ipv6->sin6_addr);
                ipver = "IPv6";
            }

            inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
            ipList.push_back(ipstr);
            LOG(Debug, domain, ipver, ipstr, ipList.size());
        }

        freeaddrinfo(res);
        return ipList;
    }

    string Utils::getCurrentDateString() {
        time_t now;
        time(&now);
        struct tm tm_current = {0};
        localtime_r(&now, &tm_current);

        char dateStr[28] = {0};
        snprintf(dateStr, sizeof(dateStr), "%04d-%02d-%02d %02d:%02d:%02d", tm_current.tm_year + 1900, tm_current.tm_mon + 1, tm_current.tm_mday, tm_current.tm_hour, tm_current.tm_min, tm_current.tm_sec);
        string rtnString;
        rtnString = dateStr;
        return rtnString;
    }



    void Utils::printLog(LOG_LEVEL lv, const char *file, const char *function, int line, const char *fmt, ...) {
        static const int MAXLOGLINE = 4096;

        LOG_LEVEL syslogLevel = Config::getLogLv();
        if (syslogLevel && (lv <= syslogLevel)) {
            char msg[MAXLOGLINE];

            va_list		ap;
            va_start(ap, fmt);
            int ret = vsnprintf( msg, MAXLOGLINE, fmt, ap );
            va_end(ap);

            if (ret < 0)
                return;

            struct timeval tv;
            ::gettimeofday(&tv,NULL);

            // 2023-06-05 11:11:11
            char buf[64] = {0x00};
            struct tm *t = ::localtime(&tv.tv_sec);
            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", t);

            stringstream ss;
            const char *fileNoPath = fileName(file);
            ss << "[" << buf << ":" << tv.tv_usec << " us][" << fileNoPath << ":" \
                    << line << "][" << getpid() << "] ";
            if (lv <= Error) {
                ss << " ***err*** ";
            }

            ss << msg;
            if (LOG_TYPE_STDOUT & Config::getLogType()) {
                std::cout << ss.str() << std::endl;
            }
            if (LOG_TYPE_FILE & Config::getLogType()) {
                Logger::logToFile(ss.str());
            }
        }
    }

    bool Utils::simpleCheckIpStr(const string &ip) {
        if (ip.empty()) {
            return false;
        }

        for (uint32_t index = 0; index < ip.length(); index++) {
            char c = ip[index];
            if ((0x30 <= c && c <= 0x39) || '.' == c) {
                continue;
            } else {
                return false;
            }
        }

        return true;
    }

    // remove first and last character from string
    std::string Utils::removeFirstAndLastDoubleQuotes(const std::string& str) {
        std::string strTmp = str;
        if (!strTmp.empty() && strTmp[0] == '"') {
            strTmp = strTmp.substr(1);
        }
        if (!strTmp.empty() && strTmp[strTmp.length() - 1] == '"') {
            strTmp = strTmp.substr(0, strTmp.length() - 1);
        }
        return strTmp;
    }

    /**
     * pharse very simple json to map other than import third library.
     *
     */
    std::map<std::string, std::string> Utils::simpleJsonToMap(const std::string& json) {
        std::map<std::string, std::string> jsonData;

        size_t pos = 0;

        int32_t maxRecycle = 100;
        int32_t recycle = 1;

        try {
            while (recycle < maxRecycle && pos < json.length()) {
                size_t keyStart = json.find('"', pos);
                if (keyStart == std::string::npos)
                    break;

                size_t keyEnd = json.find('"', keyStart + 1);
                if (keyEnd == std::string::npos)
                    break;

                std::string key = json.substr(keyStart + 1, keyEnd - keyStart - 1);

                size_t valueStart = json.find(':', keyEnd);
                if (valueStart == std::string::npos)
                    break;

                size_t valueEnd = json.find(',', valueStart);
                if (valueEnd == std::string::npos)
                    valueEnd = json.find('}', valueStart);

                std::string value = json.substr(valueStart + 1, valueEnd - valueStart - 1);
                value = Utils::removeFirstAndLastDoubleQuotes(value);

                jsonData[key] = value;

                pos = valueEnd + 1;

                ++recycle;
            }
        } catch (const std::exception& e) {
            jsonData.clear();
            return jsonData;
        } catch ( ... ) {
            jsonData.clear();
            return jsonData;
        }

        return jsonData;
    }



}

