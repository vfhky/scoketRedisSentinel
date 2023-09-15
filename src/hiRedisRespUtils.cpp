/**
 * Copyright (c) 2023-07, typecodes.com (vfhky@typecodes.com)
 *
 * All rights reserved.
 *
 * Simply imitate parsing the reply of hiRedis of RESP.
 */


#include "hiRedisRespUtils.h"
#include "hiRedisException.h"
#include "utils.h"




namespace socketRedisSentinel {

    /**
     * just read data derectly from hiRedis reply with flag +,-,:
     */
    HiRedisReplyData HiRedisRespUtils::processLineItem(const std::string &reply) {
        HiRedisReplyData result;
        result.type = REDIS_REPLY_STRING;

        size_t pos = 1;
        size_t endPos = reply.find(HI_REDIS_RESP_SPLIT.c_str());
        if (endPos != std::string::npos) {
            result.value = reply.substr(pos, endPos - pos);
        } else {
            throw HiRedisException("not enough data");
        }

        return result;
    }

    /**
     * just read data with length from hiRedis reply with flag $.
     */
    HiRedisReplyData HiRedisRespUtils::processBulkItem(const std::string &reply, size_t pos) {
        HiRedisReplyData result;
        result.type = REDIS_REPLY_STRING;

        size_t endPos = reply.find(HI_REDIS_RESP_SPLIT.c_str(), pos);
        if (endPos != std::string::npos) {
            result.field = reply.substr(pos, endPos - pos);
            size_t needReadLength = Utils::stringToI64(result.field) + HI_REDIS_RESP_SPLIT_SIZE;

            endPos = pos + result.field.size() + HI_REDIS_RESP_SPLIT_SIZE;
            LOG(Debug, needReadLength, pos, endPos, result.field.size(), reply.size());
            if (reply.size() >= (endPos + needReadLength)) {
                result.value = reply.substr(endPos, needReadLength);
                LOG(Debug, needReadLength, pos, endPos, result.value);
            } else {
                throw HiRedisException("not enough data");
            }
        } else {
            throw HiRedisException("not enough data");
        }

        return result;
    }

    /**
     * read array data from hiRedis reply with flag *.
     */
    HiRedisReplyData HiRedisRespUtils::processMultiBulkItem(const std::string &reply, size_t pos) {
        HiRedisReplyData result;
        result.type = REDIS_REPLY_ARRAY;

        size_t endPos = reply.find(HI_REDIS_RESP_SPLIT.c_str(), pos);
        if (endPos != std::string::npos) {
            result.field = reply.substr(pos, endPos - pos);
            pos = endPos + HI_REDIS_RESP_SPLIT_SIZE;
            int64_t numElements = Utils::stringToI64(result.field);
            for (int i = 0; i < numElements; i++) {
                HiRedisReplyData element = HiRedisRespUtils::processItem(reply.substr(pos));
                result.elements.push_back(element);
                pos += HI_REDIS_RESP_FLAG_SIZE + element.field.size() + element.value.size() + HI_REDIS_RESP_SPLIT_SIZE;
            }
        }

        return result;
    }

    /**
     * the entrance of parsing data of hiRedis reply.
     */
    HiRedisReplyData HiRedisRespUtils::processItem(const std::string& reply) {
        LOG(Debug, "begin", reply);

        size_t pos = 0;
        if (reply[pos] == '+') {
            HiRedisReplyData hiRedisReplyData;
            hiRedisReplyData.type = REDIS_REPLY_STATUS;
            pos++;
            return HiRedisRespUtils::processLineItem(reply);
        } else if (reply[pos] == '-') {
            pos++;
            HiRedisReplyData hiRedisReplyData;
            hiRedisReplyData.type = REDIS_REPLY_ERROR;
            return HiRedisRespUtils::processLineItem(reply);
        } else if (reply[pos] == ':') {
            pos++;
            HiRedisReplyData hiRedisReplyData;
            hiRedisReplyData.type = REDIS_REPLY_INTEGER;
            return HiRedisRespUtils::processLineItem(reply);
        } else if (reply[pos] == '$') {
            pos++;
            return HiRedisRespUtils::processBulkItem(reply, pos);
        } else if (reply[pos] == '*') {
            pos++;
            return HiRedisRespUtils::processMultiBulkItem(reply, pos);
        } else {
            throw HiRedisException("not enough data");
        }
    }




}
