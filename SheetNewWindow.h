#ifndef SHEETNEWWINDOW_H
#define SHEETNEWWINDOW_H

#include "ExportOutputSettings.h"

namespace Ui {
class SheetNewWindow;
}

class SheetNewWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SheetNewWindow(const CalculateTensionChange &tChangeReceive, SheetFormats &sheetInfo);
    bool eventFilter(QObject *watched, QEvent *e);
    ~SheetNewWindow();

private slots:

    QTableWidget* getTable(const int);
    SheetFormats getSheetInfo();

    void on_resultsTabWidget_currentChanged();
    void on_printPB_clicked();
    void on_highlightPB_clicked();
    void on_unhighlightPB_clicked();
    void on_unhighlightAllPB_clicked();
    void on_exportPDFPB_clicked();
    void keyPressEvent(QKeyEvent *keyevent);
    void copyCells();
    void showSheetMenu(const QPoint &pos);

    void on_exportSpreadsheetPB_clicked();
    void on_clearSelectionPB_clicked();

private:
    Ui::SheetNewWindow *ui;

    QTableWidget *stringingChartTW;
    QTableWidget *finalCondNoWindTW;
    QTableWidget *finalCondInitWindTW;
    QTableWidget *finalCondT500TW;
    QTableWidget *sagsNoWindTW;
    QTableWidget *sagsInitWindTW;
    QTableWidget *sagsT500WindTW;
    QTableWidget *swingsInitWindTW;
    QTableWidget *swingsT500WindTW;
    QString *plot;
    QString plotPaper;
    ExportOutputSettings *outputSettings;
    CalculateTensionChange tChange;
    QVector<double> settlingIns;
    int sagPrecision;
    int swingPrecision;
    int tensionPrecision;
    int actualSpanIndex;
};

#endif // SHEETNEWWINDOW_H
