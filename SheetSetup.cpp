#include "SheetSetup.h"
#include "ConductorData.h"
#include <QClipboard>
#include <QFile>
#include <QTextStream>
#include <QHeaderView>
#include <QApplication>

void SheetSetup::setTables(const QTableWidget *table)
{
    int row;
    int column;
    const int totalRows = table->rowCount();
    const int totalColumns = table->columnCount();

    for(column = 0; column < totalColumns; column++)
        table->horizontalHeader()->setSectionResizeMode(column, QHeaderView::Stretch);

    for(row = 0; row < totalRows; row++)
        table->verticalHeader()->setSectionResizeMode(row, QHeaderView::Stretch);
}

void SheetSetup::setTableWidgetItems(QTableWidget *table)
{
    int row;
    int column;
    const int totalRows = table->rowCount();
    const int totalColumns = table->columnCount();

    for(column = 0; column < totalColumns; column++)
    {
        for(row = 0; row < totalRows; row++)
            table->setItem(row, column, new QTableWidgetItem);
    }
}

void SheetSetup::highlight(QTableWidget *table)
{
    int row;
    int column;
    const int totalRows = table->rowCount();
    const int totalColumns = table->columnCount();

    for(row = 0; row < totalRows; row++)
    {
        for(column = 0; column < totalColumns; column++)
        {
            if(table->item(row, column)->isSelected())
                table->item(row, column)->setTextColor(Qt::red);
        }
    }
    table->clearSelection();
}

void SheetSetup::unhighlight(QTableWidget *table)
{
    int row;
    int column;
    const int totalRows = table->rowCount();
    const int totalColumns = table->columnCount();

    for(row = 0; row < totalRows; row++)
    {
        for(column = 0; column < totalColumns; column++)
        {
            if(table->item(row, column)->isSelected())
                table->item(row, column)->setTextColor(Qt::black);
        }
    }
    table->clearSelection();
}

void SheetSetup::unhighlightAll(const QTableWidget *table)
{
    int row;
    int column;
    const int totalRows = table->rowCount();
    const int totalColumns = table->columnCount();

    for(row = 0; row < totalRows; row++)
    {
        for(column = 0; column < totalColumns; column++)
            table->item(row, column)->setTextColor(Qt::black);
    }
}

void SheetSetup::copyCells(const QTableWidget *table)
{
    QString copiedCells;
    const int totalColumns = table->columnCount();
    const int totalRows = table->rowCount();
    bool firstCellSelected = false;
    int firstRow;
    int firstColumn;

    for(int row = 0; row < totalRows; row++)
    {
        for(int column = 0; column < totalColumns; column++)
        {
            if(table->item(row, column)->isSelected() && firstCellSelected == false)
            {
                firstCellSelected = true;
                firstRow = row;
                firstColumn = column;
            }

        }
    }

    for(int row = firstRow; row < totalRows; row++)
    {
        for(int column = firstColumn; column < totalColumns; column++)
        {
            if(table->item(row, column)->isSelected() == true)
            {
                copiedCells.append(table->item(row, column)->text());
                copiedCells.append("\t");
            }
        }
        copiedCells.append("\n");
    }
    QApplication::clipboard()->setText(copiedCells);
}

double SheetSetup::getTemperatureFromString(const QString &stringReceive)
{
    int index = 0;
    QString tempString;
    while(stringReceive[index].isDigit() || stringReceive[index] == '.' || stringReceive[index] == '-')
    {
        tempString.append(stringReceive[index]);
        index++;
    }
    return tempString.toDouble();
}

//bool SheetSetup::isFileOpen(const QString &filePathReceive)
//{
//    int index = 0;
//    const int filePathReceiveSize = filePathReceive.size();
//    bool fileOpen = false;
//    const QString settingsPath = qApp->applicationDirPath() + "/Settings.dat";
//    QString filePath;
//    QFile settings(settingsPath);
//    QTextStream textStream(&settings);
//    QStringList conductorDataList;

//    settings.open(QFile::ReadOnly);
//    while(!textStream.atEnd())
//        conductorDataList.append(textStream.readLine());
//    settings.close();

//    for(int i = 0; i < filePathReceiveSize; i++)
//    {
//        if(filePathReceive[i] == '\\')
//            filePath.append("/");
//        else
//            filePath.append(filePathReceive[i]);
//    }

//    while(conductorDataList[index] != "$OPEN_FILES")
//        index++;
//    index++;

//    while(conductorDataList[index] != "$ATTRIBUTE_END")
//    {
//        if(conductorDataList[index] == filePath)
//            fileOpen = true;
//        index++;
//    }

//    return fileOpen;
//}

void SheetSetup::setPreviousFilePath(const QString &filePathReceive, const QString &attribute)
{
    if(filePathReceive != "")
    {
        int index = 0;
        const int filePathReceiveSize = filePathReceive.size();
        const QString settingsPath = qApp->applicationDirPath() + "/Settings.dat";
        QString filePath, temp;
        QFile settings(settingsPath);
        QTextStream textStream(&settings);
        QStringList conductorDataList;

        settings.open(QFile::ReadOnly);
        while(!textStream.atEnd())
            conductorDataList.append(textStream.readLine());
        settings.close();

        for(int i = 0; i < filePathReceiveSize; i++)
        {
            if(filePathReceive[i] == '\\')
                temp.append("/");
            else
                temp.append(filePathReceive[i]);
        }

        index = temp.size();
        while(temp[index] != '/')
            index--;

        for(int i = 0; i <= index; i++)
            filePath.append(temp[i]);

        index = 0;

        while(conductorDataList[index] != attribute)
            index++;
        index++;

        settings.close();

        if(conductorDataList[index] != "$ATTRIBUTE_END")
            removeFilePath(conductorDataList[index], attribute);
        conductorDataList.clear();

        index = 0;

        settings.open(QFile::ReadOnly);
        while(!textStream.atEnd())
            conductorDataList.append(textStream.readLine());
        settings.close();

        while(conductorDataList[index] != attribute)
            index++;
        index++;

        settings.open(QFile::WriteOnly | QFile::Truncate);

        for(int i = 0; i < index; i++)
        {
            textStream << conductorDataList[i];
            textStream << "\n";
        }
        textStream << filePath;
        textStream << "\n";

        for(int i = index; i < conductorDataList.size(); i++)
        {
            textStream << conductorDataList[i];
            textStream << "\n";
        }
        settings.close();
    }
}

QString SheetSetup::getPreviousPath(const QString &attribute)
{
    int index = 0;
    const QString settingsPath = qApp->applicationDirPath() + "/Settings.dat";
    QString filePath;
    QFile settings(settingsPath);
    QTextStream textStream(&settings);
    QStringList conductorDataList;

    settings.open(QFile::ReadOnly);
    while(!textStream.atEnd())
        conductorDataList.append(textStream.readLine());
    settings.close();

    while(conductorDataList[index] != attribute)
        index++;
    index++;

    if(conductorDataList[index] != "$ATTRIBUTE_END")
        filePath = conductorDataList[index];

    return filePath;
}

void SheetSetup::removeFilePath(const QString &filePath, const QString &attribute)
{
    if(filePath != "")
    {
        int index = 0;
        int toBeRemoved = 0;
        const QString settingsPath = qApp->applicationDirPath() + "/Settings.dat";
        QFile settings(settingsPath);
        QTextStream textStream(&settings);
        QStringList conductorDataList;

        settings.open(QFile::ReadOnly);
        while(!textStream.atEnd())
            conductorDataList.append(textStream.readLine());
        settings.close();

        while(conductorDataList[index] != attribute)
            index++;
        index++;

        while(conductorDataList[index] != "$ATTRIBUTE_END")
        {
            if(conductorDataList[index] == filePath)
                toBeRemoved = index;
            index++;
        }

        settings.open(QFile::WriteOnly | QFile::Truncate);

        for(int i = 0; i < conductorDataList.size(); i++)
        {
            if(conductorDataList[i] != filePath && i != toBeRemoved)
            {
                textStream << conductorDataList[i];
                textStream << "\n";
            }
        }
        settings.close();
    }
}
