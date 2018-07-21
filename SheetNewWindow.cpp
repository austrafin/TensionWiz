#include "SheetNewWindow.h"
#include "SheetSetup.h"
#include "ui_SheetNewWindow.h"
#include <QMouseEvent>
#include <QMenu>

SheetNewWindow::SheetNewWindow(const CalculateTensionChange &tChangeReceive, SheetFormats &sheetInfo) :

    ui(new Ui::SheetNewWindow)
{
    ui->setupUi(this);
    tChange = tChangeReceive;
    this->setWindowTitle(sheetInfo.getProjectTitle());
    const QVector<QTableWidget*> receivedTables = sheetInfo.getResultsTables();
    QVector<QTableWidget*> tables;
    const QStringList tabLabels = sheetInfo.getTabLabels();
    const QTableWidget *initCond = sheetInfo.getInitCondTW();
    QTableWidget *table;
    const QFont font(sheetInfo.getFontName(), sheetInfo.getfontSize());
    int column;
    int row;

    stringingChartTW = new QTableWidget(this);
    finalCondNoWindTW = new QTableWidget(this);
    finalCondInitWindTW = new QTableWidget(this);
    finalCondT500TW = new QTableWidget(this);
    sagsNoWindTW = new QTableWidget(this);
    sagsInitWindTW = new QTableWidget(this);
    sagsT500WindTW = new QTableWidget(this);
    swingsInitWindTW = new QTableWidget(this);
    swingsT500WindTW = new QTableWidget(this);
    tensionPrecision = sheetInfo.getTensionPrecision();
    sagPrecision = sheetInfo.getSagPrecision();
    swingPrecision = sheetInfo.getSwingPrecision();
    actualSpanIndex = sheetInfo.getActualSpanIndex();
    settlingIns = sheetInfo.getSettlingIns();

    tables.push_back(stringingChartTW);
    tables.push_back(finalCondNoWindTW);
    tables.push_back(finalCondInitWindTW);
    tables.push_back(finalCondT500TW);
    tables.push_back(sagsNoWindTW);
    tables.push_back(sagsInitWindTW);
    tables.push_back(sagsT500WindTW);
    tables.push_back(swingsInitWindTW);
    tables.push_back(swingsT500WindTW);

    for(int i = 0; i < tables.size(); i++)
    {
        ui->resultsTabWidget->setTabText(i, tabLabels[i]);
        table = tables[i];

        table->viewport()->installEventFilter(this);

        table->setColumnCount(receivedTables[i]->columnCount()); // setting columns
        table->setRowCount(receivedTables[i]->rowCount()); // setting rows

        // Setting columns header Text
        for(column = 0; column < table->columnCount(); column++)
        {
            table->setHorizontalHeaderItem(column, new QTableWidgetItem);
            table->horizontalHeaderItem(column)->setText(receivedTables[i]->horizontalHeaderItem(column)->text());
            table->horizontalHeader()->setSectionResizeMode(column, QHeaderView::Stretch);
        }

        // Setting Rows header Text
        for(row = 0; row < table->rowCount(); row++)
        {
            table->setVerticalHeaderItem(row, new QTableWidgetItem);
            table->verticalHeaderItem(row)->setText(receivedTables[i]->verticalHeaderItem(row)->text());
            table->verticalHeader()->setSectionResizeMode(row, QHeaderView::Stretch);
        }

        for(column = 0; column < table->columnCount(); column++)
        {
            for(row = 0; row < table->rowCount(); row++)
            {
                table->setItem(row, column, new QTableWidgetItem);
                table->item(row, column)->setText(receivedTables[i]->item(row, column)->text());
                table->item(row, column)->setBackground(receivedTables[i]->item(row, column)->background());
                table->item(row, column)->setFont(receivedTables[i]->item(row, column)->font());
            }
        }

        table->horizontalHeader()->setVisible(true);
        table->verticalHeader()->setVisible(true);
        table->horizontalHeader()->setFont(receivedTables[i]->horizontalHeader()->font());
        table->verticalHeader()->setFont(receivedTables[i]->verticalHeader()->font());
    }

    SheetSetup::setTables(ui->initialCondTW);
    SheetSetup::setTableWidgetItems(ui->initialCondTW);

    // Making sure the headers are correctly displayed (glitch in editor).
    ui->initialCondTW->horizontalHeader()->setVisible(true);
    ui->initialCondTW->verticalHeader()->setVisible(false);

    // Setting Initial Conditions Table except for actual Span & settling in
    for(column = 0; column < initCond->columnCount(); column++)
        ui->initialCondTW->item(0, column)->setText(initCond->item(0, column)->text());

    ui->initialCondTW->item(0, 6)->setText(sheetInfo.getActualSpan()); // setting actual span

    // Setting horizontal header fonts
    ui->initialCondTW->horizontalHeader()->setFont(font);
    ui->initialCondTW->horizontalHeaderItem(0)->setFont(font); // Because of glitch

    ui->tab_stringingChart->layout()->addWidget(stringingChartTW);
    ui->tab_finalCondNoWind->layout()->addWidget(finalCondNoWindTW);
    ui->tab_finalCondInitWind->layout()->addWidget(finalCondInitWindTW);
    ui->tab_finalCondT500->layout()->addWidget(finalCondT500TW);
    ui->tab_sagsNoWind->layout()->addWidget(sagsNoWindTW);
    ui->tab_sagsInitWind->layout()->addWidget(sagsInitWindTW);
    ui->tab_sagsT500Wind->layout()->addWidget(sagsT500WindTW);
    ui->tab_swingsInitWind->layout()->addWidget(swingsInitWindTW);
    ui->tab_swingsT500Wind->layout()->addWidget(swingsT500WindTW);

    connect(ui->highlightPB, SIGNAL(clicked()), SLOT(highlight()));
    connect(ui->unhighlightPB, SIGNAL(clicked()), SLOT(unhighlight()));
    connect(ui->unhighlightAllPB, SIGNAL(clicked()), SLOT(unhighlightAll()));

    on_resultsTabWidget_currentChanged();
}

SheetNewWindow::~SheetNewWindow()
{
    delete ui;
}

QTableWidget* SheetNewWindow::getTable(const int table)
{
    if(table == 0)
        return stringingChartTW;
    else if(table == 1)
        return finalCondNoWindTW;
    else if(table == 2)
        return finalCondInitWindTW;
    else if(table == 3)
        return finalCondT500TW;
    else if(table == 4)
        return sagsNoWindTW;
    else if(table == 5)
        return sagsInitWindTW;
    else if(table == 6)
        return sagsT500WindTW;
    else if(table == 7)
        return swingsInitWindTW;
    else if(table == 8)
        return swingsT500WindTW;
    else
        return NULL;
}

void SheetNewWindow::on_resultsTabWidget_currentChanged()
{
    // Settling In
    ui->initialCondTW->item(0, ui->initialCondTW->columnCount() - 1)->setText( QString::number(settlingIns[ui->resultsTabWidget->currentIndex()]) );
}

void SheetNewWindow::on_printPB_clicked()
{
    outputSettings = new ExportOutputSettings(-1, tChange, getSheetInfo(), this);
    outputSettings->exec();
    outputSettings->setAttribute(Qt::WA_DeleteOnClose);
}

void SheetNewWindow::on_highlightPB_clicked()
{
    SheetSetup::highlight(getTable(ui->resultsTabWidget->currentIndex()));
}

void SheetNewWindow::on_unhighlightPB_clicked()
{
    SheetSetup::unhighlight(getTable(ui->resultsTabWidget->currentIndex()));
}

void SheetNewWindow::on_unhighlightAllPB_clicked()
{
    SheetSetup::unhighlightAll(getTable(ui->resultsTabWidget->currentIndex()));
}

void SheetNewWindow::on_exportPDFPB_clicked()
{

    outputSettings = new ExportOutputSettings(0, tChange, getSheetInfo(), this);
    outputSettings->exec();
    outputSettings->setAttribute(Qt::WA_DeleteOnClose);
}

void SheetNewWindow::on_exportSpreadsheetPB_clicked()
{
    outputSettings = new ExportOutputSettings(1, tChange, getSheetInfo(), this);
    outputSettings->exec();
    outputSettings->setAttribute(Qt::WA_DeleteOnClose);
}

bool SheetNewWindow::eventFilter(QObject *watched, QEvent *e)
{
    if(e->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* ev = (QMouseEvent*)e;
        if(ev->buttons() == Qt::RightButton)
        {
            QTableWidget *table = getTable(ui->resultsTabWidget->currentIndex());

            if(table->itemAt(ev->x(), ev->y())->isSelected() == false)
            {
                table->clearSelection();
                table->itemAt(ev->x(), ev->y())->setSelected(true);
            }

            showSheetMenu(ev->pos());
        }
    }
    return false;// return true if you are finished handling the event. So, the default event handler will not be called.
}

void SheetNewWindow::keyPressEvent(QKeyEvent *keyevent)
{
    if(keyevent->matches(QKeySequence::Copy) && getTable(ui->resultsTabWidget->currentIndex())->isActiveWindow())
        SheetSetup::copyCells(getTable(ui->resultsTabWidget->currentIndex()));
}

void SheetNewWindow::showSheetMenu(const QPoint& pos)
{
    QTableWidget *table = getTable(ui->resultsTabWidget->currentIndex());
    const QPoint globalPos = table->mapToGlobal(pos);
    table->setContextMenuPolicy(Qt::CustomContextMenu);

    QMenu sheetMenu;
    sheetMenu.addAction("Copy", this, SLOT(copyCells()));
    sheetMenu.exec(globalPos);
}

void SheetNewWindow::copyCells()
{
    SheetSetup::copyCells(getTable(ui->resultsTabWidget->currentIndex()));
}

SheetFormats SheetNewWindow::getSheetInfo()
{
    SheetFormats sheetInfo;
    QStringList tabLabels;
    const int totalTabs = ui->resultsTabWidget->count();
    QVector<QTableWidget*> tables;

    for(int i = 0; i < totalTabs; i++)
    {
        tabLabels.append(ui->resultsTabWidget->tabText(i));
        tables.push_back(getTable(i));
    }

    sheetInfo.setConductorName(ui->initialCondTW->item(0, 0)->text());
    sheetInfo.setInitCondTW(ui->initialCondTW);
    sheetInfo.setSagPrecision(sagPrecision);
    sheetInfo.setSwingPrecision(swingPrecision);
    sheetInfo.setTensionPrecision(tensionPrecision);
    sheetInfo.setActualSpan(ui->initialCondTW->item(0, 6)->text());
    sheetInfo.setActualSpanIndex(actualSpanIndex);
    sheetInfo.setSettlingIns(settlingIns);
    sheetInfo.setResultsTables(tables);
    sheetInfo.setTabLabels(tabLabels);
    sheetInfo.setFontName(ui->initialCondTW->font().toString());
    sheetInfo.setProjectTitle(this->windowTitle());
    sheetInfo.setFontSize(ui->resultsTabWidget->font().pointSize());

    return sheetInfo;
}

void SheetNewWindow::on_clearSelectionPB_clicked()
{
    const int totalTables = ui->resultsTabWidget->count();
    for(int i = 0; i < totalTables; i++)
        getTable(i)->clearSelection();
}

