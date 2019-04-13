#ifndef DIFFTEXT_H
#define DIFFTEXT_H

#include "diffline.h"

#include <vector>
#include <memory>

template <typename StringType>
struct DiffText
{
    std::vector<std::shared_ptr<DiffLine<StringType>>> fromText;
    std::vector<std::shared_ptr<DiffLine<StringType>>> toText;
};

#endif // DIFFTEXT_H
