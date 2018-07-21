#ifndef SHEETSETUP_H
#define SHEETSETUP_H

#include <QTableWidget>

class SheetSetup
{
public:

    static void setTables(const QTableWidget *table);
    static void setTableWidgetItems(QTableWidget *table);
    static void highlight(QTableWidget *);
    static void unhighlight(QTableWidget *);
    static void unhighlightAll(const QTableWidget *);
    static void copyCells(const QTableWidget *table);
    static void setPreviousFilePath(const QString &filePathReceive, const QString &attribute);
    static double getTemperatureFromString(const QString &);
    static QString getPreviousPath(const QString &attribute);
    static void removeFilePath(const QString &filePath, const QString &attribute);
};

#endif // SHEETSETUP_H
