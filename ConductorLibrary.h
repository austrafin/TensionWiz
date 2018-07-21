#ifndef CONDUCTORLIBRARY_H
#define CONDUCTORLIBRARY_H

#include "AddConductor.h"
#include "ModifyConductor.h"
#include "ChangeCategoryName.h"

namespace Ui {
class ConductorLibrary;
}

class ConductorLibrary : public QDialog
{
    Q_OBJECT

public:
    explicit ConductorLibrary(QWidget *parent = 0);
    void setConductorViews();
    void importConductors(const QStringList &files);
    ~ConductorLibrary();

private slots:

    void on_conductorLibraryCB_currentTextChanged(const QString &arg1);
    void on_addPB_clicked();
    void on_deletePB_clicked();
    void on_modifyPB_clicked();
    void on_fontSB_valueChanged(const int arg1);
    void keyPressEvent(QKeyEvent *keyevent);
    void on_exportPB_clicked();
    void on_importPB_clicked();
    void on_conductorLibraryLW_itemSelectionChanged();

    void on_changeCategoryNamePB_clicked();

    void on_exportAllPB_clicked();
    void exportConductors(const QStringList &filePaths, const QStringList &conductorNames);
    QString convertConductorName(const QString &conductorName);

    void on_deleteAllPB_clicked();

private:
    Ui::ConductorLibrary *ui;
    AddConductor *newconductor;
    ModifyConductor *modifyConductor;
    ChangeCategoryName *newCategoryName;
};

#endif // CONDUCTORLIBRARY_H
