#include "myersdiff.h"
#include "diffline.h"
#include "difftext.h"

#include <vector>
#include <map>

using namespace std;

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

static std::vector<std::vector<int>> calculateV(const QStringList &fromList, const QStringList &toList)
{
    int fromLen = fromList.size();
    int toLen = toList.size();

    std::vector<std::vector<int>> v;
    v.emplace_back(1);
    v[0][0] = 0;

    for (int i = 0; i < fromLen && i < toLen && fromList[i] == toList[i]; i++)
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

            while (end.x < fromLen && end.y < toLen
                   && fromList[end.x - 1 + 1] == toList[end.y - 1 + 1])
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

static std::vector<Snake> calculatePath(const std::vector<std::vector<int>> &v, const QStringList &fromList, const QStringList &toList)
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

static DiffText convertPath2DiffText(const std::vector<Snake> &path, const QStringList &fromList, const QStringList &toList)
{
    DiffText result;

    int fromIndex = 0;
    int toIndex = 0;

    Snake first = path[0];
    int equalPrefix = first.end.x;
    for (int i = 0; i < equalPrefix; i++)
    {
        auto fromLine = std::make_shared<DiffLine>(DiffLine::Equal);
        auto toLine = std::make_shared<DiffLine>(DiffLine::Equal);
        fromLine->setString(fromList[i]);
        toLine->setString(toList[i]);

        result.fromText.push_back(fromLine);
        result.toText.push_back(toLine);
    }
    fromIndex += equalPrefix;
    toIndex += equalPrefix;

    for (int i = 1; i < path.size(); i++)
    {
        Snake s = path[i];

        bool toRight = false;
        if (s.mid.x > s.start.x)
        {
            toRight = true;
        }

        auto changedFromLine = std::make_shared<DiffLine>(DiffLine::Empty);
        auto changedToLine = std::make_shared<DiffLine>(DiffLine::Empty);

        if (toRight)
        {
            changedFromLine->setType(DiffLine::Remove);
            changedFromLine->setString(fromList[fromIndex]);
            fromIndex++;
        }
        else
        {
            changedToLine->setType(DiffLine::Add);
            changedToLine->setString(toList[toIndex]);
            toIndex++;
        }

        result.fromText.push_back(changedFromLine);
        result.toText.push_back(changedToLine);

        int equalLen = s.end.x - s.mid.x;
        for (int i = 0; i < equalLen; i++)
        {
            auto fromLine = std::make_shared<DiffLine>(DiffLine::Equal);
            auto toLine = std::make_shared<DiffLine>(DiffLine::Equal);
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

DiffText myersDiff(const QStringList &fromList, const QStringList &toList)
{
    auto v = calculateV(fromList, toList);
    auto path = calculatePath(v, fromList, toList);
//    for (auto &s : path)
//    {
//        printf("(%d, %d)->(%d, %d)->(%d, %d)\n", s.start.x, s.start.y, s.mid.x, s.mid.y, s.end.x, s.end.y);
//    }
    return convertPath2DiffText(path, fromList, toList);
}
