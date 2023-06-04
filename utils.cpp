#include "utils.h"




namespace myRedisSentinel {



    uint32_t RedisSentinelUtils::atou32(const char *nptr)
    {
        return strtoul(nptr,0,10);
    }

    uint32_t RedisSentinelUtils::atoul(const char *nptr)
    {
        return strtoul(nptr,0,10);
    }

    uint64_t RedisSentinelUtils::atoull(const char *nptr)
    {
        return strtoull(nptr,0,10);
    }

    uint64_t RedisSentinelUtils::atou64(const char *nptr)
    {
        return strtoull(nptr,0,10);
    }

    int64_t RedisSentinelUtils::atoi64(const char *nptr)
    {
        return strtoll(nptr,0,10);
    }

    uint32_t RedisSentinelUtils::stringToU32(const string & str)
    {
        return atou32(str.c_str());
    }


    int64_t RedisSentinelUtils::stringToI64(const string & str)
    {
        return atoi64(str.c_str());
    }



    vector<string> RedisSentinelUtils::splitStr(const string & str,
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
    string RedisSentinelUtils::takeout_delims_str(const string & str,
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


    list<string> RedisSentinelUtils::domain2ip(const string &domain)
    {
        list<string> ipList;


        struct addrinfo hints, *res;
        int status;
        char ipstr[INET6_ADDRSTRLEN];

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
        hints.ai_socktype = SOCK_STREAM;

        if ((status = getaddrinfo(domain.c_str(), NULL, &hints, &res)) != 0) {
            std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << ", getaddrinfo: " << gai_strerror(status) << std::endl;
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
            //std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << ", Resolved " << domain << " to " << ipver << " address: " << ipstr << std::endl;
            ipList.push_back(ipstr);
        }

        freeaddrinfo(res);
        return ipList;
    }



}

