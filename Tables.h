#ifndef TABLES_H
#define TABLES_H

#include <QStringList>

class Tables
{
public:
    static QStringList getTables(QString, int, int, int, double, int, int);

    static QStringList getColours();
};

#endif // TABLES_H
