#include "SheetFormats.h"

void SheetFormats::setConductorName(const QString &name)
{
    conductorName = name;
}

void SheetFormats::setInitCondTW(QTableWidget *tw)
{
    initCondTW = tw;
}

void SheetFormats::setSagPrecision(const int precision)
{
    sagPrecision = precision;
}

void SheetFormats::setTensionPrecision(const int precision)
{
    tensionPrecision = precision;
}

void SheetFormats::setSwingPrecision(const int precision)
{
    swingPrecision = precision;
}

void SheetFormats::setTabLabels(const QStringList &labels)
{
    tabLabels = labels;
}

void SheetFormats::setFontName(const QString &name)
{
    fontName = name;
}

void SheetFormats::setProjectTitle(const QString &title)
{
    projectTitle = title;
}

void SheetFormats::setActualSpanIndex(const int index)
{
    actualSpanIndex = index;
}

void SheetFormats::setResultsTables(const QVector<QTableWidget *> &tablesNew)
{
    resultsTables = tablesNew;
}

void SheetFormats::setSettlingIns(const QVector<double> &values)
{
    settlingIns = values;
}

void SheetFormats::setActualSpan(const QString &span)
{
    actualSpan = span;
}

void SheetFormats::setFontSize(const int size)
{
    fontSize = size;
}

QString SheetFormats::getConductorName()
{
    return conductorName;
}

QString SheetFormats::getFontName()
{
    return fontName;
}

QString SheetFormats::getProjectTitle()
{
    return projectTitle;
}

QString SheetFormats::getActualSpan()
{
    return actualSpan;
}

QTableWidget* SheetFormats::getInitCondTW()
{
    return initCondTW;
}

QVector<QTableWidget*> SheetFormats::getResultsTables()
{
    return resultsTables;
}

int SheetFormats::getNumberOfTabs()
{
    return tabLabels.size();
}
int SheetFormats::getSagPrecision()
{
    return sagPrecision;
}

int SheetFormats::getSwingPrecision()
{
    return swingPrecision;
}

int SheetFormats::getTensionPrecision()
{
    return tensionPrecision;
}

int SheetFormats::getActualSpanIndex()
{
    return actualSpanIndex;
}

int SheetFormats::getfontSize()
{
    return fontSize;
}

QStringList SheetFormats::getTabLabels()
{
    return tabLabels;
}

QVector<double> SheetFormats::getSettlingIns()
{
    return settlingIns;
}


