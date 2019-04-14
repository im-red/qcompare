#ifndef LINEDIFF_H
#define LINEDIFF_H

#include "editdistance.h"
#include "diffline.h"
#include "difftext.h"

#include <assert.h>

template <typename StringType>
using SpLine = std::shared_ptr<DiffLine<StringType>>;

template <typename StringType>
void doLineDiff(const SpLine<StringType> &fromLine, const SpLine<StringType> &toLine)
{
    auto ops = editOperations(*(fromLine->string()), *(toLine->string()));

    std::vector<int> fromSubSection(1, -1);
    std::vector<LineType> fromSubType(1, LineType::Invalid);
    std::vector<int> toSubSection(1, -1);
    std::vector<LineType> toSubType(1, LineType::Invalid);

    int fromIndex = 0;
    int toIndex = 0;

    for (auto op : ops)
    {
        if (op == EditOperation::Copy)
        {
            if (fromSubType.back() != LineType::Equal)
            {
                fromSubType.push_back(LineType::Equal);
                fromSubSection.push_back(fromIndex);
            }
            if (toSubType.back() != LineType::Equal)
            {
                toSubType.push_back(LineType::Equal);
                toSubSection.push_back(toIndex);
            }
            fromIndex++;
            toIndex++;
        }
        else if (op == EditOperation::Add)
        {
            if (toSubType.back() != LineType::Add)
            {
                toSubType.push_back(LineType::Add);
                toSubSection.push_back(toIndex);
            }
            toIndex++;
        }
        else if (op == EditOperation::Remove)
        {
            if (fromSubType.back() != LineType::Remove)
            {
                fromSubType.push_back(LineType::Remove);
                fromSubSection.push_back(fromIndex);
            }
            fromIndex++;
        }
        else // if Replace
        {
            // this won't happen
            assert(false);
        }
    }

    fromLine->setSubSection(fromSubSection);
    fromLine->setSubType(fromSubType);
    toLine->setSubSection(toSubSection);
    toLine->setSubType(toSubType);
}

template <typename StringType>
void lineDiff(DiffText<StringType> &text)
{
    const int lineLen = text.fromText.size();

    for (int i = 0; i < lineLen; i++)
    {
        auto fromLine = text.fromText[i];
        if (fromLine->type() == LineType::Replace)
        {
            auto toLine = text.toText[i];
            doLineDiff(fromLine, toLine);
        }
    }
}

#endif // LINEDIFF_H
