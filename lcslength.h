#ifndef LCSLENGTH_H
#define LCSLENGTH_H

#include <vector>
#include <algorithm>
#include <memory>
#include <assert.h>

static inline int linearIndex(int i, int j, int toLen)
{
    return i * (toLen + 1) + j;
}

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

    const static int RESERVED = 100 * 100;
    static thread_local std::vector<int> distance(RESERVED);
    distance.reserve((fromLen + 1) * (toLen + 1));

    for (auto i = 0; i <= fromLen; i++)
    {
        distance[linearIndex(i, 0, toLen)] = i;
    }
    for (auto j = 0; j <= toLen; j++)
    {
        distance[linearIndex(0, j, toLen)] = j;
    }

    for (auto i = 1; i <= fromLen; i++)
    {
        for (auto j = 1; j <= toLen; j++)
        {
            int ifAdd = distance[linearIndex(i, j - 1, toLen)] + 1;
            int ifRemove = distance[linearIndex(i - 1, j, toLen)] + 1;
            int ifReplace = distance[linearIndex(i - 1, j - 1, toLen)] + (from[i - 1] == to[j - 1] ? 0 : 2);

            if (ifAdd <= ifRemove && ifAdd <= ifReplace)
            {
                distance[linearIndex(i, j, toLen)] = ifAdd;
            }
            else if (ifRemove <= ifAdd && ifRemove <= ifReplace)
            {
                distance[linearIndex(i, j, toLen)] = ifRemove;
            }
            else
            {
                distance[linearIndex(i, j, toLen)] = ifReplace;
            }
        }
    }

    int editDistance = distance[linearIndex(fromLen, toLen, toLen)];
    return (fromLen + toLen - editDistance) / 2;
}

#endif // LCSLENGTH_H
