#include "ConductorData.h"
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>

QStringList ConductorData::getConductorData()
{
    QFile cdata( ConductorData::getFilePath() );
    QTextStream input(&cdata);
    QStringList conductorDataList;

    cdata.open(QFile::ReadOnly);

    while(!input.atEnd())
        conductorDataList.append(input.readLine());

    cdata.close();

    return conductorDataList;
}

QStringList ConductorData::getConductorCategories()
{
    bool isExisting = false;
    QStringList categories;
    const QStringList conductorDataList = ConductorData::getConductorData();
    const int conductorDataListSize = conductorDataList.size() - 1;
    int index = 0;

    while(index < conductorDataListSize)
    {
        if(conductorDataList[index] == "$CONDUCTOR_CATEGORY")
        {
            index++;

            for(int i = 0; i < categories.size(); i++)
            {
                if(conductorDataList[index] == categories[i])
                    isExisting = true;
            }
            if(isExisting == false)
                categories.append(conductorDataList[index]);
            isExisting = false;
        }
        else
            index++;
    }

    categories.sort();

    return categories;
}

QStringList ConductorData::getConductorNames(const QString &category)
{
    QStringList conductors;
    QFile cdata( ConductorData::getFilePath() );

    cdata.open(QFile::ReadOnly);

    if(!cdata.atEnd() && category != "")
    {
        cdata.close();
        const QStringList conductorDataList = ConductorData::getConductorData();
        const int conductorDataListSize = conductorDataList.size();
        int index = 0;

        if(category == "All")
        {
            while(index < conductorDataListSize - 1)
            {
                if(conductorDataList[index] == "$CONDUCTOR_NAME")
                {
                    index++;
                    conductors.append(conductorDataList[index]);
                }
                else
                    index++;
            }
        }

        else
        {
            while(index < conductorDataList.size() - 2)
            {
                while((conductorDataList[index] != "$CONDUCTOR_CATEGORY" || conductorDataList[index + 1] != category) && index < conductorDataList.size() - 2)
                    index++;
                index++;

                while(conductorDataList[index] != "$CONDUCTOR_NAME" && index < conductorDataListSize - 2)
                    index++;

                if(conductorDataList[index] == "$CONDUCTOR_NAME")
                {
                    index++;
                    conductors.append(conductorDataList[index]);
                }
            }
        }
        conductors.sort();
    }
    else
        cdata.close();

    return conductors;
}

void ConductorData::addConductor(const QStringList &newValues)
{
    const QString filepath(ConductorData::getFilePath());
    QFile cdata(filepath);
    QTextStream input(&cdata);
    const QStringList conductorAttributes = getConductorAttributes(1);
    const int conductorAttributesSize = conductorAttributes.size();

    cdata.open(QFile::WriteOnly | QFile::Append);

    input << "$CONDUCTOR_DETAILS_START";
    input << "\n";

    for(int i = 0; i < conductorAttributesSize; i++)
    {
        input << conductorAttributes[i];
        input << "\n";
        input << newValues[i];
        input << "\n";
    }

    input << "$CONDUCTOR_DETAILS_END";
    input << "\n";

    cdata.close();
}

void ConductorData::removeConductor(const QString &conductorName)
{
    QStringList conductorDataList = getConductorData();
    QFile cdata(ConductorData::getFilePath());
    QTextStream input(&cdata);
    int index;
    QStringList replace;

    // Finding the selected conductor.
    index = findAttribute(conductorDataList, "$CONDUCTOR_NAME", conductorName);

    while(conductorDataList[index] != "$CONDUCTOR_DETAILS_START")
        index--; // Going back to Start of details.

    for(int i = 0; i < index; i++)
        replace.append(conductorDataList[i]); // Writing up to the conductor to be removed

    // Skipping the conductor
    while(conductorDataList[index] != "$CONDUCTOR_DETAILS_END")
        index++;
    index++;

    while(index < conductorDataList.size())
    {
        replace.append(conductorDataList[index]);
        index++;
    }

    const int replaceSize = replace.size();
    cdata.open(QFile::WriteOnly | QFile::Truncate);

    for(int i = 0; i < replaceSize; i++)
    {
        input << replace[i];
        input << "\n";
    }

    cdata.close();
}


void ConductorData::replaceConductor(const QStringList &newValues, const QString &conductorName)
{
    removeConductor(conductorName);
    addConductor(newValues);
}

int ConductorData::findAttribute(const QStringList &conductorData, const QString &attribute, const QString &conductorName)
{
    int index = 0;
    const int conductorDataSize = conductorData.size() - 1;

    if(conductorDataSize > 0)
    {
        while((conductorData[index] != "$CONDUCTOR_NAME" || conductorData[index + 1] != conductorName) && index < conductorDataSize)
            index++;
        while(conductorData[index] != "$CONDUCTOR_DETAILS_START")
            index--;
        while(conductorData[index] != attribute && index < conductorDataSize)
            index++;
        index++;
    }

    if(index < conductorDataSize)
        return index;
    else
        return -1;
}

QString ConductorData::getAttributeWithoutUnit(const QStringList &conductorData, const QString &attribute, const QString &conductorName)
{
    int index = findAttribute(conductorData, attribute, conductorName);
    QString attributeConverted;

    if(index >= 0)
    {
        const QString toBeConverted = conductorData[index];

        if(toBeConverted.size() != 0)
        {
            if(attribute == "$COMMENT")
            {
                while(conductorData[index] != "$CONDUCTOR_DETAILS_END" && index < conductorData.size())
                {
                    attributeConverted.append(conductorData[index]);
                    //qDebug() << conductorData[index];
                    attributeConverted.append("\n");
                    index++;
                }

            }
            else
            {
                index = 0;
                while(toBeConverted[index] != '$' && index < toBeConverted.size())
                {
                    attributeConverted.append(toBeConverted[index]);
                    index++;

                    if(index >= toBeConverted.size())
                        break;
                }
            }
        }
    }

    return attributeConverted;
}

QString ConductorData::getAttributeUnit(const QStringList &conductorData, const QString &attribute, const QString &conductorName)
{
    QString unit;
    const QString toBeConverted = conductorData[findAttribute(conductorData, attribute, conductorName)];
    int index = toBeConverted.size() - 1;

    if(toBeConverted.size() != 0)
    {
        while(toBeConverted[index] != '$' && index > 0)
        {
            unit.push_front(toBeConverted[index]);
            index--;

            if(index < 0)
                break;
        }
    }
    return unit;
}

QString ConductorData::getFilePath()
{
    return QCoreApplication::applicationDirPath() + "/Conductordata.dat";
}

QStringList ConductorData::getConductorAttributes(const bool includeCategory)
{
    QStringList attributes;

    if(includeCategory == true)
        attributes.append("$CONDUCTOR_CATEGORY");

    attributes.append("$CONDUCTOR_NAME");
    attributes.append("$STRANDING_AND_WIRE_DIAMETER");
    attributes.append("$TOTAL_DIAMETER");
    attributes.append("$CROSS_SECTIONAL_AREA");
    attributes.append("$MASS");
    attributes.append("$ULTIMATE_TENSILE_STRENGTH");
    attributes.append("$MODULUS_OF_ELASTICITY");
    attributes.append("$COEFFICIENT_OF_EXPANSION");
    attributes.append("$RESISTANCE_AT_20C");
    attributes.append("$COMMENT");

    return attributes;
}
