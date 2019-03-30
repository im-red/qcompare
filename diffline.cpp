#include "diffline.h"

#include <utility>

using namespace std;

DiffLine::DiffLine(LineType type)
    : m_type(type)
{

}

DiffLine::~DiffLine()
{

}

const std::shared_ptr<QString> DiffLine::string()
{
    return m_string;
}

void DiffLine::setString(const QString &str)
{
    m_string = make_shared<QString>(str);
}

const std::shared_ptr<std::vector<int> > DiffLine::subSection()
{
    return m_subSection;
}

void DiffLine::setSubSection(const std::vector<int> &sections)
{
    m_subSection = make_shared<vector<int>>(sections);
}

const std::shared_ptr<std::vector<DiffLine::LineType> > DiffLine::subType()
{
    return m_subType;
}

void DiffLine::setSubType(const std::vector<DiffLine::LineType> &types)
{
    m_subType = make_shared<vector<LineType>>(types);
}
