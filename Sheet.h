#ifndef SHEET_H
#define SHEET_H
#include "ui_Sheet.h"
#include "SheetNewWindow.h"

namespace Ui {
class Sheet;
}

class Sheet : public QWidget
{
    Q_OBJECT


public:

    explicit Sheet(QWidget *parent = 0);

    Ui::Sheet *ui;
    bool saveConfirmation;
    QVector<double> spans;
    QVector<double> spansRestoreTemp;

    // Functions
    void setActualSpanRulingSpanOnly();
    void setConductorCategory();
    void setConductorType();
    void addSpanColumn(const QString &spanText);
    QTableWidget* getTable(int table);
    bool getSettlingInState(int index);
    QDoubleSpinBox* getSettlingInSpinBox(int index);
    QCheckBox* getSettlingInCheckBox(int index);
    double getSettlingIn(int index);
    ~Sheet();

public slots:

    void addSpan(const double span);
    void removeAllRows(QTableWidget *table);
    void on_printPB_clicked();
    void on_exportPDFPB_clicked();
    void exportSpreadSheet(int fileType = 1);
    void on_initTensionLE_textChanged(const QString &arg1);
    void calculateTables(); // This needs to be a SLOT!!!

private slots:

    void on_addSpanPB_clicked();
    void on_resetPB_clicked();
    void on_deletePB_clicked();
    void on_changePB_clicked();
    void on_utsLE_textChanged(const QString &arg1);
    void on_cdataCB_currentTextChanged(const QString &arg1);
    void on_initTempLE_textChanged(const QString &arg1);
    void on_initWindLE_textChanged(const QString &arg1);
    void on_rulingSpanLE_textChanged(const QString &arg1);
    void on_restorePB_clicked();
    void on_SagSwingCB_currentTextChanged();
    void on_newWindowPB_clicked();
    void on_cdataCategoryCB_currentTextChanged();
    void on_addRowsPB_clicked();
    void on_deleteRowPB_clicked();
    void on_highlightPB_clicked();
    void on_unhighlightPB_clicked();
    void on_unhighlightAllPB_clicked();
    void on_clearTablePB_clicked();
    void on_exportXLSPB_clicked();
    void on_finalWindLE_textChanged();
    void setFont();
    void setResultsTabTexts();
    void copyCells();
    void removeSpanColumn(const int column);
    void showSheetMenu(const QPoint& pos);
    void keyPressEvent(QKeyEvent *);
    QVector<double> getConductorAttributes(const QString &conductorName);
    CalculateTensionChange getTensionChangeValues();
    SheetFormats getSheetInfo();
    bool eventFilter(QObject *watched, QEvent *e);

    void on_clearSelectionPB_clicked();

private:

    bool signalPrevent;
    SheetNewWindow *newWindow;
    ExportOutputSettings *outputSettings;
};

#endif // Sheet_H
