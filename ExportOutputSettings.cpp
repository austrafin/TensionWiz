#include "ExportOutputSettings.h"
#include "ui_ExportOutputSettings.h"
#include "SheetSetup.h"
#include <QFileDialog>
#include <QTextDocument>
#include <xlslib.h>
#include <QPrinter>
#include <QDesktopServices>
#include <QPrintDialog>
#include <Workbook.h>
#include <QUrl>

ExportOutputSettings::ExportOutputSettings(const int fileType, const CalculateTensionChange &tChangeReceive, const SheetFormats &sheetInfoReceive, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportOutputSettings)
{
    signalPrevent = true;
    signalPreventSettings = true;
    tChange = tChangeReceive;
    sheetInfo = sheetInfoReceive;
    fileTypeIndex = fileType;
    highLightColourState = true;
    backgroundColourState = true;
    const int numberOfBoxes = 9;

    ui->setupUi(this);

    ui->T500CB->setText("T°C+" + QString::number(tChange.getFinalWind()) + "w");

    if(fileTypeIndex == -1)
    {
        this->setWindowTitle("Print");
        delete ui->filetypeCB;
    }
    else
    {
        this->setWindowTitle("Export");
        ui->filetypeCB->setCurrentIndex(fileTypeIndex);
    }

    connect(ui->okCancelPB, SIGNAL(rejected()), this, SLOT(close()));

    for(int i = 0; i < numberOfBoxes; i++)
        connect(getCheckBox(i), SIGNAL(stateChanged(int)), this, SLOT(cbStateChanged()));
}

ExportOutputSettings::~ExportOutputSettings()
{
    delete ui;
}

void ExportOutputSettings::on_allCB_stateChanged(const int arg1)
{
    bool state;
    const int numberOfBoxes = 9;
    ui->stringingChartCB->setChecked(true);

    if(arg1 == 2)
        state = true;
    else
        state = false;

    for(int i = 1; i < numberOfBoxes; i++) // Skipping "All"
    {
        getCheckBox(i)->disconnect(this, SLOT(cbStateChanged()));
        getCheckBox(i)->setChecked(state);
        connect(getCheckBox(i), SIGNAL(stateChanged(int)), this, SLOT(cbStateChanged()));
    }
}

void ExportOutputSettings::on_filetypeCB_currentIndexChanged(const int &arg1)
{
    QCheckBox *highLight = ui->highlightColourCB;
    QCheckBox *background = ui->backgroundColourCB;
    fileTypeIndex = arg1;

    // Until I find a way to get the background colour to work in XLSX
    if(fileTypeIndex == 1)
    {
        highLightColourState = highLight->isChecked();
        backgroundColourState = background->isChecked();

        highLight->setChecked(false);
        background->setChecked(false);
        highLight->setDisabled(true);
        background->setDisabled(true);
        signalPreventSettings = false;
    }
    else
    {
        highLight->setDisabled(false);
        background->setDisabled(false);

        if(signalPreventSettings == false)
        {
            highLight->setChecked(highLightColourState);
            background->setChecked(backgroundColourState);
        }
        signalPreventSettings = true;
    }
}

void ExportOutputSettings::on_okCancelPB_accepted()
{   

    QString filePath;
    QString filename, fileNameSuggestion;

    if(fileTypeIndex != -1) // If paper print, not needed
    {
        filePath = SheetSetup::getPreviousPath("$PREVIOUS_EXPORT_FILEPATH");
        if(sheetInfo.getProjectTitle() == "")
            fileNameSuggestion = filePath + "Untitled";
        else
            fileNameSuggestion = filePath + sheetInfo.getProjectTitle();
    }

    if(fileTypeIndex == -1 || fileTypeIndex == 0) // If paper print or pdf
    {
        QPrinter printer (QPrinter::HighResolution);
        printer.setPageSize(QPrinter::A4);
        printer.setOrientation(QPrinter::Portrait);
        QTextDocument document;
        const QString html = setPrint();

        if(fileTypeIndex == -1) // Paper print
        {
            printer.setOutputFormat(QPrinter::NativeFormat);
            QPrintDialog printDialog(&printer);

            if(printDialog.exec() == QDialog::Accepted)
            {
                const QString html = setPrint();
                document.setHtml(html);
                document.print(&printer);
            }
            close();
        }

        else if(fileTypeIndex == 0) // pdf
        {
            filename = QFileDialog::getSaveFileName(this, QObject::tr("Save As"), fileNameSuggestion, ui->filetypeCB->currentText() + " (*.pdf)");

            if(filename != "")
            {
                printer.setOutputFileName(filename);
                printer.setOutputFormat(QPrinter::PdfFormat);

                document.setHtml(html);
                document.print(&printer);

                const QString path = "file:///" + filename;
                const QUrl url = path;

                QDesktopServices::openUrl(url);
                close();
            }
        }
    }

    else // if not paper print or pdf
    {
        QString actualSpan = sheetInfo.getActualSpan();
        const QStringList tabLabelsTemp = sheetInfo.getTabLabels();
        const QTableWidget *initCondTW = sheetInfo.getInitCondTW();
        QVector<int> tableIndex;
        QVector<QString> tabLabels;
        const QVector<double> settlingIns = sheetInfo.getSettlingIns();
        const QVector<QTableWidget*> tablesTemp = sheetInfo.getResultsTables();
        QVector<QTableWidget*> tables;

        const int totalTabs = sheetInfo.getNumberOfTabs();
        const int initCondTotalColumns = 6;

        for(int i = 0; i < totalTabs; i++)
        {
            if(getCheckBox(i)->isChecked())
            {
                tabLabels.push_back(tabLabelsTemp[i]);
                tableIndex.push_back(i);
                tables.push_back(tablesTemp[i]);
            }
        }
        const int tablesSize = tables.size();
        int row = 0;
        int column;
        int index = 0;
        QVector<double> results;
        QString settlingIn;


        if(fileTypeIndex == 1) // xlsx
        {
            filename = QFileDialog::getSaveFileName(this, QObject::tr("Save As"), fileNameSuggestion, ui->filetypeCB->currentText() + " (*.xlsx)");

            if(filename != "")
            {
                using namespace SimpleXlsx;

                const int cellOffset = 0;
                const int cellHeightLabel = 30;
                const int cellHeightNumber = 15;
                CWorkbook wb;
                ColumnWidth colWidth;
                CellDataStr cellLabel;
                CellDataStr cellNumber;
                std::vector<CellDataStr> cellStrings;
                std::vector<CellDataStr> cellNumbers;
                std::vector<ColumnWidth> colWidths;
                Style style;
                Style styleLabels;

                colWidth.width = 20;
                colWidth.colFrom = 0;

                style.wrapText = true;
                style.horizAlign = ALIGN_H_CENTER;
                style.vertAlign = ALIGN_V_CENTER;
                style.border.top.style = BORDER_THIN;
                style.border.bottom.style = BORDER_THIN;
                style.border.left.style = BORDER_THIN;
                style.border.right.style = BORDER_THIN;
                style.font.theme = true;
                style.font.name = sheetInfo.getFontName().toStdString();

                styleLabels = style;
                styleLabels.font.attributes = FONT_BOLD;

                const int styleIndex = wb.m_styleList.Add(style);
                const int styleIndexLabels = wb.m_styleList.Add(styleLabels);

                for(int tableIndex = 0; tableIndex < tablesSize; tableIndex++)
                {
                    results = tChange.getResults(tables[tableIndex], sheetInfo.getActualSpanIndex(), tableIndex, settlingIns[tableIndex]);

                    if(tables[tableIndex]->columnCount() > initCondTotalColumns)
                        colWidth.colTo = tables[tableIndex]->columnCount();
                    else
                        colWidth.colTo = initCondTotalColumns + 1;

                    colWidths.push_back(colWidth);

                    CWorksheet &sheet = wb.AddSheet(_T(tabLabels[tableIndex].toStdString()), colWidths);

                    settlingIn = QString::number(settlingIns[tableIndex]);

                    // Adding all the titles from Initial Conditions table
                    for(column = 0; column < initCondTotalColumns; column++)
                    {
                        cellLabel.value = initCondTW->horizontalHeaderItem(column)->text().toStdString();
                        cellLabel.style_id = styleIndexLabels;
                        cellStrings.push_back(cellLabel);
                    }

                    // Also adding Actual Span Column title
                    cellLabel.value = "Actual Span";
                    cellLabel.style_id = styleIndexLabels;
                    cellStrings.push_back(cellLabel);

                    // Also adding Settling In Column title
                    cellLabel.value = "Settling In";
                    cellLabel.style_id = styleIndexLabels;
                    cellStrings.push_back(cellLabel);

                    sheet.AddRow(cellStrings, cellOffset, cellHeightLabel);

                    cellStrings.clear();

                    // Adding all the values from Initial Conditions table
                    for(column = 0; column < initCondTotalColumns; column++)
                    {
                        if(initCondTW->item(0, column)->text() != "")
                            cellLabel.value = initCondTW->item(0, column)->text().toStdString();
                        else
                            cellLabel.value = "0";
                        cellLabel.style_id = styleIndex;

                        cellStrings.push_back(cellLabel);
                    }

                    // Also adding Actual Span value
                    cellLabel.value = actualSpan.toStdString();
                    cellLabel.style_id = styleIndex;
                    cellStrings.push_back(cellLabel);

                    // Also adding Settling In value
                    cellLabel.value = settlingIn.toStdString() + "%";
                    cellLabel.style_id = styleIndex;
                    cellStrings.push_back(cellLabel);
                    sheet.AddRow(cellStrings, cellOffset, cellHeightLabel);

                    cellStrings.clear();

                    sheet.AddRow(cellStrings, 0, 0); // Adding an empty row.

                    // Adding all the titles from result table
                    for(column = 0; column < tables[tableIndex]->columnCount(); column++)
                    {
                        cellLabel.value = tables[tableIndex]->horizontalHeaderItem(column)->text().toStdString();
                        cellLabel.style_id = styleIndexLabels;
                        cellStrings.push_back(cellLabel);
                    }
                    sheet.AddRow(cellStrings, 1, cellHeightLabel);
                    cellStrings.clear();

                    for(row = 0; row < tables[tableIndex]->rowCount(); row++)
                    {
                        // Adding temperature from the result table
                        cellLabel.value = tables[tableIndex]->verticalHeaderItem(row)->text().toStdString();
                        cellLabel.style_id = styleIndexLabels;

                        // Adding values from the result table
                        for(column = 0; column < tables[tableIndex]->columnCount(); column++)
                        {
                            if(results[index] != results[index]) // Checking if value is NaN
                            {
                                cellNumber.value = "0";
                                cellNumber.style_id = styleIndex;
                                cellNumbers.push_back(cellNumber);
                            }
                            else
                            {
                                cellNumber.value = QString("%1").arg(results[index], 0, 'f', 4).toStdString();
                                cellNumber.style_id = styleIndex;
                                cellNumbers.push_back(cellNumber);
                            }
                            index++;
                        }
                        sheet.BeginRow(cellHeightNumber);
                        sheet.AddCell(cellLabel);
                        sheet.AddCells(cellNumbers);
                        sheet.EndRow();
                        cellNumbers.clear();
                    }
                    index = 0;

                    cellStrings.clear();
                    cellNumbers.clear();
                    colWidths.clear();
                }

                wb.Save(filename.toStdString());
                close();
            }
        }

        else if(fileTypeIndex == 2)
        {

            filename = QFileDialog::getSaveFileName(this, QObject::tr("Save As"), fileNameSuggestion, ui->filetypeCB->currentText() + " (*.xls)");

            if(filename != "")
            {
                using namespace xlslib_core;
                const bool highLightColour = ui->highlightColourCB->isChecked();
                const bool backGroundColour = ui->backgroundColourCB->isChecked();
                int rowLast;
                const int rowFirst = 4;
                worksheet *sheet;
                workbook wb;
                font_t *font = wb.font(sheetInfo.getFontName().toStdString());
                xf_t *style = wb.xformat();
                const QColor color1(255, 125, 127);
                const QColor color2(255, 238, 42);

                style->SetBorderStyle(BORDER_TOP, BORDER_THIN);
                style->SetBorderStyle(BORDER_BOTTOM, BORDER_THIN);
                style->SetBorderStyle(BORDER_LEFT, BORDER_THIN);
                style->SetBorderStyle(BORDER_RIGHT, BORDER_THIN);
                style->SetFormat(FMT_NUMBER2);
                style->SetHAlign(HALIGN_CENTER);
                style->SetVAlign(VALIGN_CENTER);
                style->SetWrap(1);
                style->SetFont(font);

                for(int tableIndex = 0; tableIndex < tables.size(); tableIndex++)
                {
                    results = tChange.getResults(tables[tableIndex], sheetInfo.getActualSpanIndex(), tableIndex, settlingIns[tableIndex]);
                    sheet = wb.sheet(tabLabels[tableIndex].toStdWString());
                    sheet->defaultColwidth(15);
                    sheet->rowheight(0, 500);
                    sheet->rowheight(1, 500);

                    settlingIn = QString::number(settlingIns[tableIndex]);

                    for(column = 0; column < initCondTotalColumns; column++)
                    {
                        sheet->label(0, column, initCondTW->horizontalHeaderItem(column)->text().toStdWString(), style);
                        sheet->label(1, column, initCondTW->item(0, column)->text().toStdWString(), style);
                        sheet->FindCell(0, column)->fontbold(BOLDNESS_BOLD);
                    }

                    sheet->label(0, initCondTotalColumns, "Actual Span", style);
                    sheet->label(0, initCondTotalColumns + 1, "Settling In", style);
                    sheet->FindCell(0, initCondTotalColumns)->fontbold(BOLDNESS_BOLD);
                    sheet->FindCell(0, initCondTotalColumns + 1)->fontbold(BOLDNESS_BOLD);
                    sheet->label(1, initCondTotalColumns, actualSpan.toStdWString(), style);
                    sheet->label(1, initCondTotalColumns + 1, settlingIn.toStdWString(), style);

                    rowLast = rowFirst + tables[tableIndex]->rowCount();

                    // Setting labels
                    for(column = 0; column < tables[tableIndex]->columnCount(); column++)
                    {
                        sheet->label(rowFirst - 1, column + 1, tables[tableIndex]->horizontalHeaderItem(column)->text().toStdWString(), style);
                        sheet->FindCell(rowFirst - 1, column + 1)->fontbold(BOLDNESS_BOLD);
                    }

                    // Setting temperatures
                    for(row = rowFirst; row < rowLast; row++)
                    {
                        sheet->label(row, 0, tables[tableIndex]->verticalHeaderItem(index)->text().toStdWString(), style);
                        sheet->FindCell(row, 0)->fontbold(BOLDNESS_BOLD);
                        index++;
                    }

                    index = 0;

                    for(row = rowFirst; row < rowLast; row++)
                    {                        
                        //Setting values
                        for(column = 1; column <= tables[tableIndex]->columnCount(); column++)
                        {
                            if(results[index] != results[index]) // Checking if value is NaN
                                sheet->number(row, column, 0, style); // Writing 0 insted of NaN
                            else
                                sheet->number(row, column, results[index], style); // else writing the value
                            sheet->FindCell(row, column)->fontbold(BOLDNESS_NORMAL);

                            if(tables[tableIndex]->item(row - rowFirst, column - 1)->textColor() == Qt::red && highLightColour == true)
                                sheet->FindCell(row, column)->fontcolor(2);
                            if(tables[tableIndex]->item(row - rowFirst, column - 1)->backgroundColor() == color1 && backGroundColour == true)
                            {
                                sheet->FindCell(row, column)->fillstyle(FILL_SOLID);
                                sheet->FindCell(row, column)->fillfgcolor (CLR_RED); // red
                            }
                            else if(tables[tableIndex]->item(row - rowFirst, column - 1)->backgroundColor() == color2 && backGroundColour == true)
                            {
                                sheet->FindCell(row, column)->fillstyle(FILL_SOLID);
                                sheet->FindCell(row, column)->fillfgcolor(CLR_YELLOW); // yellow
                            }
                            index++;
                        }
                    }
                    index = 0;
                }
                wb.Dump(filename.toStdString());
                close();
            }
        }

    }

    if(filename != "")
        SheetSetup::setPreviousFilePath(filename, "$PREVIOUS_EXPORT_FILEPATH");
}

QCheckBox* ExportOutputSettings::getCheckBox(const int index)
{
    if(index == 0)
        return ui->stringingChartCB;
    else if(index == 1)
        return ui->noWindCB;
    else if(index == 2)
        return ui->initWindCB;
    else if(index == 3)
        return ui->T500CB;
    else if(index == 4)
        return ui->sagsNoWindCB;
    else if(index == 5)
        return ui->sagsInitWindCB;
    else if(index == 6)
        return ui->sagsT500CB;
    else if(index == 7)
        return ui->swingsInitWindCB;
    else if(index == 8)
        return ui->swingsT500CB;
    else
        return NULL;
}

void ExportOutputSettings::cbStateChanged()
{
    bool checkState = true;
    const int numberOfBoxes = 9;

    for(int i = 0; i < numberOfBoxes; i++)
    {
        if(getCheckBox(i)->isChecked() == false)
            checkState = false;
    }

    ui->allCB->disconnect(this, SLOT(on_allCB_stateChanged(int)));
    ui->allCB->setChecked(checkState);
    connect(ui->allCB, SIGNAL(stateChanged(int)), this, SLOT(on_allCB_stateChanged(int)));
}


QString ExportOutputSettings::setPrint()
{
    bool rowIsSelected = false;
    bool firstPage = true;
    const bool highLightColour = ui->highlightColourCB->isChecked();
    const bool backgroundColour = ui->backgroundColourCB->isChecked();
    QString html, initialConditions, horizontalLabel, tdStart, tdEnd;
    const QString finalWind = QString::number(tChange.getFinalWind());
    const QString projectTitle = sheetInfo.getProjectTitle();
    const QString actualSpan = sheetInfo.getActualSpan();
    const QString tableBorder = "4";
    const QStringList tabLabels = sheetInfo.getTabLabels();
    const QTableWidget *initCondTable = sheetInfo.getInitCondTW();
    const QVector<double> settlingIns = sheetInfo.getSettlingIns();
    const QVector<QTableWidget*> tables = sheetInfo.getResultsTables();
    const QColor color1(255, 125, 127);
    const QColor color2(255, 238, 42);
    int i2, column, columnLimit, rowFirst = 0, rowLast, row, totalRows, totalColumns;
    const int tablesSize = tables.size();

    html.append("<style> {border: 1px solid black;} </style>");
    initialConditions = "<table border=""" + tableBorder + """ cellpadding=""4"" cellspacing=""0"" width=""100%""> <tr> <th>Conductor</th> <th>Total Length</th> <th>Ruling Span</th> <th>Tension</th> <th>Temperature</th> <th>Wind</th> <th>Settling In</th> </tr> <tr> <td> <center>" + initCondTable->item(0, 0)->text() + "</center> </td>";
    if(initCondTable->item(0, 1)->text() != "N/A" && initCondTable->item(0, 1)->text() != "")
        initialConditions.append("<td> <center>" + initCondTable->item(0, 1)->text() + "m </center> </td>");
    else
        initialConditions.append("<td> <center></center> </td>");

    if(initCondTable->item(0, 2)->text() != "")
        initialConditions.append("<td> <center>" + initCondTable->item(0, 2)->text() + "m </center> </td>");
    else
        initialConditions.append("<td> <center></center> </td>");

    if(initCondTable->item(0, 3)->text() != "")
        initialConditions.append("<td> <center>" + initCondTable->item(0, 3)->text() + "N </center> </td>");
    else
        initialConditions.append("<td> <center></center> </td>");

    if(initCondTable->item(0, 4)->text() != "")
        initialConditions.append("<td> <center>" + initCondTable->item(0, 4)->text() + "°C </center> </td>");
    else
        initialConditions.append("<td> <center></center> </td>");

    if(initCondTable->item(0, 5)->text() != "")
        initialConditions.append("<td> <center>" + initCondTable->item(0, 5)->text() + "Pa </center> </td>");
    else
        initialConditions.append("<td> <center></center> </td>");

    for(int i = 0; i < tablesSize; i++)
    {
        if(getCheckBox(i)->isChecked() == true) // Ignored if not checked
        {
            if(firstPage == false) // if it's not the first page, starting new page
                html.append("<DIV style=""page-break-after:always""></DIV>");

            column = 0;
            columnLimit = 10;
            totalRows = tables[i]->rowCount();
            totalColumns = tables[i]->columnCount();

            // Finding out if the user has selected specific rows to be printed
            for(int r = 0; r < totalRows; r++)
            {
                for(int c = 0; c < totalColumns; c++)
                {
                    if(rowFirst == 0)
                    {
                        if(tables[i]->item(r, c)->isSelected())
                        {
                            rowFirst = r;
                            rowIsSelected = true;
                        }
                    }
                    if(tables[i]->item(r, c)->isSelected())
                    {
                        rowLast = r + 1;
                        rowIsSelected = true;
                    }
                }
            }

            if(rowIsSelected == false)
            {
                rowFirst = 0;
                rowLast = totalRows;
            }

            do
            {
                if(projectTitle != "")
                    html.append("<th><font>" + projectTitle + "</font></th> <br></br>");

                html.append("<table border=""" + tableBorder + """ cellpadding=""4"" cellspacing=""0"" width=""100%""> <tr> <th>" + tabLabels[i] + "</th> </tr></table>");
                html.append(initialConditions);
                html.append("<td> <center>");
                html.append(QString::number(settlingIns[i]));
                html.append("% </center> </td>");
                html.append("</table> <br></br>");

                html.append("<table border=""" + tableBorder + """ cellpadding=""4"" cellspacing=""3"" width=""100%""> <tr> <th>Temperature</th>");

                i2 = column;

                while(i2 < columnLimit && i2 < totalColumns)
                {
                    horizontalLabel = tables[i]->horizontalHeaderItem(i2)->text();
                    html.append("<th>" + horizontalLabel);

                    if((horizontalLabel == "Sag (m)\nWithout Wind" || horizontalLabel == "Sag (m)\n+ " + finalWind + "Pa Wind" || horizontalLabel == "Swing (m) With\nInitial Wind" || horizontalLabel == "Swing (m)\n+ " + finalWind + "Pa Wind") && actualSpan == "Ruling Span")
                       html.append( " (Ruling Span)" );
                    else if((horizontalLabel == "Sag (m)\nWithout Wind" || horizontalLabel == "Sag (m)\n+ " + finalWind + "Pa Wind" || horizontalLabel == "Swing (m) With\nInitial Wind" || horizontalLabel == "Swing (m)\n+ " + finalWind + "Pa Wind") && actualSpan != "Ruling Span")
                        html.append(" (" + actualSpan + ")");

                    html.append("</th>");
                    i2++;
                }

                i2 = column;

                html.append("</tr>");

                // Setting tensions
                for(row = rowFirst; row < rowLast; row++)
                {
                    html.append("<tr> <td>");
                    html.append(tables[i]->verticalHeaderItem(row)->text());
                    html.append("</td>");

                    while(column < columnLimit && column < totalColumns)
                    {
                        tdStart = "<td";
                        tdEnd;

                        if(tables[i]->item(row, column)->textColor() == Qt::red && highLightColour == true)
                        {
                            tdStart.append("><font color=""red""");
                            tdEnd.append("</font>");
                        }
                        if(tables[i]->item(row, column)->backgroundColor() != Qt::white && backgroundColour == true)
                        {
                            if(tables[i]->item(row, column)->backgroundColor() == color1)
                                tdStart.append(" bgcolor=""#FF5050""");
                            else if(tables[i]->item(row, column)->backgroundColor() == color2)
                                tdStart.append(" bgcolor=""#FFFF00""");
                        }

                        tdStart.append(">");

                        tdEnd.append("</td>");
                        html.append(tdStart);
                        html.append(tables[i]->item(row, column)->text());
                        html.append(tdEnd);
                        tdStart.clear();
                        tdEnd.clear();
                        column++;
                    }
                    column = i2;

                    html.append("</tr>");
                }
                column = columnLimit;
                columnLimit += 10;

                html.append("</table>");

                if(column < totalColumns) // if there's more than ten columns, starting a new page
                    html.append("<DIV style=""page-break-after:always""></DIV>");
            }
            while(column < totalColumns);

            rowIsSelected = false;
            rowFirst = 0;
            firstPage = false;
        }
    }
    return html;
}
