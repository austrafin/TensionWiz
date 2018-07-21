#ifndef CONDUCTORDATA_H
#define CONDUCTORDATA_H

#include <QStringList>

class ConductorData
{
public:
    static QStringList getConductorData();
    static void removeConductor(const QString &);
    static void replaceConductor(const QStringList &newValues, const QString &);
    static void addConductor(const QStringList &newValues);
    static int findAttribute(const QStringList &conductorData, const QString &attribute, const QString &conductorName);
    static QString getAttributeWithoutUnit(const QStringList &conductorData, const QString &attribute, const QString &conductorName);
    static QString getFilePath();
    static QStringList getConductorAttributes(const bool includeCategory);
    static QStringList getConductorCategories();
    static QStringList getConductorNames(const QString &category);
    static QString getAttributeUnit(const QStringList &conductorData, const QString &attribute, const QString &conductorName);
};

#endif // CONDUCTORDATA_H
