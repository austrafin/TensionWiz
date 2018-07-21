#ifndef ADDMODIFYUI_H
#define ADDMODIFYUI_H

#include <QTimer>
#include "ui_AddConductor.h"

class AddModifyUi : public QObject
{
    Q_OBJECT
public:

    explicit AddModifyUi(Ui_AddConductor *uiReceive, const QString &currentCategoryReceive, QObject *parent = 0);
    ~AddModifyUi();

    void setUserInterface(QDialog *dialog);
    bool verifyInputs(QWidget *w);
    QString getConductorCategory();
    QStringList getNewValues();

private slots:
    void blinkLabel();
    void changeValidator(const QString &cbText);

private:
    //Ui::AddConductor *uim;
    Ui_AddConductor *ui;
    QTimer *timer;
    int timerIndex;
    bool condition_name;
    QString currentCategory;
};

#endif // ADDMODIFYUI_H
