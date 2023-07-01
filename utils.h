#ifndef __SCOKET_REDIS_SENTINEL_UTILS_H__
#define __SCOKET_REDIS_SENTINEL_UTILS_H__


#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <netdb.h>
#include <stdarg.h>
#include <sys/time.h>
#include <zlib.h>
#include <vector>
#include <list>
#include <map>
#include <sstream>

#include "common.h"




using namespace std;

namespace scoketRedisSentinel {




    extern int syslogLevel;


    // 计算__VA_ARGS__参数个数辅助宏
    #define __ARG_N(											\
        _1, _2, _3, _4, _5, _6, _7, _8, _9,_10,					\
        _11,_12,_13,_14,_15,_16,_17,_18,_19,_20,				\
        _21,_22,_23,_24,_25,_26,_27,_28,_29,_30,				\
        _31,_32,_33,_34,_35,_36,_37,_38,_39,_40,				\
        _41,_42,_43,_44,_45,_46,_47,_48,_49,_50,				\
        _51,_52,_53,_54,_55,_56,_57,_58,_59,_60,				\
        _61,_62,_63,N,...) N

    #define __RSEQ_N()											\
        63,62,61,60,											\
        59,58,57,56,55,54,53,52,51,50,							\
        49,48,47,46,45,44,43,42,41,40,							\
        39,38,37,36,35,34,33,32,31,30,							\
        29,28,27,26,25,24,23,22,21,20,							\
        19,18,17,16,15,14,13,12,11,10,							\
        9,8,7,6,5,4,3,2,1,0

    // microsoft vc support
    #define __VC__(t) t

    #define __NARG_I(...) __VC__(__ARG_N(__VA_ARGS__))

    #define __NARG__(...) __NARG_I(__VA_ARGS__, __RSEQ_N())


    //格式化__VA_ARGS__辅助宏
    #define __VFUNC__(name, n) name##n

    #define __VFUNC(name, n) __VFUNC__(name, n)

    #define VFUNC(func, ...) __VC__(__VFUNC(func, __NARG__(__VA_ARGS__))) (__VA_ARGS__)

    #define FORMAT_ITEMS(SS, ...) VFUNC(FORMAT_ITEM, SS, ##__VA_ARGS__)

    #define FORMAT_ITEM1(SS)

    #define FORMAT_ITEM2(SS, ITEM)								\
        do {													\
            const char *_key = #ITEM;							\
            if (_key[0] != 0) {									\
                if(_key[0] == '"') {							\
                    std::string s = std::string(_key);          \
                    unsigned _len = s.length();				    \
                    SS << std::string(_key + 1, _len - 2);		\
                } else {										\
                    SS << _key << ":" << ITEM;					\
                }												\
                SS << " ";										\
            }													\
        } while (0)

    #define FORMAT_ITEM3(SS, _1, _2)							\
        FORMAT_ITEM2(SS, _1); 									\
        FORMAT_ITEM2(SS, _2)

    #define FORMAT_ITEM4(SS, _1, _2, _3)						\
        FORMAT_ITEM2(SS, _1);									\
        FORMAT_ITEM3(SS, _2, _3)

    #define FORMAT_ITEM5(SS, _1, _2, _3, _4)					\
        FORMAT_ITEM2(SS, _1);									\
        FORMAT_ITEM4(SS, _2, _3, _4)

    #define FORMAT_ITEM6(SS, _1, _2, _3, _4, _5)				\
        FORMAT_ITEM2(SS, _1);									\
        FORMAT_ITEM5(SS, _2, _3, _4, _5)

    #define FORMAT_ITEM7(SS, _1, _2, _3, _4, _5, _6)			\
        FORMAT_ITEM2(SS, _1); 									\
        FORMAT_ITEM6(SS, _2, _3, _4, _5, _6)

    #define FORMAT_ITEM8(SS, _1, _2, _3, _4, _5, _6, _7)		\
        FORMAT_ITEM2(SS, _1); 									\
        FORMAT_ITEM7(SS, _2, _3, _4, _5, _6, _7)

    #define FORMAT_ITEM9(SS, _1, _2, _3, _4, _5, _6, _7, _8)	\
        FORMAT_ITEM2(SS, _1); 									\
        FORMAT_ITEM8(SS, _2, _3, _4, _5, _6, _7, _8)


    #define FORMAT_ITEM10(SS, _1, _2, _3, _4, _5, _6, _7, _8, _9)	\
        FORMAT_ITEM2(SS, _1);										\
        FORMAT_ITEM9(SS, _2, _3, _4, _5, _6, _7, _8, _9)

    #define FORMAT_ITEM11(SS, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10)	\
        FORMAT_ITEM2(SS, _1); 											\
        FORMAT_ITEM10(SS, _2, _3, _4, _5, _6, _7, _8, _9, _10)

    #define FORMAT_ITEM12(SS, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11)	\
        FORMAT_ITEM2(SS, _1);												\
        FORMAT_ITEM11(SS, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11)

    #define FORMAT_ITEM13(SS, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12) \
        FORMAT_ITEM2(SS, _1);												 	 \
        FORMAT_ITEM12(SS, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12);

    #define FORMAT_ITEM14(SS, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13) \
        FORMAT_ITEM2(SS, _1);												  	  	  \
        FORMAT_ITEM13(SS, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13)

    #define FORMAT_ITEM15(SS, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14) \
        FORMAT_ITEM2(SS, _1);														   	   \
        FORMAT_ITEM14(SS, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14)

    #define FORMAT_ITEM16(SS, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15) \
        FORMAT_ITEM2(SS, _1);																	\
        FORMAT_ITEM15(SS, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15)

    #define FORMAT_ITEM17(SS, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16) \
        FORMAT_ITEM2(SS, _1);																	 	 \
        FORMAT_ITEM16(SS, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16)

    #define FORMAT_ITEM18(SS, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17) \
        FORMAT_ITEM2(SS, _1);																	 	 \
        FORMAT_ITEM17(SS, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17)

    #define __FORMAT_DEBUG_STRING(...) 						\
	std::stringstream _ss;									\
	do {						 							\
		FORMAT_ITEMS(_ss, ##__VA_ARGS__);					\
	} while (0)

    #define LOG(l, ...) 								\
	do {													\
		__FORMAT_DEBUG_STRING(__VA_ARGS__);					\
        int lv = l;                                         \
		RedisSentinelUtils::printLog(lv, __FILE__, __FUNCTION__, __LINE__, _ss.str().c_str());	\
	} while(0)



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

        // print list,vector,etc. of a map
        template<typename TListOfMap >
        static string printListOfMap(const TListOfMap & tc, const string &split = " ")
        {
            stringstream ss;
            ss << "{";
            typename TListOfMap::const_iterator it;
            for ( it = tc.begin(); it != tc.end(); ++it )
            {
                if ( it == tc.begin() )
                {
                    ss << printMap(*it);
                }
                else
                {
                    ss << ", " << printMap(*it);
                }
            }
            ss << "}";
            return ss.str();
        }

        template<typename T>
        static string toString( const T& from )
        {
            stringstream strstream;
            strstream <<  from;
            return strstream.str();
        }


        static list<string> domain2ip(const string &domain);

        static string getCurrentDateString();


        static void printLog(int lv, const char *file, const char *function, int line, const char *fmt, ...);


        static bool simpleCheckIpStr(const string &ip);




    };




}






#endif




