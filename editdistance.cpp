#include "editdistance.h"

#include <QStringList>
#include <QDebug>
#include <vector>
#include <iostream>

int stringEditDistance(const QString &from, const QString &to)
{
    auto fromLen = from.size();
    auto toLen = to.size();

    std::vector<int> distance((fromLen + 1) * (toLen + 1));

    auto index = [fromLen, toLen](auto i, auto j)
    {
        return i * (toLen + 1) + j;
    };

    for (auto i = 0; i <= fromLen; i++)
    {
        distance[index(i, 0)] = i;
    }
    for (auto j = 0; j <= toLen; j++)
    {
        distance[index(0, j)] = j;
    }

    for (auto i = 1; i <= fromLen; i++)
    {
        for (auto j = 1; j <= toLen; j++)
        {
            int ifAdd = distance[index(i, j - 1)] + 1;
            int ifRemove = distance[index(i - 1, j)] + 1;
            int ifReplace = distance[index(i - 1, j - 1)] + (from[i - 1] == to[j - 1] ? 0 : 1);

            distance[index(i, j)] = std::min({ifAdd, ifRemove, ifReplace});
        }
    }

    return distance[index(fromLen, toLen)];
}

std::vector<EditOperation> articleEditOperations(const QString &from, const QString &to)
{
    const static QChar DELIMITER = '\n';

    const QStringList fromList = from.split(DELIMITER);
    const QStringList toList = to.split(DELIMITER);

    return articleEditOperations(fromList, toList);
}

std::vector<EditOperation> articleEditOperations(const QStringList &fromList, const QStringList &toList)
{
    const int fromLen = fromList.size();
    const int toLen = toList.size();

    std::vector<int> distance((fromLen + 1) * (toLen + 1));
    std::vector<EditOperation> operations((fromLen + 1) * (toLen + 1));

    auto index = [fromLen, toLen](int i, int j)
    {
        return i * (toLen + 1) + j;
    };

    for (int i = 0; i <= fromLen; i++)
    {
        distance[index(i, 0)] = i;
        operations[index(i, 0)] = Remove;
    }
    for (int j = 0; j <= toLen; j++)
    {
        distance[index(0, j)] = j;
        operations[index(0, j)] = Add;
    }
    operations[0] = Nop;

    for (int i = 1; i <= fromLen; i++)
    {
        for (int j = 1; j <= toLen; j++)
        {
            int ifAdd = distance[index(i, j - 1)] + 1;
            int ifRemove = distance[index(i - 1, j)] + 1;

            bool needReplace = fromList[i - 1] != toList[j - 1];
            int ifReplace = distance[index(i - 1, j - 1)] + (needReplace ? 1 : 0);

            if (ifAdd <= ifRemove && ifAdd <= ifReplace)
            {
                distance[index(i, j)] = ifAdd;
                operations[index(i, j)] = Add;
            }
            else if (ifRemove <= ifAdd && ifRemove <= ifReplace)
            {
                distance[index(i, j)] = ifRemove;
                operations[index(i, j)] = Remove;
            }
            else
            {
                distance[index(i, j)] = ifReplace;
                if (needReplace)
                {
                    operations[index(i, j)] = Replace;
                }
                else
                {
                    operations[index(i, j)] = Nop;
                }
            }
        }
    }

    std::vector<EditOperation> rresult;

    int i = fromLen;
    int j = toLen;

    while (!(i == 0 && j == 0))
    {
        EditOperation op = operations[index(i, j)];
        rresult.push_back(op);
        if (op == Add)
        {
            j--;
        }
        else if (op == Remove)
        {
            i--;
        }
        else
        {
            i--;
            j--;
        }
    }

    int opLen = static_cast<int>(rresult.size());
    std::vector<EditOperation> result(opLen);
    for (int k = 0; k < opLen; k++)
    {
        result[k] = rresult[opLen - 1 - k];
    }
    return result;
}
