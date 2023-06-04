#ifndef __MY_REDIS_SENTINEL_UTILS_H__
#define __MY_REDIS_SENTINEL_UTILS_H__


#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <netdb.h>
#include <vector>
#include <list>
#include <map>
#include <sstream>



using namespace std;

namespace myRedisSentinel {

    class RedisSentinelUtils {
    public:

        static uint32_t atou32(const char *nptr);

        static uint32_t atoul(const char *nptr);

        static uint64_t atoull(const char *nptr);

        static uint64_t atou64(const char *nptr);

        static int64_t atoi64(const char *nptr);

        static uint32_t stringToU32(const string & str);

        static int64_t stringToI64(const string & str);




        static vector<string> splitStr(const string & str,
                                const string & delims);

        static string takeout_delims_str(const string & str,
                                const string & delims);



        //for map of int/string.
        template<typename TTowDimensionTextContainer >
        static string printTowDimensionTextContainer(const TTowDimensionTextContainer & tc,
                string split = " ")
        {
            stringstream ss;
            ss << "[";
            typename TTowDimensionTextContainer::const_iterator it;
            uint32_t count = 1;
            for ( it = tc.begin(); it != tc.end(); it++ )
            {
                if ( count == tc.size() )
                {
                    ss << it->first << ":" << it->second;
                }
                else
                {
                    ss << it->first << ":" << it->second << split;
                }
                count++;
            }
            ss << "]";
            return ss.str();
        }

        template<typename TMap >
        static string printMap(const TMap & tc)
        {
            return printTowDimensionTextContainer(tc);
        }

        //only support vector/set/list of int/string.
        template<typename TOneDimensionTextContainer >
        static string printOneDimensionTextContainer(const TOneDimensionTextContainer & tc,
                string split = " ")
        {
            stringstream ss;
            ss << "(";
            typename TOneDimensionTextContainer::const_iterator it;
            uint32_t count = 1;
            for ( it = tc.begin(); it != tc.end(); it++ )
            {
                if ( count == tc.size() )
                {
                    ss << *it;
                }
                else
                {
                    ss << *it << split;
                }
                count++;
            }
            ss << ")";
            return ss.str();
        }

        template<typename TList >
        static string printList(const TList & tc)
        {
            return printOneDimensionTextContainer(tc);
        }

        template<typename T>
        static string toString( const T& from )
        {
            stringstream strstream;
            strstream <<  from;
            return strstream.str();
        }


        static list<string> domain2ip(const string &domain);

    };




}






#endif




