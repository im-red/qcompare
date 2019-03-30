#ifndef DIFFLINE_H
#define DIFFLINE_H

#include <vector>
#include <QString>
#include <memory>

class DiffLine
{
public:
    enum LineType
    {
        Empty,
        Equal,
        Add,
        Remove,
        Replace
    };
public:
    DiffLine(LineType type);
    ~DiffLine();
    LineType type() { return m_type; }
    void setType(LineType type) { m_type = type; }

    const std::shared_ptr<QString> string();
    void setString(const QString &str);
    const std::shared_ptr<std::vector<int>> subSection();
    void setSubSection(const std::vector<int> &sections);
    const std::shared_ptr<std::vector<LineType>> subType();
    void setSubType(const std::vector<LineType> &types);

private:
    LineType m_type;
    std::shared_ptr<QString> m_string;
    std::shared_ptr<std::vector<int>> m_subSection;
    std::shared_ptr<std::vector<LineType>> m_subType;
};

#endif // DIFFLINE_H
