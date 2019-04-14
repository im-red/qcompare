#ifndef COMPAREFUNCTION_H
#define COMPAREFUNCTION_H

#include "lcslength.h"

enum class CompareResult
{
    NotEqual,
    Equal,
    Similar
};

template <typename StringType>
using CompareFunction = std::function<CompareResult(const StringType &, const StringType &)>;

template <typename StringType>
CompareResult defaultCompare(const StringType &left, const StringType &right)
{
    if (left == right)
    {
        return CompareResult::Equal;
    }
    else
    {
        return CompareResult::NotEqual;
    }
}

template <typename StringType>
CompareResult lcsCompare(const StringType &from, const StringType &to)
{
    const int lcs = lcsLength(from, to);
    const int fromLen = from.size();
    const int toLen = to.size();
    const int totalLen = fromLen + toLen;
    const int diffLen = totalLen - lcs * 2;

    constexpr double SIMILAR_THRESHOLD = 0.6;
    if (lcs == fromLen && lcs == toLen)
    {
        return CompareResult::Equal;
    }
    else if (fromLen == 0 || toLen == 0)
    {
        return CompareResult::NotEqual;
    }
    else
    {
        if (diffLen * 1.0 / totalLen <= SIMILAR_THRESHOLD)
        {
            return CompareResult::Similar;
        }
        else
        {
            return CompareResult::NotEqual;
        }
    }
}

template <typename StringType>
CompareResult affixCompare(const StringType &from, const StringType &to)
{
    const int fromLen = from.size();
    const int toLen = to.size();

    int prefixLen = 0;
    for (int i = 0; i < fromLen && i < toLen; i++)
    {
        if (from[i] == to[i])
        {
            prefixLen++;
        }
        else
        {
            break;
        }
    }

    int suffixLen = 0;
    for (int i = 0; i < fromLen && i < toLen; i++)
    {
        if (from[fromLen - 1 - i] == to[toLen - 1 - i])
        {
            suffixLen++;
        }
        else
        {
            break;
        }
    }

    const int affixLen = std::min({fromLen, toLen, prefixLen + suffixLen});
    const int totalLen = fromLen + toLen;
    const int diffLen = totalLen - affixLen * 2;
    constexpr double SIMILAR_THRESHOLD = 0.6;
    if (affixLen == fromLen && affixLen == toLen)
    {
        return CompareResult::Equal;
    }
    else if (fromLen == 0 || toLen == 0)
    {
        return CompareResult::NotEqual;
    }
    else
    {
        if (diffLen * 1.0 / totalLen <= SIMILAR_THRESHOLD)
        {
            return CompareResult::Similar;
        }
        else
        {
            return CompareResult::NotEqual;
        }
    }
}

#endif // COMPAREFUNCTION_H
