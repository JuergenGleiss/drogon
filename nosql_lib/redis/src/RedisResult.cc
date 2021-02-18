/**
 *
 *  @file RedisResult.cc
 *  @author An Tao
 *
 *  Copyright 2018, An Tao.  All rights reserved.
 *  https://github.com/an-tao/drogon
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Drogon
 *
 */

#include <drogon/nosql/RedisResult.h>
#include <hiredis/hiredis.h>

using namespace drogon::nosql;

std::string RedisResult::getStringForDisplaying() const noexcept
{
    return getStringForDisplayingWithIndent(0);
}

std::string RedisResult::getStringForDisplayingWithIndent(
    size_t indent) const noexcept
{
    switch (result_->type)
    {
        case REDIS_REPLY_STRING:
            return "\"" + std::string{result_->str, result_->len} + "\"";
            break;
        case REDIS_REPLY_STATUS:
            return std::string{result_->str, result_->len};
            break;
        case REDIS_REPLY_ERROR:
            return "'ERROR:" + std::string{result_->str, result_->len} + "'";
            break;
        case REDIS_REPLY_NIL:
            return "(nil)";
            break;
        case REDIS_REPLY_INTEGER:
            return std::to_string(result_->integer);
            break;
        case REDIS_REPLY_ARRAY:
        {
            std::string ret;
            for (size_t i = 0; i < result_->elements; ++i)
            {
                std::string lineNum = std::to_string(i + 1) + ") ";
                if (i > 0)
                {
                    ret += std::string(indent, ' ');
                }
                ret += lineNum;
                ret += RedisResult(result_->element[i])
                           .getStringForDisplayingWithIndent(lineNum.length());
                if (i != result_->elements - 1)
                {
                    ret += '\n';
                }
            }
            return ret;
        }
        break;
        default:
            return "*";
    }
}

std::string RedisResult::asString() const noexcept(false)
{
    auto rtype = type();
    if (rtype == RedisResultType::kString ||
        rtype == RedisResultType::kStatus || rtype == RedisResultType::kError)
    {
        return std::string(result_->str, result_->len);
    }
    else if (rtype == RedisResultType::kInteger)
    {
        return std::to_string(result_->integer);
    }
    else
    {
        throw std::runtime_error("type error");
    }
}

RedisResultType RedisResult::type() const noexcept
{
    switch (result_->type)
    {
        case REDIS_REPLY_STRING:
            return RedisResultType::kString;
            break;
        case REDIS_REPLY_ARRAY:
            return RedisResultType::kArray;
            break;
        case REDIS_REPLY_INTEGER:
            return RedisResultType::kInteger;
            break;
        case REDIS_REPLY_NIL:
            return RedisResultType::kNil;
            break;
        case REDIS_REPLY_STATUS:
            return RedisResultType::kStatus;
            break;
        case REDIS_REPLY_ERROR:
        default:
            return RedisResultType::kError;
            break;
    }
}

std::vector<RedisResult> RedisResult::asArray() const noexcept(false)
{
    auto rtype = type();
    if (rtype == RedisResultType::kArray)
    {
        std::vector<RedisResult> array;
        for (size_t i = 0; i < result_->elements; ++i)
        {
            array.emplace_back(result_->element[i]);
        }
        return array;
    }
    throw std::runtime_error("type error");
}
long long RedisResult::asInteger() const noexcept(false)
{
    if (type() == RedisResultType::kInteger)
        return result_->integer;
    throw std::runtime_error("type error");
}
bool RedisResult::isNil() const noexcept
{
    return type() == RedisResultType::kNil;
}