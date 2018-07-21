#ifndef SHEETFORMATS_H
#define SHEETFORMATS_H

#include <QTableWidget>

class SheetFormats
{
public:
    void setConductorName(const QString &name);
    void setTabLabels(const QStringList &labels);
    void setTensionPrecision(const int precision);
    void setSagPrecision(const int precision);
    void setSwingPrecision(const int precision);
    void setInitCondTW(QTableWidget *tw);
    void setFontName(const QString &name);
    void setProjectTitle(const QString &title);
    void setActualSpan(const QString &span);
    void setActualSpanIndex(const int index);
    void setResultsTables(const QVector<QTableWidget*> &tablesNew);
    void setSettlingIns(const QVector<double> &values);
    void setFontSize(const int size);

    QString getConductorName();
    QString getProjectTitle();
    QString getFontName();
    QStringList getTabLabels();
    QString getActualSpan();
    int getTensionPrecision();
    int getSagPrecision();
    int getSwingPrecision();
    int getNumberOfTabs();
    int getActualSpanIndex();
    int getfontSize();
    QTableWidget* getInitCondTW();
    QVector<QTableWidget*> getResultsTables();
    QVector<double> getSettlingIns();

private:
    QString conductorName;
    QString fontName;
    QStringList tabLabels;
    QString projectTitle;
    QString actualSpan;
    int tensionPrecision;
    int sagPrecision;
    int swingPrecision;
    int actualSpanIndex;
    int fontSize;
    QTableWidget *initCondTW;
    QVector<QTableWidget*> resultsTables;
    QVector<double> settlingIns;
};

#endif // SHEETFORMATS_H
