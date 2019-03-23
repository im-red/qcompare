#ifndef EDITDISTANCE_H
#define EDITDISTANCE_H

#include <QString>
#include <vector>
#include <map>

enum EditOperation
{
    Nop,
    Add,
    Remove,
    Replace
};

int stringEditDistance(const QString &from, const QString &to);
std::vector<EditOperation> articleEditOperations(const QString &from, const QString &to);
std::vector<EditOperation> articleEditOperations(const QStringList &fromList, const QStringList &toList);

#endif // EDITDISTANCE_H
