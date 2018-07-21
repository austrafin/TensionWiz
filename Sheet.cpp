#include "Sheet.h"
#include "ConductorData.h"
#include "SheetSetup.h"
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>

Sheet::Sheet(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Sheet)
{
    ui->setupUi(this);

    this->installEventFilter(this);
    const int tablesTotal = ui->resultsTabWidget->count();
    QTableWidget *table;
    QDoubleValidator *validator = new QDoubleValidator(0, 99999.99999, 5, this);
    QDoubleValidator *validatorTemp = new QDoubleValidator(-99, 999.99999, 5, this);
    validator->setNotation(QDoubleValidator::StandardNotation);
    validatorTemp->setNotation(QDoubleValidator::StandardNotation);
    signalPrevent = true;
    saveConfirmation = false;

    connect(ui->spanLE, SIGNAL(returnPressed()), this, SLOT(on_addSpanPB_clicked()));
    connect(ui->rulingSpanLW, SIGNAL(QEvent::KeyPress), this, SLOT(kPressEvent(key)));
    connect(ui->tensionPrecisionCB, SIGNAL(currentIndexChanged(int)), this, SLOT(calculateTables()));
    connect(ui->sagPrecisionCB, SIGNAL(currentIndexChanged(int)), this, SLOT(calculateTables()));
    connect(ui->swingPrecisionCB, SIGNAL(currentIndexChanged(int)), this, SLOT(calculateTables()));
    connect(ui->fontSB, SIGNAL(valueChanged(int)), this, SLOT(setFont()));
    connect(ui->fontComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(setFont()));
    connect(ui->resultsTabWidget, SIGNAL(currentChanged(int)), this, SLOT(calculateTables()));
    //connect(ui->gravityLE, SIGNAL(textChanged(QString)), this, SLOT(calculateTables()));

    for(int i = 0; i < tablesTotal; i++)
    {
        connect(getSettlingInSpinBox(i), SIGNAL(valueChanged(double)), this, SLOT(calculateTables()));
        connect(getSettlingInCheckBox(i), SIGNAL(stateChanged(int)), this, SLOT(calculateTables()));
    }

    setResultsTabTexts();

    SheetSetup::setTableWidgetItems(ui->initialCondTW);
    SheetSetup::setTables(ui->initialCondTW);

    // Making sure the headers are correctly displayed (glitch in editor).
    ui->initialCondTW->horizontalHeader()->setVisible(true);
    ui->initialCondTW->verticalHeader()->setVisible(false);

    for(int i = 0; i < tablesTotal; i++)
    {
        table = getTable(i);
        SheetSetup::setTableWidgetItems(table);
        SheetSetup::setTables(table);

        // Making sure the headers are correctly displayed (glitch in editor).
        table->horizontalHeader()->setVisible(true);
        table->verticalHeader()->setVisible(true);
        table->viewport()->installEventFilter(this); // Installing event filter for right click menu
    }

    setConductorCategory();

    ui->initTensionLE->setValidator(validator);
    ui->utsLE->setValidator(validator);
    ui->initWindLE->setValidator(validator);
    ui->spanLE->setValidator(validator);
    ui->rulingSpanLE->setValidator(validator);
    ui->initTempLE->setValidator(validatorTemp);
    ui->startValueLE->setValidator(validatorTemp);
    ui->incrementLE->setValidator(validator);
    ui->finalWindLE->setValidator(validator);
    //ui->gravityLE->setValidator(validator);
    ui->startValueLE->setValidator(validator);
    ui->incrementLE->setValidator(validator);
}

Sheet::~Sheet()
{
    delete ui;
}

void Sheet::setResultsTabTexts()
{
    QString wind;

    if(ui->finalWindLE->text() == "")
        wind = "0";
    else
        wind = ui->finalWindLE->text();

    ui->resultsTabWidget->tabBar()->setTabText(3, "T°C+" + wind + "w");
    ui->resultsTabWidget->tabBar()->setTabText(6, "Sags + " + wind + "w");
    ui->resultsTabWidget->tabBar()->setTabText(8, "Swings + " + wind + "w");

    ui->stringingChartTW->horizontalHeaderItem(1)->setText("Tension (N)\n+ " + wind + "Pa Wind");
    ui->stringingChartTW->horizontalHeaderItem(3)->setText("Tension (kg)\n+ " + wind + "Pa Wind");
    ui->stringingChartTW->horizontalHeaderItem(5)->setText("Sag (m)\n+ " + wind + "Pa Wind");

    ui->finalCondT500TW->horizontalHeaderItem(0)->setText("Tension (N)\n+ " + wind + "Pa Wind");
    ui->finalCondT500TW->horizontalHeaderItem(1)->setText("Sag (m)\n+ " + wind + "Pa Wind");
    ui->finalCondT500TW->horizontalHeaderItem(2)->setText("Swing (m)\n+ " + wind + "Pa Wind");
}

void Sheet::setConductorCategory()
{
    const QString currentConductorCat = ui->cdataCategoryCB->currentText();
    ui->cdataCategoryCB->clear();
    ui->cdataCB->clear();    
    ui->cdataCategoryCB->addItem("All");
    ui->cdataCategoryCB->addItems(ConductorData::getConductorCategories());
    ui->cdataCategoryCB->setCurrentText(currentConductorCat);
}

void Sheet::setConductorType()
{
    const QString currentConductor = ui->cdataCB->currentText();

    ui->cdataCB->clear();
    ui->cdataCB->addItems(ConductorData::getConductorNames(ui->cdataCategoryCB->currentText()));
    ui->cdataCB->setCurrentText(currentConductor);
}

void Sheet::calculateTables()
{
    int row = 0;
    int column = 0;
    const int totalRowsSC = ui->stringingChartTW->rowCount();
    const int totalRowsNoWind = ui->finalCondTW->rowCount();
    const int totalRowsInitWind = ui->finalCondInitWindTW->rowCount();
    const int totalRowsT500Wind = ui->finalCondT500TW->rowCount();
    const int tablesTotal = ui->resultsTabWidget->count();
    const double tensionInit = ui->initTensionLE->text().toDouble();
    const QString conductorName = ui->cdataCB->currentText();

    if(CalculateTensionChange::calculateRulingSpan(spans) > 0 && tensionInit > 0 && conductorName != "")
    {
        CalculateTensionChange tChange = getTensionChangeValues();
        QTableWidget *table;
        QVector<double> results;
        QVector<double> resultsTemp;
        const QColor color1(255, 125, 127);
        const QColor color2(255, 238, 42);
        const QStringList conductorDataList = ConductorData::getConductorData();
        const int tensionDecimals = ui->tensionPrecisionCB->currentIndex();
        const int sagDecimals = ui->sagPrecisionCB->currentIndex();
        const int swingDecimals = ui->swingPrecisionCB->currentIndex();
        const int actualSpanIndex = ui->SagSwingCB->currentIndex();
        int index = 0;
        const double uts = ConductorData::getAttributeWithoutUnit(conductorDataList, "$ULTIMATE_TENSILE_STRENGTH", conductorName).toDouble(); // Finding UTS
        char decimalFormat = 'f';

        for(int i = 0; i < tablesTotal; i++)
        {
            table = getTable(i);
            resultsTemp = tChange.getResults(table, actualSpanIndex, i, getSettlingIn(i));

            for(int i2 = 0; i2 < resultsTemp.size(); i2++)
                results.push_back(resultsTemp[i2]);
        }

        // Checking if the user has specified initial temperature. If not, using zero.
        if(ui->initTempLE->text().toDouble() == 0)
            ui->initialCondTW->item(0, 4)->setText("0");
        if(ui->initWindLE->text().toDouble() == 0)
            ui->initialCondTW->item(0, 5)->setText("0");

        for(row = 0; row < totalRowsSC; row++)
        {
            // Setting Tension cells for Stringing Chart
            ui->stringingChartTW->item(row,0)->setText(QString("%1").arg(results[index], 0, decimalFormat, tensionDecimals)); index++;
            ui->stringingChartTW->item(row,1)->setText(QString("%1").arg(results[index], 0, decimalFormat, tensionDecimals)); index++;
            ui->stringingChartTW->item(row,2)->setText(QString("%1").arg(results[index], 0, decimalFormat, tensionDecimals)); index++;
            ui->stringingChartTW->item(row,3)->setText(QString("%1").arg(results[index], 0, decimalFormat, tensionDecimals)); index++;

            // Setting Sag & Swing cells
            ui->stringingChartTW->item(row,4)->setText(QString("%1").arg(results[index], 0, decimalFormat, sagDecimals)); index++;
            ui->stringingChartTW->item(row,5)->setText(QString("%1").arg(results[index], 0, decimalFormat, sagDecimals)); index++;

            // Colouring
            if(ui->stringingChartTW->item(row, 0)->text().toDouble() > uts)
            {
                ui->stringingChartTW->item(row, 0)->setBackgroundColor(color1);
                ui->stringingChartTW->item(row, 2)->setBackgroundColor(color1);
            }
            else if(ui->stringingChartTW->item(row, 0)->text().toDouble() > uts / 2)
            {
                ui->stringingChartTW->item(row, 0)->setBackgroundColor(color2);
                ui->stringingChartTW->item(row, 2)->setBackgroundColor(color2);
            }
            else
            {
                ui->stringingChartTW->item(row, 0)->setBackgroundColor(Qt::white);
                ui->stringingChartTW->item(row, 2)->setBackgroundColor(Qt::white);
            }

            if(ui->stringingChartTW->item(row, 1)->text().toDouble() > uts)
            {
                ui->stringingChartTW->item(row, 1)->setBackgroundColor(color1);
                ui->stringingChartTW->item(row, 3)->setBackgroundColor(color1);
            }
            else if(ui->stringingChartTW->item(row, 1)->text().toDouble() > uts / 2)
            {
                ui->stringingChartTW->item(row, 1)->setBackgroundColor(color2);
                ui->stringingChartTW->item(row, 3)->setBackgroundColor(color2);
            }
            else
            {
                ui->stringingChartTW->item(row, 1)->setBackgroundColor(Qt::white);
                ui->stringingChartTW->item(row, 3)->setBackgroundColor(Qt::white);
            }
        }

        for(row = 0; row < totalRowsNoWind; row++)
        {
            // Setting Tension cells for No Wind
            ui->finalCondTW->item(row,0)->setText(QString("%1").arg(results[index], 0, decimalFormat, tensionDecimals)); index++;

            // Setting Sag  cell.
            ui->finalCondTW->item(row, 1)->setText(QString("%1").arg(results[index], 0, decimalFormat, sagDecimals)); index++;

            // Colouring
            if(ui->finalCondTW->item(row, 0)->text().toDouble() > uts)
                ui->finalCondTW->item(row, 0)->setBackgroundColor(color1);
            else if(ui->finalCondTW->item(row, 0)->text().toDouble() > uts / 2)
                ui->finalCondTW->item(row, 0)->setBackgroundColor(color2);
            else
                ui->finalCondTW->item(row, 0)->setBackgroundColor(Qt::white);
        }

        for(row = 0; row < totalRowsInitWind; row++)
        {
            // Setting Tension cells for Init Wind
            ui->finalCondInitWindTW->item(row,0)->setText(QString("%1").arg(results[index], 0, decimalFormat, tensionDecimals)); index++;

            // Setting Sag & Swing cells. If Actual span not selected, using Ruling Span.
            ui->finalCondInitWindTW->item(row,1)->setText(QString("%1").arg(results[index], 0, decimalFormat, sagDecimals)); index++;
            ui->finalCondInitWindTW->item(row,2)->setText(QString("%1").arg(results[index], 0, decimalFormat, swingDecimals)); index++;

            // Colouring
            if(ui->finalCondInitWindTW->item(row, 0)->text().toDouble() > uts)
                ui->finalCondInitWindTW->item(row, 0)->setBackgroundColor(color1);
            else if(ui->finalCondInitWindTW->item(row, 0)->text().toDouble() > uts / 2)
                ui->finalCondInitWindTW->item(row, 0)->setBackgroundColor(color2);
            else
                ui->finalCondInitWindTW->item(row, 0)->setBackgroundColor(Qt::white);
        }

        for(row = 0; row < totalRowsT500Wind; row++)
        {
            // Setting Tension cells for T500
            ui->finalCondT500TW->item(row,0)->setText(QString("%1").arg(results[index], 0, decimalFormat, tensionDecimals)); index++;

            // Setting Sag & Swing cells.
            ui->finalCondT500TW->item(row,1)->setText(QString("%1").arg(results[index], 0, decimalFormat, sagDecimals)); index++;
            ui->finalCondT500TW->item(row,2)->setText(QString("%1").arg(results[index], 0, decimalFormat, swingDecimals)); index++;

            // Colouring
            if(ui->finalCondT500TW->item(row, 0)->text().toDouble() > uts)
                ui->finalCondT500TW->item(row, 0)->setBackgroundColor(color1);
            else if(ui->finalCondT500TW->item(row, 0)->text().toDouble() > uts / 2)
                ui->finalCondT500TW->item(row, 0)->setBackgroundColor(color2);
            else
                ui->finalCondT500TW->item(row, 0)->setBackgroundColor(Qt::white);
        }

        // Setting sag tabs
        for(int i = 4; i < 7; i++)
        {
            table = getTable(i);

            for(row = 0; row < table->rowCount(); row++)
            {
                for(column = 0; column < table->columnCount(); column++)
                {
                    table->item(row, column)->setText(QString("%1").arg(results[index], 0, decimalFormat, sagDecimals));
                    index++;
                }
            }
        }

        //Setting swing tabs
        for(int i = 7; i < ui->resultsTabWidget->count(); i++)
        {
            table = getTable(i);

            for(row = 0; row < table->rowCount(); row++)
            {
                for(column = 0; column < table->columnCount(); column++)
                {
                    table->item(row, column)->setText(QString("%1").arg(results[index], 0, decimalFormat, swingDecimals));
                    index++;
                }
            }
        }
    }

    else
    {
        QTableWidget *table;
        int totalRows;
        int totalColumns;

        for(int i = 0; i < tablesTotal; i++)
        {
            table = getTable(i);
            totalRows = table->rowCount();
            totalColumns = table->columnCount();

            for(row = 0; row < totalRows; row++)
            {
                for(column = 0; column < totalColumns; column++)
                {
                    table->item(row, column)->setText("");
                    table->item(row, column)->setBackgroundColor(Qt::white);
                }
            }
        }
    }
}

void Sheet::setFont()
{
    QTableWidget *table;
    const QString fontName = ui->fontComboBox->currentText();
    const QFont font(fontName, ui->fontSB->value());
    int row;
    int column;
    int totalRows;
    const int columnInitCond = ui->initialCondTW->columnCount();
    const int tablesTotal = ui->resultsTabWidget->count();
    int totalColumns;

    ui->initialCondTW->horizontalHeader()->setFont(font);
    ui->initialCondTW->horizontalHeaderItem(0)->setFont(font); // Because of glitch

    for(column = 0; column < columnInitCond; column++)
        ui->initialCondTW->item(0, column)->setFont(font);

    for(int i = 0; i < tablesTotal; i++)
    {
        table = getTable(i);
        totalColumns = table->columnCount();
        totalRows = table->rowCount();

        table->horizontalHeader()->setFont(font);
        table->verticalHeader()->setFont(font);

        for(column = 0; column < totalColumns; column++)
        {
            for(row = 0; row < totalRows; row++)
                table->item(row, column)->setFont(font);
        }
    }
}

void Sheet::addSpan(const double span)
{
    if(span > 0)
    {
        const int tablesTotal = ui->resultsTabWidget->count();
        double totalSpanLength = 0;
        QString spanText;
        spanText = "Span ";
        spanText.append(QString("%1").arg(ui->rulingSpanLW->count() + 1));
        spanText.append(" - ");
        spanText.append(QString("%1").arg(span, 0, 'g', 10));
        spanText.append("m");
        ui->rulingSpanLW->addItem(spanText);
        ui->SagSwingCB->addItem(spanText);

        if(ui->SagSwingCB->count() == 2)
            ui->SagSwingCB->setCurrentIndex(1); // If the list only had ruling Span, switching to the new span for convinience.

        spans.push_back(span);

        for(int i = 0; i < spans.size(); i++)
            totalSpanLength += spans[i];

        addSpanColumn(spanText);

        ui->initialCondTW->item(0, 1)->setText(QString("%1").arg(totalSpanLength, 0, 'g', 15));
        ui->initialCondTW->item(0, 2)->setText(QString("%1").arg(CalculateTensionChange::calculateRulingSpan(spans), 0, 'f', 3));

        for(int i = 0; i < tablesTotal; i++)
            SheetSetup::setTables(getTable(i));
    }
}



void Sheet::removeAllRows(QTableWidget *table)
{
    while(table->rowCount() > 0)
    {
        for(int i = 0; i < table->rowCount(); i++)
            table->removeRow(i);
    }
}

bool Sheet::eventFilter(QObject *watched, QEvent *e)
{
    if(e->type() == QEvent::MouseButtonPress)
    {
        saveConfirmation = true;
        QMouseEvent* ev = (QMouseEvent*)e;

        if(ev->buttons() == Qt::RightButton)
        {
            QTableWidget *table = getTable(ui->resultsTabWidget->currentIndex());

            if(table->columnCount() != 0)
            {
                if(table->itemAt(ev->x(), ev->y())->isSelected() == false)
                {
                    table->clearSelection();
                    table->itemAt(ev->x(), ev->y())->setSelected(true);
                }

                showSheetMenu(ev->pos());
            }
        }
    }
    else if(e->type() == QEvent::KeyPress)
        saveConfirmation = true;
    return false;// return true if you are finished handling the event. So, the default event handler will not be called.
}

void Sheet::showSheetMenu(const QPoint& pos)
{
    QTableWidget *table = getTable(ui->resultsTabWidget->currentIndex());
    table->setContextMenuPolicy(Qt::CustomContextMenu);

    const QPoint globalPos = table->mapToGlobal(pos);

    QMenu sheetMenu;
    sheetMenu.addAction("Copy", this, SLOT(copyCells()));
    sheetMenu.exec(globalPos);
}

void Sheet::copyCells()
{
    SheetSetup::copyCells(getTable(ui->resultsTabWidget->currentIndex()));
}

void Sheet::addSpanColumn(const QString &spanText)
{
    QTableWidget *table;
    int totalColumns;
    int totalRows;
    const int totalTabs = ui->resultsTabWidget->count();

    for(int i = 4; i < totalTabs; i++)
    {
        table = getTable(i);
        table->insertColumn(table->columnCount());
        totalColumns = table->columnCount();
        totalRows = table->rowCount();
        table->setHorizontalHeaderItem(totalColumns - 1, new QTableWidgetItem);
        table->horizontalHeaderItem(totalColumns - 1)->setText(spanText);

        for(int row = 0; row < totalRows; row++)
            table->setItem(row, totalColumns - 1, new QTableWidgetItem);
    }
}

void Sheet::removeSpanColumn(const int column)
{
    QTableWidget *table;
    const int totalTabs = ui->resultsTabWidget->count();

    for(int tableIndex = 4; tableIndex < totalTabs; tableIndex++)
    {
        table = getTable(tableIndex);
        table->removeColumn(column);
    }
}

CalculateTensionChange Sheet::getTensionChangeValues()
{
    CalculateTensionChange tChange;
    const QVector<double> conductorAttributes = getConductorAttributes(ui->cdataCB->currentText());

    tChange.setInitialTension(ui->initTensionLE->text().toDouble());
    tChange.setInitialWind(ui->initWindLE->text().toDouble());
    tChange.setFinalWind(ui->finalWindLE->text().toDouble());
    tChange.setInitialTemperature(ui->initTempLE->text().toDouble());
    tChange.setSpans(spans);
    tChange.setInitialTemperature(ui->initTempLE->text().toDouble());
    tChange.setGravity(9.81);

    tChange.setMass(conductorAttributes[0]);
    tChange.setConductorDiameter(conductorAttributes[1]);
    tChange.setCrossSectionalArea(conductorAttributes[2]);
    tChange.setModulusOfElasticy(conductorAttributes[3]);
    tChange.setCoefficientOfExpansion(conductorAttributes[4]);

    return tChange;
}

SheetFormats Sheet::getSheetInfo()
{
    SheetFormats sheetInfo;
    QStringList tabLabels;
    const int totalTabs = ui->resultsTabWidget->count();
    QVector<double> settlingIns;
    QVector<QTableWidget*> tables;

    for(int i = 0; i < totalTabs; i++)
    {
        tabLabels.append(ui->resultsTabWidget->tabText(i));
        settlingIns.push_back(getSettlingIn( i));
        tables.push_back(getTable(i));
    }

    sheetInfo.setConductorName(ui->cdataCB->currentText());
    sheetInfo.setInitCondTW( ui->initialCondTW);
    sheetInfo.setSagPrecision(ui->sagPrecisionCB->currentIndex());
    sheetInfo.setSwingPrecision(ui->swingPrecisionCB->currentIndex());
    sheetInfo.setTensionPrecision(ui->tensionPrecisionCB->currentIndex());
    sheetInfo.setActualSpan(ui->SagSwingCB->currentText());
    sheetInfo.setActualSpanIndex(ui->SagSwingCB->currentIndex());
    sheetInfo.setSettlingIns(settlingIns);
    sheetInfo.setResultsTables(tables);
    sheetInfo.setTabLabels(tabLabels);
    sheetInfo.setFontName(ui->fontComboBox->currentText());
    sheetInfo.setProjectTitle(ui->titleLE->text());
    sheetInfo.setFontSize(ui->fontSB->value());

    return sheetInfo;
}

QVector<double> Sheet::getConductorAttributes(const QString &conductorName) // Finds attributes required for calculations
{
    QVector<double> conductorAttributes;

    const QStringList conductorDataList = ConductorData::getConductorData();

    conductorAttributes.push_back(ConductorData::getAttributeWithoutUnit(conductorDataList, "$MASS", conductorName).toDouble());
    conductorAttributes.push_back(ConductorData::getAttributeWithoutUnit(conductorDataList, "$TOTAL_DIAMETER", conductorName).toDouble());
    conductorAttributes.push_back(ConductorData::getAttributeWithoutUnit(conductorDataList, "$CROSS_SECTIONAL_AREA", conductorName).toDouble());
    conductorAttributes.push_back(ConductorData::getAttributeWithoutUnit(conductorDataList, "$MODULUS_OF_ELASTICITY", conductorName).toDouble());
    conductorAttributes.push_back(ConductorData::getAttributeWithoutUnit(conductorDataList, "$COEFFICIENT_OF_EXPANSION", conductorName).toDouble());

    return conductorAttributes;
}

QTableWidget* Sheet::getTable(int table)
{
    if(table == 0)
        return ui->stringingChartTW;
    else if(table == 1)
        return ui->finalCondTW;
    else if(table == 2)
        return ui->finalCondInitWindTW;
    else if(table == 3)
        return ui->finalCondT500TW;
    else if(table == 4)
        return ui->sagsNoWindTW;
    else if(table == 5)
        return ui->sagsInitWindTW;
    else if(table == 6)
        return ui->sagsT500TW;
    else if(table == 7)
        return ui->swingsInitWindTW;
    else if(table == 8)
        return ui->swingsT500TW;
    else
        return NULL;
}

double Sheet::getSettlingIn(int index)
{
    if(index == 0 && ui->settlingInSCcheckBox->isChecked())
        return ui->settlingInSCSB->value();
    else if(index == 1 && ui->settlingIncheckBox->isChecked())
        return ui->settlingInSB->value();
    else if(index == 2 && ui->settlingInInitWindcheckBox->isChecked())
        return ui->settlingInInitWindSB->value();
    else if(index == 3 && ui->settlingInT500checkBox->isChecked())
        return ui->settlingInT500SB->value();
    else if(index == 4 && ui->settlingInSagsNoWindCheckBox->isChecked())
        return ui->settlingInSagsNoWindSB->value();
    else if(index == 5 && ui->settlingInSagsInitWindCheckBox->isChecked())
        return ui->settlingInSagsInitWindSB->value();
    else if(index == 6 && ui->settlingInSagsT500WindCheckBox->isChecked())
        return ui->settlingInSagsT500SB->value();
    else if(index == 7 && ui->settlingInSwingsInitWindCheckBox->isChecked())
        return ui->settlingInSwingsInitWindSB->value();
    else if(index == 8 && ui->settlingInSwingsT500CheckBox->isChecked())
        return ui->settlingInSwingsT500SB->value();
    else return 0;
}

QDoubleSpinBox* Sheet::getSettlingInSpinBox(int index)
{
    if(index == 0)
        return ui->settlingInSCSB;
    else if(index == 1)
        return ui->settlingInSB;
    else if(index == 2)
        return ui->settlingInInitWindSB;
    else if(index == 3)
        return ui->settlingInT500SB;
    else if(index == 4)
        return ui->settlingInSagsNoWindSB;
    else if(index == 5)
        return ui->settlingInSagsInitWindSB;
    else if(index == 6)
        return ui->settlingInSagsT500SB;
    else if(index == 7)
        return ui->settlingInSwingsInitWindSB;
    else if(index == 8)
        return ui->settlingInSwingsT500SB;
    else return 0;
}

QCheckBox* Sheet::getSettlingInCheckBox(int index)
{
    if(index == 0)
        return ui->settlingInSCcheckBox;
    else if(index == 1)
        return ui->settlingIncheckBox;
    else if(index == 2)
        return ui->settlingInInitWindcheckBox;
    else if(index == 3)
        return ui->settlingInT500checkBox;
    else if(index == 4)
        return ui->settlingInSagsNoWindCheckBox;
    else if(index == 5)
        return ui->settlingInSagsInitWindCheckBox;
    else if(index == 6)
        return ui->settlingInSagsT500WindCheckBox;
    else if(index == 7)
        return ui->settlingInSwingsInitWindCheckBox;
    else if(index == 8)
        return ui->settlingInSwingsT500CheckBox;
    else return 0;
}

bool Sheet::getSettlingInState(int index)
{
    if(index == 0 && ui->settlingInSCcheckBox->isChecked())
        return true;
    else if(index == 1 && ui->settlingIncheckBox->isChecked())
        return true;
    else if(index == 2 && ui->settlingInInitWindcheckBox->isChecked())
        return true;
    else if(index == 3 && ui->settlingInT500checkBox->isChecked())
        return true;
    else if(index == 4 && ui->settlingInSagsNoWindCheckBox->isChecked())
        return true;
    else if(index == 5 && ui->settlingInSagsInitWindCheckBox->isChecked())
        return true;
    else if(index == 6 && ui->settlingInSagsT500WindCheckBox->isChecked())
        return true;
    else if(index == 7 && ui->settlingInSwingsInitWindCheckBox->isChecked())
        return true;
    else if(index == 8 && ui->settlingInSwingsT500CheckBox->isChecked())
        return true;
    else return false;
}

void Sheet::on_addSpanPB_clicked()
{
    if(ui->spanLE->text().toDouble() > 0)
    {
        addSpan(ui->spanLE->text().toDouble());
        ui->spanLE->clear();
        calculateTables();
    }
}

void Sheet::on_resetPB_clicked()
{
    if(spans.size() != 0)
    {
        QTableWidget *table;
        int totalColumns;
        const int tablesTotal = ui->resultsTabWidget->count();

        spans.clear();
        ui->rulingSpanLW->clear();
        signalPrevent = false;
        ui->SagSwingCB->clear();
        signalPrevent = true;
        ui->SagSwingCB->addItem("Ruling Span");
        ui->initialCondTW->item(0, 1)->setText("0");
        ui->initialCondTW->item(0, 2)->setText("0");

        for(int i = 4; i < tablesTotal; i++)
        {
            table = getTable(i);

            while(table->columnCount() > 0)
            {
                totalColumns = table->columnCount();
                for(int column = 0; column < totalColumns; column++)
                    table->removeColumn(column);
            }
        }
    }
}

void Sheet::on_deletePB_clicked()
{
    int totalSpans = spans.size();
    if(spans.size() != 0)
    {
        QTableWidget *table;
        QVector<double> spansTemp;

        const int totalTabs = ui->resultsTabWidget->count();
        QString spanText;
        const QString oldSpan = ui->SagSwingCB->currentText();
        double totalSpanLength = 0;

        for(int i = 0; i < totalSpans; i++)
            spansTemp.push_back(spans[i]);

        spans.clear();

        for(int i = 0; i < ui->rulingSpanLW->count(); i++)
        {
            if(ui->rulingSpanLW->item(i)->isSelected() == false)
                spans.push_back(spansTemp[i]);
            else
                removeSpanColumn(i);
        }

        ui->rulingSpanLW->clear();

        signalPrevent = false;
        ui->SagSwingCB->clear();
        ui->SagSwingCB->addItem("Ruling Span");
        signalPrevent = true;

        totalSpans = spans.size();
        for(int i = 0; i < totalSpans; i++)
        {
            totalSpanLength += spans[i];
            spanText = "Span ";
            spanText.append(QString("%1").arg(ui->rulingSpanLW->count() + 1));
            spanText.append(" - ");
            spanText.append(QString("%1").arg(spans[i]));
            spanText.append("m");
            ui->rulingSpanLW->addItem(spanText);

            ui->SagSwingCB->addItem(spanText);

            for(int tableIndex = 4; tableIndex < totalTabs; tableIndex++)
            {
                table = getTable(tableIndex);
                table->horizontalHeaderItem(i)->setText(spanText);
            }
        }

        ui->SagSwingCB->setCurrentText(oldSpan);

        if(ui->SagSwingCB->currentText() != oldSpan && ui->SagSwingCB->count() > 1)
            ui->SagSwingCB->setCurrentIndex(1);

        ui->initialCondTW->item(0, 1)->setText(QString("%1").arg(totalSpanLength));
        ui->initialCondTW->item(0, 2)->setText(QString("%1").arg(CalculateTensionChange::calculateRulingSpan(spans)));

        calculateTables();
    }
}

void Sheet::on_changePB_clicked()
{
    if(ui->spanLE->text().toDouble() > 0)
    {
        QTableWidget *table;
        double totalSpanLength = 0;
        const int currentSpanIndex = ui->rulingSpanLW->currentIndex().row();

        spans[currentSpanIndex] = ui->spanLE->text().toDouble();
        ui->rulingSpanLW->item(currentSpanIndex)->setText("Span " + QString("%1").arg(currentSpanIndex + 1) + " - " + QString("%1").arg(spans[currentSpanIndex]) + "m");

        for(int i = 0; i < spans.size(); i++)
            totalSpanLength += spans[i];

        signalPrevent = false;
        ui->SagSwingCB->setItemText(currentSpanIndex + 1,"Span " + QString("%1").arg(currentSpanIndex + 1) + " - " + QString("%1").arg(spans[currentSpanIndex]) + "m");
        signalPrevent = true;

        for(int i = 4; i < ui->resultsTabWidget->count(); i++)
        {
            table = getTable(i);
            table->horizontalHeaderItem(currentSpanIndex)->setText(ui->SagSwingCB->itemText(currentSpanIndex + 1));
        }
        ui->initialCondTW->item(0, 1)->setText(QString("%1").arg(totalSpanLength));
        ui->initialCondTW->item(0, 2)->setText(QString("%1").arg(CalculateTensionChange::calculateRulingSpan(spans)));
    }

    calculateTables();
}

void Sheet::on_restorePB_clicked()
{
    QTableWidget *table;
    QString spanText;
    const int spansRestoreTempSize = spansRestoreTemp.size();
    const int tablesTotal = ui->resultsTabWidget->count();
    double totalSpanLength = 0;
    ui->spanLE->setDisabled(false);
    ui->addSpanPB->setDisabled(false);
    ui->deletePB->setDisabled(false);
    ui->changePB->setDisabled(false);
    ui->rulingSpanLW->setDisabled(false);
    ui->resetPB->setDisabled(false);
    ui->SagSwingCB->setDisabled(false);
    ui->restorePB->setEnabled(false);

    spans.clear();

    for(int i = 0; i < spansRestoreTempSize; i++)
    {
        totalSpanLength += spansRestoreTemp[i];
        spans.push_back(spansRestoreTemp[i]);
    }

    spansRestoreTemp.clear();

    removeSpanColumn(0);

    for(int spanIndex = 0; spanIndex < spans.size(); spanIndex++)
    {
        spanText = "Span ";
        spanText.append(QString("%1").arg(spanIndex + 1));
        spanText.append(" - ");
        spanText.append(QString("%1").arg(spans[spanIndex]));
        spanText.append("m");
        addSpanColumn(spanText);
    }

    for(int i = 4; i < tablesTotal; i++)
    {
        table = getTable(i);
        SheetSetup::setTables(table);
    }

    ui->initialCondTW->item(0, 1)->setText(QString("%1").arg(totalSpanLength));
    ui->initialCondTW->item(0, 2)->setText(QString("%1").arg(CalculateTensionChange::calculateRulingSpan(spans)));

    signalPrevent = false;
    ui->rulingSpanLE->clear();
    signalPrevent = true;

    calculateTables();
}

void Sheet::on_addRowsPB_clicked()
{
    if(ui->startValueLE->text() != NULL)
    {
        QTableWidget *table;
        double temp;
        double temperature = ui->startValueLE->text().toDouble();
        double increment = ui->incrementLE->text().toDouble();
        int index = 0;
        int index_2 = 0;
        int tableIndex = 0;
        int tablesTotal = ui->resultsTabWidget->count();
        int row = 0;
        int totalRowsOriginal;
        int column = 0;
        int numberOfRows;
        bool isExisting = false;
        QStringList newValues;

        if(ui->incrementLE->text().toDouble() > 0)
            numberOfRows = ui->numberOfRowsSB->value();
        else
            numberOfRows = 1;

        for(int i = 0; i < numberOfRows; i++)
        {
            newValues.append(QString("%1").arg(temperature, 0, 'g', 10) + "°C");
            temperature += increment;
        }

        if(ui->thisTableRB->isChecked())
        {
            tableIndex = ui->resultsTabWidget->currentIndex();
            tablesTotal = tableIndex + 1;
        }

        for(tableIndex; tableIndex < tablesTotal; tableIndex++)
        {
            table = getTable(tableIndex);

            for(index = 0; index < newValues.size(); index++)
            {
                totalRowsOriginal = table->rowCount();

                for(row = 0; row < totalRowsOriginal; row++)
                {
                    if(newValues[index] == table->verticalHeaderItem(row)->text())
                        isExisting = true;
                }

                if(isExisting == false)
                {
                    temp = SheetSetup::getTemperatureFromString(newValues[index]);
                    while(index_2 < table->rowCount() && temp > SheetSetup::getTemperatureFromString(table->verticalHeaderItem(index_2)->text()))
                        index_2++;
                    //index_2++;

                    table->insertRow(index_2);
                    table->setVerticalHeaderItem(index_2, new QTableWidgetItem);
                    table->verticalHeaderItem(index_2)->setText(newValues[index]);

                    for(column = 0; column < table->columnCount(); column++)
                        table->setItem(index_2, column, new QTableWidgetItem);

                    index_2 = 0;
                }
                isExisting = false;
             }
        }
        calculateTables();
        for(int i = 0; i < tablesTotal; i++)
            SheetSetup::setTables(getTable(i));
    }
}

void Sheet::on_deleteRowPB_clicked()
{
    QTableWidget *table;
    QStringList toBeRemoved;
    int row;
    int tableIndex = 0;
    int rowToBeRemoved;
    int tablesTotal = ui->resultsTabWidget->count();
    int totalColumns;
    int totalRows;
    bool deleteRow = false;

    if(ui->thisTableRB->isChecked())
    {
        tableIndex = ui->resultsTabWidget->currentIndex();
        tablesTotal = tableIndex + 1;
    }

    table = getTable(ui->resultsTabWidget->currentIndex());
    totalColumns = table->columnCount();
    totalRows = table->rowCount();

    for(int row = 0; row < totalRows; row++)
    {
        for(int column = 0; column < totalColumns; column++)
        {
            if(table->item(row, column)->isSelected() == true)
                toBeRemoved.append(table->verticalHeaderItem(row)->text()); // Saving the rows to be deleted in a StringList
        }
    }

    for(tableIndex; tableIndex < tablesTotal; tableIndex++)
    {
        table = getTable(tableIndex);
        totalRows = table->rowCount();

        for(int i = 0; i < toBeRemoved.size(); i++)
        {
            totalRows = table->rowCount();
            for(row = 0; row < totalRows; row++)
            {
                if(table->verticalHeaderItem(row)->text() == toBeRemoved[i])
                {
                    rowToBeRemoved = row;
                    deleteRow = true;
                }
            }

            if(deleteRow == true)
                table->removeRow(rowToBeRemoved);
            deleteRow = false;
        }

    }

    for(int i = 0; i < ui->resultsTabWidget->count(); i++)
        SheetSetup::setTables(getTable(i));
}

void Sheet::on_clearTablePB_clicked()
{
    QTableWidget *table = getTable(ui->resultsTabWidget->currentIndex());
    QMessageBox mbox;
    mbox.setIcon(QMessageBox::Warning);

    mbox.setWindowTitle("Clear Table(s)");
    mbox.setText( "This action cannot be undone. Are you sure you want to continue?" );
    mbox.setStandardButtons( QMessageBox::YesAll | QMessageBox::Yes);
    mbox.setButtonText(QMessageBox::YesAll, "Yes All Tables");
    mbox.setButtonText(QMessageBox::Yes, "Yes This Table");
    mbox.setDefaultButton( mbox.addButton( QMessageBox::Cancel ) );

    switch (mbox.exec())
    {
        case QMessageBox::Yes:
            removeAllRows(table);
            break;
        case QMessageBox::YesAll:
            for(int i = 0; i < ui->resultsTabWidget->tabBar()->count(); i++)
            {
                table = getTable(i);
                removeAllRows(table);
            }
            break;

        case QMessageBox::Cancel:
            break;

    }
}

void Sheet::on_highlightPB_clicked()
{
    SheetSetup::highlight(getTable(ui->resultsTabWidget->currentIndex()));
}

void Sheet::on_unhighlightPB_clicked()
{
    SheetSetup::unhighlight(getTable(ui->resultsTabWidget->currentIndex()));
}

void Sheet::on_unhighlightAllPB_clicked()
{
    SheetSetup::unhighlightAll(getTable(ui->resultsTabWidget->currentIndex()));
}

void Sheet::on_printPB_clicked()
{
    outputSettings = new ExportOutputSettings(-1, getTensionChangeValues(), getSheetInfo(), this);
    outputSettings->exec();
    outputSettings->setAttribute(Qt::WA_DeleteOnClose);
}

void Sheet::on_exportPDFPB_clicked()
{

    outputSettings = new ExportOutputSettings(0, getTensionChangeValues(), getSheetInfo(), this);
    outputSettings->exec();
    outputSettings->setAttribute(Qt::WA_DeleteOnClose);
}

void Sheet::on_exportXLSPB_clicked()
{
    exportSpreadSheet();
}

void Sheet::exportSpreadSheet(int fileType)
{
    outputSettings = new ExportOutputSettings(fileType, getTensionChangeValues(), getSheetInfo(), this);
    outputSettings->exec();
    outputSettings->setAttribute(Qt::WA_DeleteOnClose);
}

void Sheet::on_initTensionLE_textChanged(const QString &arg1)
{
    if(signalPrevent == true)
    {
        const double uts =  ConductorData::getAttributeWithoutUnit(ConductorData::getConductorData(), "$ULTIMATE_TENSILE_STRENGTH", ui->cdataCB->currentText()).toDouble();

        if(uts > 0)
        {

            signalPrevent = false;
            ui->utsLE->setText(QString("%1").arg((arg1.toDouble() / uts * 100), 0, 'g', 9));
            signalPrevent = true;
        }

        ui->initialCondTW->item(0, 3)->setText(arg1);

        calculateTables();
    }
}

void Sheet::on_utsLE_textChanged(const QString &arg1)
{
    if(signalPrevent == true)
    {
        const double uts =  ConductorData::getAttributeWithoutUnit(ConductorData::getConductorData(), "$ULTIMATE_TENSILE_STRENGTH", ui->cdataCB->currentText()).toDouble();
        if(uts > 0)
        {
            signalPrevent = false;

            ui->initTensionLE->setText( QString("%1").arg((uts / 100 * arg1.toDouble()), 0, 'g', 9) );
            signalPrevent = true;
        }

        calculateTables();
    }
}

void Sheet::on_initTempLE_textChanged(const QString &arg1)
{
    ui->initialCondTW->item(0, 4)->setText(arg1);
    calculateTables();
}

void Sheet::on_initWindLE_textChanged(const QString &arg1)
{
    ui->initialCondTW->item(0, 5)->setText(arg1);
    calculateTables();
}

void Sheet::on_finalWindLE_textChanged()
{
    setResultsTabTexts();
    calculateTables();
}

void Sheet::on_rulingSpanLE_textChanged(const QString &arg1)
{
    if(signalPrevent == true)
    {
        if(ui->restorePB->isEnabled() == false) // This only has to be once. Making sure that spansRestoreTemp doesn't get the new Ruling Span value.
        {
            for(int i = 0; i < spans.size(); i++)
                spansRestoreTemp.push_back(spans[i]);

            setActualSpanRulingSpanOnly();
        }
        spans.clear();
        spans.push_back(ui->rulingSpanLE->text().toDouble());
        ui->initialCondTW->item(0, 2)->setText(arg1);

        calculateTables();
    }
}

void Sheet::setActualSpanRulingSpanOnly()
{
    QTableWidget *table = getTable(4);

    ui->spanLE->setDisabled(true);
    ui->addSpanPB->setDisabled(true);
    ui->deletePB->setDisabled(true);
    ui->changePB->setDisabled(true);
    ui->rulingSpanLW->setDisabled(true);
    ui->resetPB->setDisabled(true);
    ui->SagSwingCB->setDisabled(true);
    ui->restorePB->setEnabled(true);

    ui->initialCondTW->item(0, 1)->setText("N/A");

    while(table->columnCount() != 0)
        removeSpanColumn(0);
    addSpanColumn("Ruling Span");
    table->horizontalHeaderItem(0)->setText("Ruling Span");
}

void Sheet::on_SagSwingCB_currentTextChanged()
{
    if(signalPrevent == true)
        calculateTables();
}

void Sheet::on_cdataCategoryCB_currentTextChanged()
{
    setConductorType();
}

void Sheet::on_cdataCB_currentTextChanged(const QString &arg1)
{
    ui->initialCondTW->item(0, 0)->setText(arg1);
    if(ui->initTensionLE->text() != "")
    {
        if(arg1 != "")
            on_initTensionLE_textChanged(ui->initTensionLE->text());
    }
}

void Sheet::on_newWindowPB_clicked()
{
    newWindow = new SheetNewWindow(getTensionChangeValues(), getSheetInfo());
    newWindow->setAttribute(Qt::WA_DeleteOnClose);
    newWindow->show();
}

void Sheet::keyPressEvent(QKeyEvent *keyevent)
{
    if((keyevent->key() == Qt::Key_Delete) && ui->rulingSpanLW->isActiveWindow())
        on_deletePB_clicked();
    else if(keyevent->matches(QKeySequence::Copy) && getTable(ui->resultsTabWidget->currentIndex())->isActiveWindow())
        SheetSetup::copyCells(getTable(ui->resultsTabWidget->currentIndex()));
}

void Sheet::on_clearSelectionPB_clicked()
{
    if(ui->allTablesRB->isChecked())
    {
        const int totalTables = ui->resultsTabWidget->count();
        for(int i = 0; i < totalTables; i++)
            getTable(i)->clearSelection();
    }
    else
        getTable(ui->resultsTabWidget->currentIndex())->clearSelection();
}
