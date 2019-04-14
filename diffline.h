#ifndef DIFFLINE_H
#define DIFFLINE_H

#include <vector>
#include <memory>
#include <utility>

enum class LineType
{
    Empty,
    Equal,
    Add,
    Remove,
    Replace,
    Invalid
};

template <typename StringType>
class DiffLine
{
public:
    DiffLine(LineType type)
        : m_type(type)
    {

    }

    ~DiffLine()
    {

    }

    LineType type() { return m_type; }
    void setType(LineType type) { m_type = type; }

    const std::shared_ptr<StringType> string()
    {
        return m_string;
    }

    void setString(const StringType &str)
    {
        m_string = std::make_shared<StringType>(str);
    }

    const std::shared_ptr<std::vector<int> > subSection()
    {
        return m_subSection;
    }

    void setSubSection(const std::vector<int> &sections)
    {
        m_subSection = std::make_shared<std::vector<int>>(sections);
    }

    const std::shared_ptr<std::vector<LineType> > subType()
    {
        return m_subType;
    }

    void setSubType(const std::vector<LineType> &types)
    {
        m_subType = std::make_shared<std::vector<LineType>>(types);
    }

private:
    LineType m_type;
    std::shared_ptr<StringType> m_string;
    std::shared_ptr<std::vector<int>> m_subSection;
    std::shared_ptr<std::vector<LineType>> m_subType;
};

#endif // DIFFLINE_H
