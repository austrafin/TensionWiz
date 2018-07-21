#ifndef ADDCONDUCTOR_H
#define ADDCONDUCTOR_H

#include "AddModifyUi.h"

namespace Ui {
class AddConductor;
}

class AddConductor : public QDialog
{
    Q_OBJECT

public:
    explicit AddConductor(const QString &currentCategory, QWidget *parent = 0);
    ~AddConductor();

private slots:

    void on_okPB_clicked();  
    void on_categoryLE_textChanged(const QString &arg1);
    void on_cdataCategoryCB_currentTextChanged();
    bool isExistingCond(const QString &);

private:
    Ui::AddConductor *ui;

    AddModifyUi *uiSetup;
};

#endif // ADDCONDUCTOR_H
