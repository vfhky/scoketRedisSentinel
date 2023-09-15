/**
 * Copyright (c) 2023-07, typecodes.com (vfhky@typecodes.com)
 *
 * All rights reserved.
 *
 * Simply imitate parsing the reply of hiRedis of RESP.
 */


#ifndef __SCOKET_REDIS_SENTINEL_HI_REDIS_RESP_UTILS_H__
#define __SCOKET_REDIS_SENTINEL_HI_REDIS_RESP_UTILS_H__




#include <iostream>
#include <vector>
#include <string>
#include <sstream>




namespace socketRedisSentinel {


    static const std::string HI_REDIS_RESP_SPLIT = "\r\n";
    static const size_t HI_REDIS_RESP_SPLIT_SIZE = 2;
    static const size_t HI_REDIS_RESP_FLAG_SIZE = 1;


    // hiRedisResp type
    enum HiRedisRespType {
        RespTypeInvalid         = 0,
        /**
         * flag of +/-
         */
        REDIS_REPLY_STRING      = 1,

        /**
         * flag of *
         */
        REDIS_REPLY_ARRAY       = 2,

        /**
         * flag of :
         */
        REDIS_REPLY_INTEGER     = 3,

        /**
         * flag of *
         */
        REDIS_REPLY_STATUS      = 5,

        REDIS_REPLY_ERROR       = 6,
    };

    // hiRedisResp struct
    struct HiRedisReplyData {
        HiRedisRespType type;
        std::string field;
        std::string value;
        std::vector<HiRedisReplyData> elements;

        HiRedisReplyData() {
            type = RespTypeInvalid;
            field.clear();
            value.clear();
            elements.clear();
        }

        const std::string dump() const {
            std::stringstream ss;
            ss << "HiRedisReplyData - {"
                << "[type:" << type << "]"
                << "[field:" << field << "]"
                << "[value:" << value << "]"
                << "}";
            return ss.str();
        }
    };


    class HiRedisRespUtils {

    public:

        /**
         * just read data derectly from hiRedis reply with flag +,-,:
         */
        static HiRedisReplyData processItem(const std::string &reply);

        /**
         * just read data with length from hiRedis reply with flag $.
         */
        static HiRedisReplyData processLineItem(const std::string &reply);

        /**
         * read array data from hiRedis reply with flag *.
         */
        static HiRedisReplyData processBulkItem(const std::string &reply, size_t pos);

        /**
         * the entrance of parsing data of hiRedis reply.
         */
        static HiRedisReplyData processMultiBulkItem(const std::string &reply, size_t pos);


    private:




    };



}







#endif
