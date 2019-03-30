#ifndef DIFFTEXT_H
#define DIFFTEXT_H

#include "diffline.h"

#include <vector>
#include <memory>

struct DiffText
{
    std::vector<std::shared_ptr<DiffLine>> fromText;
    std::vector<std::shared_ptr<DiffLine>> toText;
};

#endif // DIFFTEXT_H
