#ifndef LCSLENGTH_H
#define LCSLENGTH_H

#include <vector>
#include <algorithm>
#include <assert.h>

// use edit distance algorithm to calculate lcs length
template <typename StringType>
int lcsLength(const StringType &from, const StringType &to)
{
    auto fromLen = from.size();
    auto toLen = to.size();

    if (from == to)
    {
        return fromLen;
    }
    if (fromLen == 0 || toLen == 0)
    {
        return 0;
    }

    std::vector<int> distance((fromLen + 1) * (toLen + 1));

    auto linearIndex = [fromLen, toLen](int i, int j) -> int
    {
        return i * (toLen + 1) + j;
    };

    for (auto i = 0; i <= fromLen; i++)
    {
        distance[linearIndex(i, 0)] = i;
    }
    for (auto j = 0; j <= toLen; j++)
    {
        distance[linearIndex(0, j)] = j;
    }

    for (auto i = 1; i <= fromLen; i++)
    {
        for (auto j = 1; j <= toLen; j++)
        {
            int ifAdd = distance[linearIndex(i, j - 1)] + 1;
            int ifRemove = distance[linearIndex(i - 1, j)] + 1;
            int ifReplace = distance[linearIndex(i - 1, j - 1)] + (from[i - 1] == to[j - 1] ? 0 : 1000);

            distance[linearIndex(i, j)] = std::min({ifAdd, ifRemove, ifReplace});
        }
    }

    int editDistance = distance[linearIndex(fromLen, toLen)];
    return (fromLen + toLen - editDistance) / 2;
}

#endif // LCSLENGTH_H
