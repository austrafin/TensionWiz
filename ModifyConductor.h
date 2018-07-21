#ifndef MODIFYCONDUCTOR_H
#define MODIFYCONDUCTOR_H

#include "AddModifyUi.h"

namespace Ui {
class ModifyConductor;
}

class ModifyConductor : public QDialog
{
    Q_OBJECT

public:
    explicit ModifyConductor(const QString &, QWidget *parent = 0);
    ~ModifyConductor();

private slots:

    bool isExistingCond(const QString &);
    void on_okPB_clicked();
    void on_categoryLE_textChanged(const QString &arg1);
    void on_cdataCategoryCB_currentTextChanged();

private:
    Ui::AddConductor *ui;
    QString oldConductorName;
    QString oldConductorCategory;
    AddModifyUi *uiSetup;
};

#endif // MODIFYCONDUCTOR_H
