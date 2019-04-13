#ifndef MYERSDIFF_H
#define MYERSDIFF_H

#include "difftext.h"
#include "comparefunction.h"

#include <vector>

struct Point
{
    Point() : x(0), y(0) {}
    Point(int xIn, int yIn) : x(xIn), y(yIn) {}
    int x;
    int y;
};

struct Snake
{
    Snake() : start(), mid(), end() {}
    Point start;
    Point mid;
    Point end;
};

inline int index(int k)
{
    if (k >= 0)
    {
        return k;
    }
    else
    {
        return -k - 1;
    }
}

using VTable = std::vector<std::vector<int>>;

template <typename StringContainer>
VTable calculateVTable(const StringContainer &fromList, const StringContainer &toList, CompareFunction<typename StringContainer::value_type> compare)
{
    const int fromLen = fromList.size();
    const int toLen = toList.size();

    auto inBound = [&fromLen, &toLen](int fromIndex, int toIndex) -> bool
    {
        return fromIndex < fromLen && toIndex < toLen;
    };

    auto compResult = [&fromList, &toList, &compare](int fromIndex, int toIndex) -> CompareResult
    {
        return compare(fromList[fromIndex], toList[toIndex]);
    };

    VTable v;
    v.emplace_back(1);
    v[0][0] = 0;

    for (int i = 0; inBound(i, i) && compResult(i, i) != CompareResult::NotEqual; i++)
    {
        v[0][0]++;
    }

    if (fromLen == toLen && v[0][0] == fromLen)
    {
        return std::move(v);
    }

    for (int d = 1; d <= fromLen + toLen; d++)
    {
        v.emplace_back(d + 1);
        for (int k = -d; k <= d; k += 2)
        {
            bool fromUp = false;
            if (k == -d || (k != d && v[d - 1][index(k - 1)] < v[d - 1][index(k + 1)]))
            {
                fromUp = true;
            }

            Point start;
            Point mid;
            Point end;

            if (fromUp)
            {
                start.x = v[d - 1][index(k + 1)];
                start.y = start.x - (k + 1);
                mid.x = start.x;
                mid.y = mid.x - k;
            }
            else
            {
                start.x = v[d - 1][index(k - 1)];
                start.y = start.x - (k - 1);
                mid.x = start.x + 1;
                mid.y = mid.x - k;
            }
            end = mid;

            while (inBound(end.x, end.y)
                   && compResult(end.x, end.y) != CompareResult::NotEqual)
            {
                end.x++;
                end.y++;
            }
            v[d][index(k)] = end.x;

            if (end.x == fromLen && end.y == toLen)
            {
                return std::move(v);
            }
        }
    }
    assert(false);
    return std::move(v);
}

template <typename StringContainer>
std::vector<Snake> calculatePath(const VTable &v, const StringContainer &fromList, const StringContainer &toList)
{
    std::vector<Snake> result(v.size());

    int fromLen = fromList.size();
    int toLen = toList.size();

    Point last(fromLen, toLen);
    for (int d = static_cast<int>(v.size()) - 1; d > 0; d--)
    {
        Snake s;
        s.end = last;

        int k = last.x - last.y;
        bool fromUp = false;
        if (k == -d || (k != d && v[d - 1][index(k - 1)] < v[d - 1][index(k + 1)]))
        {
            fromUp = true;
        }

        if (fromUp)
        {
            s.start.x = v[d - 1][index(k + 1)];
            s.start.y = s.start.x - (k + 1);
            s.mid.x = s.start.x;
            s.mid.y = s.mid.x - k;
        }
        else
        {
            s.start.x = v[d - 1][index(k - 1)];
            s.start.y = s.start.x - (k - 1);
            s.mid.x = s.start.x + 1;
            s.mid.y = s.mid.x - k;
        }
        last = s.start;
        result[d] = s;
    }

    Snake first;
    first.end.x = v[0][index(0)];
    first.end.y = first.end.x;

    result[0] = first;

    return std::move(result);
}

template <typename StringContainer>
DiffText<typename StringContainer::value_type> convertPath2DiffText(const std::vector<Snake> &path, const StringContainer &fromList, const StringContainer &toList, CompareFunction<typename StringContainer::value_type> compare)
{
    using StringType = typename StringContainer::value_type;
    DiffText<StringType> result;

    int fromIndex = 0;
    int toIndex = 0;

    Snake first = path[0];
    int equalPrefix = first.end.x;
    for (int i = 0; i < equalPrefix; i++)
    {
        auto fromLine = std::make_shared<DiffLine<StringType>>(LineType::Equal);
        auto toLine = std::make_shared<DiffLine<StringType>>(LineType::Equal);

        if (compare(fromList[i], toList[i]) == CompareResult::Similar)
        {
            fromLine->setType(LineType::Replace);
            toLine->setType(LineType::Replace);
        }

        fromLine->setString(fromList[i]);
        toLine->setString(toList[i]);

        result.fromText.push_back(fromLine);
        result.toText.push_back(toLine);
    }
    fromIndex += equalPrefix;
    toIndex += equalPrefix;

    int length = path.size();
    for (int i = 1; i < length; i++)
    {
        Snake s = path[i];

        bool toRight = false;
        if (s.mid.x > s.start.x)
        {
            toRight = true;
        }

        auto changedFromLine = std::make_shared<DiffLine<StringType>>(LineType::Empty);
        auto changedToLine = std::make_shared<DiffLine<StringType>>(LineType::Empty);

        if (toRight)
        {
            changedFromLine->setType(LineType::Remove);
            changedFromLine->setString(fromList[fromIndex]);
            fromIndex++;
        }
        else
        {
            changedToLine->setType(LineType::Add);
            changedToLine->setString(toList[toIndex]);
            toIndex++;
        }

        result.fromText.push_back(changedFromLine);
        result.toText.push_back(changedToLine);

        int equalLen = s.end.x - s.mid.x;
        for (int i = 0; i < equalLen; i++)
        {
            auto fromLine = std::make_shared<DiffLine<StringType>>(LineType::Equal);
            auto toLine = std::make_shared<DiffLine<StringType>>(LineType::Equal);

            if (compare(fromList[fromIndex], toList[toIndex]) == CompareResult::Similar)
            {
                fromLine->setType(LineType::Replace);
                toLine->setType(LineType::Replace);
            }

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
DiffText<typename StringContainer::value_type> myersDiff(const StringContainer &fromList, const StringContainer &toList, CompareFunction<typename StringContainer::value_type> compare = defaultCompare<typename StringContainer::value_type>)
{
    auto v = calculateVTable(fromList, toList, compare);
    auto path = calculatePath(v, fromList, toList);
    return convertPath2DiffText(path, fromList, toList, compare);
}
#endif // MYERSDIFF_H
