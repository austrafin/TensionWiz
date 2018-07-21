#ifndef EXPORTOUTPUTSETTINGS_H
#define EXPORTOUTPUTSETTINGS_H

#include <QDialog>
#include <QCheckBox>
#include "CalculateTensionChange.h"
#include "SheetFormats.h"
#include <QComboBox>

namespace Ui {
class ExportOutputSettings;
}

class ExportOutputSettings : public QDialog
{
    Q_OBJECT

public:
    explicit ExportOutputSettings(const int fileType, const CalculateTensionChange &tChangeReceive, const SheetFormats &sheetInfoReceive, QWidget *parent = 0);
    ~ExportOutputSettings();

private slots:

    void on_okCancelPB_accepted();   
    void on_allCB_stateChanged(const int arg1);
    void on_filetypeCB_currentIndexChanged(const int &arg1);
    void cbStateChanged();
    //void highLightBackgroundStateChanged();
    QCheckBox *getCheckBox(const int index);

private:

    QString setPrint();

    Ui::ExportOutputSettings *ui;
    bool signalPreventSettings;
    bool signalPrevent;
    int fileTypeIndex;
    bool highLightColourState; // Until I find a way to get the background colour to work in XLSX
    bool backgroundColourState; // Until I find a way to get the background colour to work in XLSX
    CalculateTensionChange tChange;
    SheetFormats sheetInfo;
    QComboBox *sagCB;
};

#endif // EXPORTOUTPUTSETTINGS_H
