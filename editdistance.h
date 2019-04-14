#ifndef EDITDISTANCE_H
#define EDITDISTANCE_H

#include "costfunction.h"
#include "difftext.h"

#include <vector>

enum class EditOperation
{
    Copy,
    Add,
    Remove,
    Replace,
    Invalid
};

constexpr float ADD_COST = 1.0;
constexpr float REMOVE_COST = 1.0;

template <typename StringContainer>
std::vector<EditOperation> editOperations(const StringContainer &fromList, const StringContainer &toList, CostFunction<typename StringContainer::value_type> cost = defaultCost<typename StringContainer::value_type>)
{
    const int fromLen = fromList.size();
    const int toLen = toList.size();

    std::vector<float> distance((fromLen + 1) * (toLen + 1));
    std::vector<EditOperation> operations((fromLen + 1) * (toLen + 1));

    auto index = [fromLen, toLen](int i, int j)
    {
        return i * (toLen + 1) + j;
    };

    for (int i = 0; i <= fromLen; i++)
    {
        distance[index(i, 0)] = i;
        operations[index(i, 0)] = EditOperation::Remove;
    }
    for (int j = 0; j <= toLen; j++)
    {
        distance[index(0, j)] = j;
        operations[index(0, j)] = EditOperation::Add;
    }
    operations[0] = EditOperation::Copy;

    for (int i = 1; i <= fromLen; i++)
    {
        for (int j = 1; j <= toLen; j++)
        {
            float ifAdd = distance[index(i, j - 1)] + ADD_COST;
            float ifRemove = distance[index(i - 1, j)] + REMOVE_COST;

            bool needReplace = fromList[i - 1] != toList[j - 1];
            float ifReplace = distance[index(i - 1, j - 1)] + cost(fromList[i - 1], toList[j - 1]);

            if (ifAdd <= ifRemove && ifAdd <= ifReplace)
            {
                distance[index(i, j)] = ifAdd;
                operations[index(i, j)] = EditOperation::Add;
            }
            else if (ifRemove <= ifAdd && ifRemove <= ifReplace)
            {
                distance[index(i, j)] = ifRemove;
                operations[index(i, j)] = EditOperation::Remove;
            }
            else
            {
                distance[index(i, j)] = ifReplace;
                if (needReplace)
                {
                    operations[index(i, j)] = EditOperation::Replace;
                }
                else
                {
                    operations[index(i, j)] = EditOperation::Copy;
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
        if (op == EditOperation::Add)
        {
            //qDebug() << distance[index(i, j)] - distance[index(i, j - 1)];
            j--;
        }
        else if (op == EditOperation::Remove)
        {
            //qDebug() << distance[index(i, j)] - distance[index(i - 1, j)];
            i--;
        }
        else
        {
            //qDebug() << distance[index(i, j)] - distance[index(i - 1, j - 1)];
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
    return std::move(result);
}

template <typename StringContainer>
DiffText<typename StringContainer::value_type> convertEditOperations2DiffText(const std::vector<EditOperation> &ops, const StringContainer &fromList, const StringContainer &toList)
{
    using StringType = typename StringContainer::value_type;
    DiffText<StringType> result;

    int fromIndex = 0;
    int toIndex = 0;

    for (auto op : ops)
    {
        if (op == EditOperation::Copy)
        {
            auto fromLine = std::make_shared<DiffLine<StringType>>(LineType::Equal);
            auto toLine = std::make_shared<DiffLine<StringType>>(LineType::Equal);

            fromLine->setString(fromList[fromIndex]);
            toLine->setString(toList[toIndex]);

            fromIndex++;
            toIndex++;

            result.fromText.push_back(fromLine);
            result.toText.push_back(toLine);
        }
        else if (op == EditOperation::Add)
        {
            auto fromLine = std::make_shared<DiffLine<StringType>>(LineType::Empty);
            auto toLine = std::make_shared<DiffLine<StringType>>(LineType::Add);

            toLine->setString(toList[toIndex]);

            toIndex++;

            result.fromText.push_back(fromLine);
            result.toText.push_back(toLine);
        }
        else if (op == EditOperation::Remove)
        {
            auto fromLine = std::make_shared<DiffLine<StringType>>(LineType::Remove);
            auto toLine = std::make_shared<DiffLine<StringType>>(LineType::Empty);

            fromLine->setString(fromList[fromIndex]);

            fromIndex++;

            result.fromText.push_back(fromLine);
            result.toText.push_back(toLine);
        }
        else // if Replace
        {
            auto fromLine = std::make_shared<DiffLine<StringType>>(LineType::Replace);
            auto toLine = std::make_shared<DiffLine<StringType>>(LineType::Replace);

            fromLine->setString(fromList[fromIndex]);
            toLine->setString(toList[toIndex]);

            fromIndex++;
            toIndex++;

            result.fromText.push_back(fromLine);
            result.toText.push_back(toLine);
        }
    }
    return std::move(result);
}

template <typename StringContainer>
DiffText<typename StringContainer::value_type> editDistance(const StringContainer &fromList, const StringContainer &toList, CostFunction<typename StringContainer::value_type> cost = defaultCost<typename StringContainer::value_type>)
{
    auto ops = editOperations(fromList, toList, cost);
    return convertEditOperations2DiffText(ops, fromList, toList);
}

#endif // EDITDISTANCE_H
