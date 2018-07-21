#ifndef CHANGECATEGORYNAME_H
#define CHANGECATEGORYNAME_H

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class ChangeCategoryName;
}

class ChangeCategoryName : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeCategoryName(const QString &conductorCategory, QWidget *parent = 0);
    ~ChangeCategoryName();

private slots:
    void on_conductorCategoriesLW_itemClicked(QListWidgetItem *item);
    void on_okPB_clicked();
    void changeCategoryName();

private:
    Ui::ChangeCategoryName *ui;
    void setConductorViews();
};

#endif // CHANGECATEGORYNAME_H
