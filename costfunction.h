#ifndef COSTFUNCTION_H
#define COSTFUNCTION_H

#include "lcslength.h"

#include <QDebug>
#include <functional>
#include <cmath>

constexpr float MIN_COST = 0;
constexpr float MAX_COST = 2.0;

template <typename StringType>
using CostFunction = std::function<float(const StringType &, const StringType &)>;

template <typename StringType>
float defaultCost(const StringType &from, const StringType &to)
{
    if (from == to)
    {
        return MIN_COST;
    }
    else
    {
        return MAX_COST;
    }
}

template <typename StringType>
float lcsCost(const StringType &from, const StringType &to)
{
    const int fromLen = from.size();
    const int toLen = to.size();

    if (fromLen == 0 && toLen == 0)
    {
        return MIN_COST;
    }

    const int lcsLen = lcsLength(from, to);
    const int totalLen = fromLen + toLen;
    const int diffLen = totalLen - lcsLen * 2;

    const float diffRatio = diffLen * 1.0 / totalLen;

    constexpr float MIN_THRESHOLD = 0.2;
    constexpr float MAX_THRESHOLD = 0.6;
    constexpr float GRADIENT = (MAX_COST - MIN_COST) / (MAX_THRESHOLD - MIN_THRESHOLD);

    if (diffRatio <= MIN_THRESHOLD)
    {
        return MIN_COST;
    }
    else if (diffRatio >= MAX_THRESHOLD)
    {
        return MAX_COST;
    }
    else
    {
        return (diffRatio - MIN_THRESHOLD) * GRADIENT;
    }
}
#endif // COSTFUNCTION_H
