#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ConductorLibrary.h"
#include "Sheet.h"
#include <QFile>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &fileName, QWidget *parent = 0);

    bool eventFilter(QObject *watched, QEvent *e);

    ~MainWindow();

private slots:

    void on_actionSave_triggered();
    void on_actionSave_As_triggered();
    void on_actionOpen_triggered();
    void on_actionNew_triggered();
    void on_sheetsTabWidget_currentChanged(const int index);
    void on_actionManage_Conductors_triggered();
    void on_actionAbout_triggered();
    void insertSheet();
    void deleteSheet();
    void renameSheet();
    bool closeFile();
    void saveFile(const QString &filePath);
    void openFile(const QString &filePath);
    void replaceValues(const QStringList &newValues, const QString &attribute, QStringList &file);
    void showSheetMenu(const QPoint &pos);
    void copySheet();
    void tabOrderChanged(const int tabIndexCurrent, const int previousTab);
    void on_actionPrint_triggered();
    void on_actionPDF_triggered();
    void on_actionXLSX_triggered();
    void on_actionXLS_triggered();
    void closeEvent ( QCloseEvent * event );
    QString getFileName(const QString &filePath);

private:
    Ui::MainWindow *ui;

    bool signalPrevent;
    bool saveConfirmation;
    bool isMouseReleased;
    int positionGlobal;
    int tabIndexGlobal;
    QPoint globalPoint;
    QVector<Sheet*> sheets; // For storing new sheets
    QVector<QWidget*> sheetWidgets;
    ConductorLibrary *library;
    QFile twzFile;
};

#endif // MAINWINDOW_H
