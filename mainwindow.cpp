#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "SheetSetup.h"
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QKeyEvent>
#include <QInputDialog>

MainWindow::MainWindow(const QString &filePath, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    int index = filePath.size() - 1;
    QString fileExtension;
    signalPrevent = true;
    saveConfirmation = false;
    isMouseReleased = true;
    ui->setupUi(this);

    this->setWindowIcon(QIcon(":/Images/icon.ico"));
    this->setWindowTitle("TensionWiz");

    connect(ui->actionExit, SIGNAL(triggered()), SLOT(close()));
    connect(ui->actionPrint, SIGNAL(triggered()), SLOT(close()));
    connect(ui->sheetsTabWidget->tabBar(), SIGNAL(tabMoved(int,int)), SLOT(tabOrderChanged(int,int)));

    ui->sheetsTabWidget->tabBar()->installEventFilter(this);
    ui->sheetsTabWidget->tabBar()->setMouseTracking(true);
    ui->sheetsTabWidget->setMovable(true);
    ui->sheetsTabWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    if(filePath != "")
    {
        while(filePath[index] != '.')
        {
            fileExtension.push_front(filePath[index]);
            index--;
        }

        if(fileExtension == "twz")
            openFile(filePath);
        else if(fileExtension == "cond") // TensionWiz conductor file extension
        {
            insertSheet();
            this->setWindowTitle("Untitled - TensionWiz");

            const QMessageBox::StandardButtons mBox = QMessageBox::question(this, "Import file", "Do you want to import file?", QMessageBox::Yes|QMessageBox::No);

            if(mBox == QMessageBox::Yes)
            {
                QStringList files;
                files.append(filePath);
                library = new ConductorLibrary(this);
                library->setAttribute(Qt::WA_DeleteOnClose);
                library->importConductors(files);
                library->exec();

                // Sending signal to Sheet to update Conductor list.
                for(int i = 0; i < sheets.size(); i++)
                {
                    sheets[i]->setConductorCategory();
                    sheets[i]->setConductorType();
                }
            }

        }
        else
        {
            QMessageBox::critical(this, "File not supported", "This file is corrupted or not supported by TensionWiz");
            QApplication::quit();
        }
    }
    else
    {
        insertSheet();
        this->setWindowTitle("Untitled - TensionWiz");
    }
    QWidget::showMaximized();
}

MainWindow::~MainWindow()
{
    delete ui;

    if(twzFile.isOpen())
        twzFile.close();
}

void MainWindow::closeEvent ( QCloseEvent* event )
{
    event->ignore();

    const int totalSheets = sheets.size();

    for(int i = 0; i < totalSheets; i++)
    {
        if(sheets[i]->saveConfirmation == true)
        {
            if(closeFile() == true)
                qApp->exit();
            else
                return;
        }
    }

    if(saveConfirmation == false)
        qApp->exit();
    else if(closeFile() == true)
        qApp->exit();
}

void MainWindow::on_sheetsTabWidget_currentChanged(const int index)
{
    if(signalPrevent == true)
    {
        if(index == ui->sheetsTabWidget->count() - 1)
            insertSheet();
    }
}

void MainWindow::insertSheet()
{
    // Creating new sheet and storing it in a vector for future example when updating sheets.
    sheetWidgets.push_back(new QWidget);
    sheets.push_back(new Sheet);

    const int sheetsIndex = sheets.size() - 1;

    sheets[sheetsIndex]->setParent(sheetWidgets[sheetWidgets.size() - 1]);

    ui->sheetsTabWidget->insertTab(ui->sheetsTabWidget->count() - 1, sheets[sheetsIndex], "New Sheet " + QString::number(ui->sheetsTabWidget->count()));
    ui->sheetsTabWidget->setCurrentIndex(ui->sheetsTabWidget->count() - 2);
}

void MainWindow::deleteSheet()
{
    QVector<Sheet*> sheetsTemp;
    QVector<QWidget*> sheetWidgetsTemp;
    signalPrevent = false;

    for(int i = 0; i < sheets.size(); i++)
    {
        if(i != ui->sheetsTabWidget->currentIndex())
        {
            sheetsTemp.push_back(sheets[i]);
            sheetWidgetsTemp.push_back(sheetWidgets[i]);
        }
        else
        {
            delete sheets[i];
            delete sheetWidgets[i];
        }
    }

    sheets.clear();
    sheetWidgets.clear();

    for (int i = 0; i < sheetsTemp.size(); i++)
    {
        sheets.push_back(sheetsTemp[i]);
        sheetWidgets.push_back(sheetWidgetsTemp[i]);
    }

    ui->sheetsTabWidget->setCurrentIndex(0);
    signalPrevent = true;
}

void MainWindow::renameSheet()
{
    bool ok = true;
    const int tabIndex = ui->sheetsTabWidget->tabBar()->tabAt(globalPoint);
    const QString oldName = ui->sheetsTabWidget->tabBar()->tabText(tabIndex);
    const QString newName = QInputDialog::getText(this, tr("Change Name"), tr("Insert new tab Name"), QLineEdit::Normal, oldName, &ok);

    if(ok)
        ui->sheetsTabWidget->setTabText(tabIndex, newName);
}

void MainWindow::showSheetMenu(const QPoint& pos)
{
    const QPoint globalPos = ui->sheetsTabWidget->tabBar()->mapToGlobal(pos);
    globalPoint = pos;
    QMenu sheetMenu;
    QAction *del = new QAction(this);
    del->deleteLater();
    del->setText("Delete");
    connect(del, SIGNAL(triggered()), this, SLOT(deleteSheet()));

    if(ui->sheetsTabWidget->count() < 3)
        del->setDisabled(true);

    sheetMenu.addAction("Insert...", this, SLOT(insertSheet()));
    sheetMenu.addAction("Dublicate", this, SLOT(copySheet()));
    sheetMenu.addAction(del);
    sheetMenu.addAction("Rename", this, SLOT(renameSheet()));

    sheetMenu.exec(globalPos);
}

void MainWindow::on_actionSave_triggered()
{
    const QString fileName = twzFile.fileName();

    if(fileName != "")
        saveFile(twzFile.fileName());
    else
        on_actionSave_As_triggered();
}

void MainWindow::on_actionSave_As_triggered()
{
    QFileDialog fileDialog;
    const QString previousPath = SheetSetup::getPreviousPath("$PREVIOUS_TWZ_FILEPATH");
    QString directory;

    if(twzFile.fileName() != "")
        directory = previousPath + getFileName(twzFile.fileName());
    else
        directory = previousPath + "Untitled";

    const QString fileName = fileDialog.getSaveFileName(this, QObject::tr("Save As"), directory, "TensionWiz File (*.twz)");
    if(fileName != "")
        saveFile(fileName);
}

void MainWindow::on_actionOpen_triggered()
{
    SheetSetup::setPreviousFilePath(twzFile.fileName(), "$PREVIOUS_TWZ_FILEPATH");
    const QString fileName = QFileDialog::getOpenFileName(this, QObject::tr("Open File"), SheetSetup::getPreviousPath("$PREVIOUS_TWZ_FILEPATH"), "TensionWiz File (*.twz)");
    if(fileName != "")
    {
        if(closeFile() == true)
        {
            openFile(fileName);
            saveConfirmation = false;
        }
    }
}

bool MainWindow::closeFile()
{
    bool close = true;
    QMessageBox mbox;
    mbox.setIcon(QMessageBox::Question);
    mbox.setWindowTitle("TensionWiz");
    mbox.setText( "Do you want to save changes to " + getFileName(twzFile.fileName()) + "?" );
    mbox.setStandardButtons( QMessageBox::Yes | QMessageBox::No);
    //mbox.setButtonText(QMessageBox::Yes, "Yes All Tables");
    //mbox.setButtonText(QMessageBox::No, "Yes This Table");
    mbox.setDefaultButton( mbox.addButton( QMessageBox::Cancel ) );

    switch (mbox.exec())
    {
        case QMessageBox::Yes:
            if(twzFile.fileName() == "")
                on_actionSave_As_triggered();
            else
                saveFile(twzFile.fileName());
            break;

        case QMessageBox::No:
            break;

        case QMessageBox::Cancel:
            close = false;
            break;
    }

    return close;
}

void MainWindow::saveFile(const QString &filePath)
{
    const int totalTabs = ui->sheetsTabWidget->count() - 1;
    twzFile.close();
    twzFile.setFileName(filePath);
    QTextStream textStream(&twzFile);
    QStringList fileSL;
    QStringList values;
    SheetSetup::setPreviousFilePath(filePath, "$PREVIOUS_TWZ_FILEPATH");

    twzFile.open(QFile::ReadOnly);
    while(!textStream.atEnd())
        fileSL.append(textStream.readLine());
    twzFile.close();

    twzFile.open(QFile::ReadWrite);

    for(int i = 0; i < totalTabs; i++)
        values.append(ui->sheetsTabWidget->tabText(i));
    replaceValues(values, "$SHEETS", fileSL);
    values.clear();

    for(int i = 0; i < totalTabs; i++)
        values.append(sheets[i]->ui->titleLE->text() + "$" + QString::number(i));
    replaceValues(values, "$PROJECT_TITLES", fileSL);
    values.clear();

    for(int i = 0; i < totalTabs; i++)
        values.append(sheets[i]->ui->initTensionLE->text() + "$" + QString::number(i));
    replaceValues(values, "$INITIAL_TENSIONS", fileSL);
    values.clear();

    for(int i = 0; i < totalTabs; i++)
        values.append(sheets[i]->ui->initTempLE->text() + "$" + QString::number(i));
    replaceValues(values, "$INITIAL_TEMPERATURES", fileSL);
    values.clear();

    for(int i = 0; i < totalTabs; i++)
        values.append(sheets[i]->ui->initWindLE->text() + "$" + QString::number(i));
    replaceValues(values, "$INITIAL_WINDS", fileSL);
    values.clear();

    for(int i = 0; i < totalTabs; i++)
        values.append(sheets[i]->ui->finalWindLE->text() + "$" + QString::number(i));
    replaceValues(values, "$FINAL_WINDS", fileSL);
    values.clear();

//    for(int i = 0; i < totalTabs; i++)
//        values.append(sheets[i]->ui->gravityLE->text() + "$" + QString::number(i));
//    replaceValues(values, "$GRAVITY", fileSL);
//    values.clear();

    for(int i = 0; i < totalTabs; i++)
        values.append(sheets[i]->ui->cdataCategoryCB->currentText() + "$" + QString::number(i));
    replaceValues(values, "$CONDUCTOR_CATEGORIES", fileSL);
    values.clear();

    for(int i = 0; i < totalTabs; i++)
        values.append(sheets[i]->ui->cdataCB->currentText() + "$" + QString::number(i));
    replaceValues(values, "$CONDUCTOR_NAMES", fileSL);
    values.clear();

    for(int i = 0; i < totalTabs; i++)
        values.append(QString::number(sheets[i]->ui->tensionPrecisionCB->currentIndex()) + "$" + QString::number(i));
    replaceValues(values, "$TENSION_PRECISIONS", fileSL);
    values.clear();

    for(int i = 0; i < totalTabs; i++)
        values.append(QString::number(sheets[i]->ui->sagPrecisionCB->currentIndex()) + "$" + QString::number(i));
    replaceValues(values, "$SAG_PRECISIONS", fileSL);
    values.clear();

    for(int i = 0; i < totalTabs; i++)
        values.append(QString::number(sheets[i]->ui->swingPrecisionCB->currentIndex()) + "$" + QString::number(i));
    replaceValues(values, "$SWING_PRECISIONS", fileSL);
    values.clear();

    for(int i = 0; i < totalTabs; i++)
        values.append(sheets[i]->ui->fontComboBox->currentText() + "$" + QString::number(i));
    replaceValues(values, "$FONT_NAME", fileSL);
    values.clear();

    for(int i = 0; i < totalTabs; i++)
        values.append(QString::number(sheets[i]->ui->fontSB->value()) + "$" + QString::number(i));
    replaceValues(values, "$FONT_SIZE", fileSL);
    values.clear();

    for(int i = 0; i < totalTabs; i++)
        values.append(sheets[i]->ui->rulingSpanLE->text() + "$" + QString::number(i));
    replaceValues(values, "$RULING_SPAN", fileSL);
    values.clear();

    for(int i = 0; i < totalTabs; i++)
        values.append(sheets[i]->ui->startValueLE->text() + "$" + QString::number(i));
    replaceValues(values, "$ROW_START_VALUE", fileSL);
    values.clear();

    for(int i = 0; i < totalTabs; i++)
        values.append(sheets[i]->ui->incrementLE->text() + "$" + QString::number(i));
    replaceValues(values, "$ROW_INCREMENT", fileSL);
    values.clear();

    for(int i = 0; i < totalTabs; i++)
        values.append(QString::number(sheets[i]->ui->numberOfRowsSB->value()) + "$" + QString::number(i));
    replaceValues(values, "$NUMBER_OF_ROWS", fileSL);
    values.clear();

    for(int i = 0; i < totalTabs; i++)
    {
        if(sheets[i]->ui->allTablesRB->isChecked())
            values.append(QString::number(1) + "$" + QString::number(i));
        else
            values.append(QString::number(0) + "$" + QString::number(i));
    }
    replaceValues(values, "$ROWS_ALL_TABLES_STATE", fileSL);
    values.clear();

    for(int i = 0; i < totalTabs; i++)
    {
        for(int spanIndex = 0; spanIndex < sheets[i]->spans.size(); spanIndex++)
            values.append(QString::number(sheets[i]->spans[spanIndex]) + "$" + QString::number(i));
    }
    replaceValues(values, "$SPANS", fileSL);
    values.clear();

    for(int i = 0; i < totalTabs; i++)
    {
        for(int spanIndex = 0; spanIndex < sheets[i]->spansRestoreTemp.size(); spanIndex++)
            values.append(QString::number(sheets[i]->spansRestoreTemp[spanIndex]) + "$" + QString::number(i));
    }
    replaceValues(values, "$SPANS_RESTORES", fileSL);
    values.clear();

    for(int i = 0; i < totalTabs; i++)
    {
        for(int settlingInIndex = 0; settlingInIndex < sheets[i]->ui->resultsTabWidget->count(); settlingInIndex++)
            values.append(QString::number(sheets[i]->getSettlingIn(settlingInIndex)) + "$" + QString::number(i));
    }
    replaceValues(values, "$SETTLING_IN_VALUES", fileSL);
    values.clear();

    for(int i = 0; i < totalTabs; i++)
    {
        for(int settlingInIndex = 0; settlingInIndex < sheets[i]->ui->resultsTabWidget->count(); settlingInIndex++)
            values.append(QString::number(sheets[i]->getSettlingInState(settlingInIndex)) + "$" + QString::number(i));
    }
    replaceValues(values, "$SETTLING_IN_STATES", fileSL);
    values.clear();

    for(int i = 0; i < totalTabs; i++)
    {
        for(int table = 0; table < sheets[i]->ui->resultsTabWidget->count(); table++)
        {
            for(int row = 0; row < sheets[i]->getTable(table)->rowCount(); row++)
                values.append(sheets[i]->getTable(table)->verticalHeaderItem(row)->text() + "$" + QString::number(i));
        }
    }
    replaceValues(values, "$ROW_TEXTS", fileSL);
    values.clear();

    for(int i = 0; i < totalTabs; i++)
    {
        for(int table = 0; table < sheets[i]->ui->resultsTabWidget->count(); table++)
            values.append(QString::number(sheets[i]->getTable(table)->rowCount())); // No need for sheet number here
    }
    replaceValues(values, "$TOTAL_ROWS", fileSL);
    values.clear();

    for(int i = 0; i < fileSL.size(); i++)
    {
        textStream << fileSL[i];
        textStream << "\n";
    }
    this->setWindowTitle(getFileName(filePath) + " - TensionWiz");

    for(int i = 0; i < totalTabs; i++)
        sheets[i]->saveConfirmation = false;
    saveConfirmation = false;
}

void MainWindow::openFile(const QString &filePath)
{
    twzFile.close();
    twzFile.setFileName(filePath);
    SheetSetup::setPreviousFilePath(filePath, "$PREVIOUS_TWZ_FILEPATH");

    int totalTables, index = 0, count = 0, valuesCount = 0, tempIndex = 0, totalSheets = sheets.size();
    double totalSpansLength;
    QString temp;
    QString spanText;
    QStringList fileSL, attributes, values;
    QVector<int> sheetNumbers, numberOfValues, totalRows;
    QTextStream textStream(&twzFile);
    QTableWidget *table;

    signalPrevent = false;
    for(int i = 0; i < totalSheets; i++)
    {
        delete sheets[i];
        delete sheetWidgets[i];
    }
    sheets.clear();
    sheetWidgets.clear();
    signalPrevent = true;

    twzFile.open(QFile::ReadWrite);

    while(!textStream.atEnd())
        fileSL.append(textStream.readLine());

    while(fileSL[index] != "$SHEETS" && index < fileSL.size())
        index++;
    index++;

    while(fileSL[index] != "$ATTRIBUTE_END" && index < fileSL.size())
    {
        insertSheet();
        ui->sheetsTabWidget->setTabText(ui->sheetsTabWidget->count() - 2, fileSL[index]);
        totalTables = sheets[sheets.size() - 1]->ui->resultsTabWidget->count();

        for(int i = 0; i < totalTables; i++)
            sheets[sheets.size() - 1]->removeAllRows(sheets[sheets.size() - 1]->getTable(i));
        index++;
    }
    index = 0;

    totalSheets = sheets.size();

    while(fileSL[index] != "$TOTAL_ROWS" && index < fileSL.size())
        index++;
    index++;

    while(fileSL[index] != "$ATTRIBUTE_END" && index < fileSL.size())
    {
        totalRows.push_back(fileSL[index].toInt());
        index++;
    }
    index = 0;

    attributes.append("$PROJECT_TITLES");
    attributes.append("$INITIAL_TENSIONS");
    attributes.append("$INITIAL_TEMPERATURES");
    attributes.append("$INITIAL_WINDS");
    attributes.append("$FINAL_WINDS");
    //attributes.append("$GRAVITY");
    attributes.append("$CONDUCTOR_CATEGORIES");
    attributes.append("$CONDUCTOR_NAMES");
    attributes.append("$TENSION_PRECISIONS");
    attributes.append("$SAG_PRECISIONS");
    attributes.append("$SWING_PRECISIONS");
    attributes.append("$FONT_NAME");
    attributes.append("$FONT_SIZE");
    attributes.append("$RULING_SPAN");
    attributes.append("$ROW_START_VALUE");
    attributes.append("$ROW_INCREMENT");
    attributes.append("$NUMBER_OF_ROWS");
    attributes.append("$ROWS_ALL_TABLES_STATE");
    attributes.append("$SPANS");
    attributes.append("$SPANS_RESTORES");
    attributes.append("$SETTLING_IN_VALUES");
    attributes.append("$SETTLING_IN_STATES");
    attributes.append("$ROW_TEXTS");

    // Getting all the values from file and storing them in a stringlist
    for(int i = 0; i < attributes.size(); i++)
    {
        while(fileSL[index] != attributes[i] && index < fileSL.size())
            index++;
        index++;

        if(fileSL[index] == "$ATTRIBUTE_END")
            numberOfValues.push_back(0);
        else
        {
            while(fileSL[index] != "$ATTRIBUTE_END" && index < fileSL.size())
            {
                valuesCount++;
                while(fileSL[index][tempIndex] != '$')
                {
                    temp.append(fileSL[index][tempIndex]);
                    tempIndex++;
                }
                tempIndex++;

                values.append(temp);
                temp.clear();

                for(int i2 = tempIndex; i2 < fileSL[index].size(); i2++)
                    temp.append(fileSL[index][tempIndex]);

                sheetNumbers.push_back(temp.toInt());

                temp.clear();
                tempIndex = 0;
                index++;
            }

            numberOfValues.push_back(valuesCount);
            valuesCount = 0;
        }
        index = 0;
    }

    for(int i = 0; i < numberOfValues[index]; i++)
    {
        sheets[sheetNumbers[count]]->ui->titleLE->setText(values[count]);
        count++;
    }
    index++;

    for(int i = 0; i < numberOfValues[index]; i++)
    {
        sheets[sheetNumbers[count]]->ui->initTensionLE->setText(values[count]);
        count++;
    }
    index++;

    for(int i = 0; i < numberOfValues[index]; i++)
    {
        sheets[sheetNumbers[count]]->ui->initTempLE->setText(values[count]);
        count++;
    }
    index++;

    for(int i = 0; i < numberOfValues[index]; i++)
    {
        sheets[sheetNumbers[count]]->ui->initWindLE->setText(values[count]);
        count++;
    }
    index++;

    for(int i = 0; i < numberOfValues[index]; i++)
    {
        sheets[sheetNumbers[count]]->ui->finalWindLE->setText(values[count]);
        count++;
    }
    index++;

//    for(int i = 0; i < numberOfValues[index]; i++)
//    {
//        sheets[sheetNumbers[count]]->ui->gravityLE->setText(values[count]);
//        count++;
//    }
//    index++;

    for(int i = 0; i < numberOfValues[index]; i++)
    {
        sheets[sheetNumbers[count]]->ui->cdataCategoryCB->setCurrentText(values[count]);
        count++;
    }
    index++;

    for(int i = 0; i < numberOfValues[index]; i++)
    {
        sheets[sheetNumbers[count]]->ui->cdataCB->setCurrentText(values[count]);
        count++;
    }
    index++;

    for(int i = 0; i < numberOfValues[index]; i++)
    {
        sheets[sheetNumbers[count]]->ui->tensionPrecisionCB->setCurrentIndex(values[count].toInt());
        count++;
    }
    index++;

    for(int i = 0; i < numberOfValues[index]; i++)
    {
        sheets[sheetNumbers[count]]->ui->sagPrecisionCB->setCurrentIndex(values[count].toInt());
        count++;
    }
    index++;

    for(int i = 0; i < numberOfValues[index]; i++)
    {
        sheets[sheetNumbers[count]]->ui->swingPrecisionCB->setCurrentIndex(values[count].toInt());
        count++;
    }
    index++;

    for(int i = 0; i < numberOfValues[index]; i++)
    {
        sheets[sheetNumbers[count]]->ui->fontComboBox->setCurrentText(values[count]);
        count++;
    }
    index++;

    for(int i = 0; i < numberOfValues[index]; i++)
    {
        sheets[sheetNumbers[count]]->ui->fontSB->setValue(values[count].toInt());
        count++;
    }
    index++;

    for(int i = 0; i < numberOfValues[index]; i++)
    {
        sheets[sheetNumbers[count]]->ui->rulingSpanLE->setText(values[count]);
        count++;
    }
    index++;

    for(int i = 0; i < numberOfValues[index]; i++)
    {
        sheets[sheetNumbers[count]]->ui->startValueLE->setText(values[count]);
        count++;
    }
    index++;

    for(int i = 0; i < numberOfValues[index]; i++)
    {
        sheets[sheetNumbers[count]]->ui->incrementLE->setText(values[count]);
        count++;
    }
    index++;

    for(int i = 0; i < numberOfValues[index]; i++)
    {
        sheets[sheetNumbers[count]]->ui->numberOfRowsSB->setValue(values[count].toInt());
        count++;
    }
    index++;

    for(int i = 0; i < numberOfValues[index]; i++)
    {
        sheets[sheetNumbers[count]]->ui->allTablesRB->setChecked(values[count].toInt());
        count++;
    }
    index++;

    for(int i = 0; i < numberOfValues[index]; i++)
    {
        spanText = "Span " + QString::number(i + 1) + " - " + values[count] + "m";
        sheets[sheetNumbers[count]]->spans.push_back(values[count].toDouble());
        sheets[sheetNumbers[count]]->ui->rulingSpanLW->addItem(spanText);
        sheets[sheetNumbers[count]]->ui->SagSwingCB->addItem(spanText);
        sheets[sheetNumbers[count]]->addSpanColumn(spanText);
        count++;
    }
    index++;

    for(int i = 0; i < numberOfValues[index]; i++)
    {
        sheets[sheetNumbers[count]]->spansRestoreTemp.push_back(values[count].toDouble());
        count++;
    }
    index++;

    for(int i = 0; i < totalSheets; i++)
    {
        if(sheets[i]->spansRestoreTemp.size() > 0)
        {
            sheets[i]->ui->rulingSpanLW->clear();
            sheets[i]->ui->SagSwingCB->removeItem(1); // Removing the ruling span because the ruling span was actually stored in the vector "spans" so this is the wrong ruling span. If I just clear the whole CB, program crashes

            for(int span = 0; span < sheets[i]->spansRestoreTemp.size(); span++)
            {
                sheets[i]->ui->rulingSpanLW->addItem("Span " + QString::number(span + 1) + " - " + QString::number(sheets[i]->spansRestoreTemp[span]) + "m");
                sheets[i]->ui->SagSwingCB->addItem("Span " + QString::number(span + 1) + " - " + QString::number(sheets[i]->spansRestoreTemp[span]) + "m");
            }

            sheets[i]->setActualSpanRulingSpanOnly();
        }
    }

    for(int i = 0; i < totalSheets; i++)
    {
        sheets[sheetNumbers[count]]->ui->settlingInSCSB->setValue(values[count].toDouble()); count++;
        sheets[sheetNumbers[count]]->ui->settlingInSB->setValue(values[count].toDouble()); count++;
        sheets[sheetNumbers[count]]->ui->settlingInInitWindSB->setValue(values[count].toDouble()); count++;
        sheets[sheetNumbers[count]]->ui->settlingInT500SB->setValue(values[count].toDouble()); count++;
        sheets[sheetNumbers[count]]->ui->settlingInSagsNoWindSB->setValue(values[count].toDouble()); count++;
        sheets[sheetNumbers[count]]->ui->settlingInSagsInitWindSB->setValue(values[count].toDouble()); count++;
        sheets[sheetNumbers[count]]->ui->settlingInSagsT500SB->setValue(values[count].toDouble()); count++;
        sheets[sheetNumbers[count]]->ui->settlingInSwingsInitWindSB->setValue(values[count].toDouble()); count++;
        sheets[sheetNumbers[count]]->ui->settlingInSwingsT500SB->setValue(values[count].toDouble()); count++;
    }
    index++;

    for(int i = 0; i < totalSheets; i++)
    {
        sheets[sheetNumbers[count]]->ui->settlingInSCcheckBox->setChecked(values[count].toInt()); count++;
        sheets[sheetNumbers[count]]->ui->settlingIncheckBox->setChecked(values[count].toInt()); count++;
        sheets[sheetNumbers[count]]->ui->settlingInInitWindcheckBox->setChecked(values[count].toInt()); count++;
        sheets[sheetNumbers[count]]->ui->settlingInT500checkBox->setChecked(values[count].toInt()); count++;
        sheets[sheetNumbers[count]]->ui->settlingInSagsNoWindCheckBox->setChecked(values[count].toInt()); count++;
        sheets[sheetNumbers[count]]->ui->settlingInSagsInitWindCheckBox->setChecked(values[count].toInt()); count++;
        sheets[sheetNumbers[count]]->ui->settlingInSagsT500WindCheckBox->setChecked(values[count].toInt()); count++;
        sheets[sheetNumbers[count]]->ui->settlingInSwingsInitWindCheckBox->setChecked(values[count].toInt()); count++;
        sheets[sheetNumbers[count]]->ui->settlingInSwingsT500CheckBox->setChecked(values[count].toInt()); count++;
    }
    index = 0;

    for(int sheetIndex = 0; sheetIndex < totalSheets; sheetIndex++)
    {
        totalSpansLength = 0;
        totalTables = sheets[sheetIndex]->ui->resultsTabWidget->count();

        for(int tableIndex = 0; tableIndex < totalTables; tableIndex++)
        {
            table = sheets[sheetIndex]->getTable(tableIndex);

            for(int row = 0; row < totalRows[index]; row++)
            {
                table->insertRow(row);
                table->setVerticalHeaderItem(row, new QTableWidgetItem);
                table->verticalHeaderItem(row)->setText(values[count]);

                count++;
            }
            SheetSetup::setTableWidgetItems(table);
            SheetSetup::setTables(table);
            index++;
        }

        sheets[sheetIndex]->calculateTables();

        for(int i = 0; i < sheets[sheetIndex]->spans.size(); i++)
            totalSpansLength += sheets[sheetIndex]->spans[i];

        sheets[sheetIndex]->ui->initialCondTW->item(0, 1)->setText(QString("%1").arg(totalSpansLength, 0, 'g', 15));
        sheets[sheetIndex]->ui->initialCondTW->item(0, 2)->setText(QString("%1").arg(CalculateTensionChange::calculateRulingSpan(sheets[sheetIndex]->spans), 0, 'f', 3));
    }

    ui->sheetsTabWidget->setCurrentIndex(0);

    this->setWindowTitle(getFileName(filePath) + " - TensionWiz");
}

// Function to replace values from twz file
void MainWindow::replaceValues(const QStringList &newValues, const QString &attribute, QStringList &file)
{
    QStringList newFile;
    int firstHalfIndex, index = 0;

    if(file.size() == 0) // Avoidoing index out of range error
        file.append("\n");

    while(file[index] != attribute && index < file.size() - 1)
        index++;

    if(file[index] == attribute)
    {
        index++;

        for(firstHalfIndex = 0; firstHalfIndex < index; firstHalfIndex++)
            newFile.append(file[firstHalfIndex]);

        for(int i = 0; i < newValues.size(); i++)
            newFile.append(newValues[i]);
        newFile.append("$ATTRIBUTE_END");

        while(file[firstHalfIndex] != "$ATTRIBUTE_END")
            firstHalfIndex++;
        firstHalfIndex++;

        for(firstHalfIndex; firstHalfIndex < file.size(); firstHalfIndex++)
            newFile.append(file[firstHalfIndex]);
    }
    else if(index >= file.size() - 1)
    {
        for(int i = 0; i < file.size(); i++)
            newFile.append(file[i]);
        newFile.append("\n");
        newFile.append(attribute);

        for(int i = 0; i < newValues.size(); i++)
            newFile.append(newValues[i]);
        newFile.append("$ATTRIBUTE_END");
    }
    file = newFile;
}

void MainWindow::on_actionNew_triggered()
{
    if(closeFile() == true)
    {
        twzFile.close();
        twzFile.fileName().clear();
        signalPrevent = false;
        for(int i = 0; i < sheets.size(); i++)
        {
            delete sheets[i];
            delete sheetWidgets[i];
        }
        sheets.clear();
        sheetWidgets.clear();
        insertSheet();
        signalPrevent = true;

        this->setWindowTitle("Untitled - TensionWiz");

        saveConfirmation = false;
    }
}

QString MainWindow::getFileName(const QString &filePath)
{
    if(filePath != "")
    {
        QString fileName;
        int index = filePath.size() - 1;

        while(filePath[index] != '\\' && filePath[index] != '/')
        {
            fileName.push_front(filePath[index]);
            index--;
        }

        return fileName;
    }
    else return "Untitled";
}

void MainWindow::copySheet()
{
    QTableWidget *table;
    QTableWidget *tableToBeCopied;
    const int sheetToBeCopiedIndex = ui->sheetsTabWidget->currentIndex();
    const int totalTables = sheets[sheetToBeCopiedIndex]->ui->resultsTabWidget->count();

    insertSheet();

    const int copiedSheetIndex = sheets.size() - 1;
    int totalRows;
    int totalColumns;

    for(int i = 0; i < sheets[sheetToBeCopiedIndex]->spans.size(); i++)
    {
        sheets[copiedSheetIndex]->spans.push_back(sheets[sheetToBeCopiedIndex]->spans[i]);

    }

    for(int i = 0; i < sheets[sheetToBeCopiedIndex]->ui->rulingSpanLW->count(); i++)
    {
        sheets[copiedSheetIndex]->ui->rulingSpanLW->addItem(sheets[sheetToBeCopiedIndex]->ui->rulingSpanLW->item(i)->text());
        sheets[copiedSheetIndex]->ui->SagSwingCB->addItem(sheets[sheetToBeCopiedIndex]->ui->SagSwingCB->itemText(i + 1));
    }

    for(int i = 0; i < sheets[sheetToBeCopiedIndex]->ui->initialCondTW->columnCount(); i++)
        sheets[copiedSheetIndex]->ui->initialCondTW->item(0, i)->setText(sheets[sheetToBeCopiedIndex]->ui->initialCondTW->item(0, i)->text());

    for(int i = 0; i < sheets[sheetToBeCopiedIndex]->spansRestoreTemp.size(); i++)
        sheets[copiedSheetIndex]->spansRestoreTemp.push_back(sheets[sheetToBeCopiedIndex]->spansRestoreTemp[i]);

    for(int i = 0; i < totalTables; i++)
    {
        table = sheets[copiedSheetIndex]->getTable(i);
        tableToBeCopied = sheets[sheetToBeCopiedIndex]->getTable(i);
        totalRows = tableToBeCopied->rowCount();
        totalColumns = tableToBeCopied->columnCount();
        sheets[copiedSheetIndex]->getSettlingInSpinBox(i)->setValue(sheets[sheetToBeCopiedIndex]->getSettlingInSpinBox(i)->value());
        sheets[copiedSheetIndex]->getSettlingInCheckBox(i)->setChecked(sheets[sheetToBeCopiedIndex]->getSettlingInState(i));
        sheets[copiedSheetIndex]->removeAllRows(table);

        while(table->columnCount() > 0)
            table->removeColumn(0);

        for(int column = 0; column < totalColumns; column++)
        {
            table->insertColumn(column);
            table->setHorizontalHeaderItem(column, new QTableWidgetItem);
            table->horizontalHeaderItem(column)->setText(tableToBeCopied->horizontalHeaderItem(column)->text());
        }

        for(int row = 0; row < totalRows; row++)
        {
            table->insertRow(row);
            table->setVerticalHeaderItem(row, new QTableWidgetItem);
            table->verticalHeaderItem(row)->setText(tableToBeCopied->verticalHeaderItem(row)->text());
        }
        SheetSetup::setTableWidgetItems(table);
        SheetSetup::setTables(table);

    }

    sheets[copiedSheetIndex]->ui->titleLE->setText(sheets[sheetToBeCopiedIndex]->ui->titleLE->text());
    sheets[copiedSheetIndex]->ui->initTensionLE->setText(sheets[sheetToBeCopiedIndex]->ui->initTensionLE->text());
    sheets[copiedSheetIndex]->ui->initTempLE->setText(sheets[sheetToBeCopiedIndex]->ui->initTempLE->text());
    sheets[copiedSheetIndex]->ui->initWindLE->setText(sheets[sheetToBeCopiedIndex]->ui->initWindLE->text());
    sheets[copiedSheetIndex]->ui->finalWindLE->setText(sheets[sheetToBeCopiedIndex]->ui->finalWindLE->text());
    //sheets[copiedSheetIndex]->ui->gravityLE->setText(sheets[sheetToBeCopiedIndex]->ui->gravityLE->text());
    sheets[copiedSheetIndex]->ui->cdataCategoryCB->setCurrentText(sheets[sheetToBeCopiedIndex]->ui->cdataCategoryCB->currentText());
    sheets[copiedSheetIndex]->ui->cdataCB->setCurrentText(sheets[sheetToBeCopiedIndex]->ui->cdataCB->currentText());
    sheets[copiedSheetIndex]->ui->tensionPrecisionCB->setCurrentIndex(sheets[sheetToBeCopiedIndex]->ui->tensionPrecisionCB->currentIndex());
    sheets[copiedSheetIndex]->ui->sagPrecisionCB->setCurrentIndex(sheets[sheetToBeCopiedIndex]->ui->sagPrecisionCB->currentIndex());
    sheets[copiedSheetIndex]->ui->swingPrecisionCB->setCurrentIndex(sheets[sheetToBeCopiedIndex]->ui->swingPrecisionCB->currentIndex());
    sheets[copiedSheetIndex]->ui->fontComboBox->setCurrentText(sheets[sheetToBeCopiedIndex]->ui->fontComboBox->currentText());
    sheets[copiedSheetIndex]->ui->fontSB->setValue(sheets[sheetToBeCopiedIndex]->ui->fontSB->value());
    sheets[copiedSheetIndex]->ui->spanLE->setText(sheets[sheetToBeCopiedIndex]->ui->spanLE->text());
    sheets[copiedSheetIndex]->ui->SagSwingCB->setCurrentText(sheets[sheetToBeCopiedIndex]->ui->SagSwingCB->currentText());
    sheets[copiedSheetIndex]->ui->allTablesRB->setChecked(sheets[sheetToBeCopiedIndex]->ui->allTablesRB->isChecked());
    sheets[copiedSheetIndex]->ui->thisTableRB->setChecked(sheets[sheetToBeCopiedIndex]->ui->thisTableRB->isChecked());
    sheets[copiedSheetIndex]->ui->startValueLE->setText(sheets[sheetToBeCopiedIndex]->ui->startValueLE->text());
    sheets[copiedSheetIndex]->ui->incrementLE->setText(sheets[sheetToBeCopiedIndex]->ui->incrementLE->text());
    sheets[copiedSheetIndex]->ui->numberOfRowsSB->setValue(sheets[sheetToBeCopiedIndex]->ui->numberOfRowsSB->value());
    sheets[copiedSheetIndex]->ui->rulingSpanLE->setText(sheets[sheetToBeCopiedIndex]->ui->rulingSpanLE->text()); // This needs to be last
}

void MainWindow::tabOrderChanged(const int tabIndexCurrent, const int previousTab)
{
    if(signalPrevent == true)
    {
        if(tabIndexCurrent == ui->sheetsTabWidget->count() - 1)
        {
            signalPrevent = false;
            ui->sheetsTabWidget->tabBar()->moveTab(previousTab, tabIndexCurrent);
            signalPrevent = true;
        }
        else
        {
            signalPrevent = false;

            std::swap(sheets[tabIndexCurrent], sheets[previousTab]);
            std::swap(sheetWidgets[tabIndexCurrent], sheetWidgets[previousTab]);

            signalPrevent = true;
        }
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *e)
{
    const QMouseEvent *ev = (QMouseEvent*)e;
    const QPoint position = ev->pos();
    const int numberOfTabs = ui->sheetsTabWidget->tabBar()->count();
    const int tabIndex = ui->sheetsTabWidget->tabBar()->tabAt(position);

    if(e->type() == QEvent::MouseButtonPress)
    {
        saveConfirmation = true;
        if(ev->buttons() == Qt::RightButton)
        {
            if(tabIndex != numberOfTabs - 1)
            {
                ui->sheetsTabWidget->setCurrentIndex(ui->sheetsTabWidget->tabBar()->tabAt(ev->pos()));
                showSheetMenu(ev->pos());
            }
        }
    }
    else if(e->type() == QEvent::KeyPress)
        saveConfirmation = true;

    return false; // return true if you are finished handling the event. So, the default event handler will not be called.
}

void MainWindow::on_actionManage_Conductors_triggered()
{

    library = new ConductorLibrary(this);
    library->setAttribute(Qt::WA_DeleteOnClose);
    library->exec();
    const int sheetSize = sheets.size();
    QString conductorCategory;
    QString conductor;

    // Sending signal to Sheet to update Conductor list.
    for(int i = 0; i < sheetSize; i++)
    {
        conductorCategory = sheets[i]->ui->cdataCategoryCB->currentText();
        conductor = sheets[i]->ui->cdataCB->currentText();
        sheets[i]->setConductorCategory();
        sheets[i]->ui->cdataCategoryCB->setCurrentText(conductorCategory);
        sheets[i]->ui->cdataCB->setCurrentText(conductor);
    }
}

void MainWindow::on_actionPrint_triggered()
{
    sheets[ui->sheetsTabWidget->currentIndex()]->on_printPB_clicked();
}

void MainWindow::on_actionPDF_triggered()
{
    sheets[ui->sheetsTabWidget->currentIndex()]->on_exportPDFPB_clicked();
}

void MainWindow::on_actionXLSX_triggered()
{
    sheets[ui->sheetsTabWidget->currentIndex()]->exportSpreadSheet(1);
}

void MainWindow::on_actionXLS_triggered()
{
    sheets[ui->sheetsTabWidget->currentIndex()]->exportSpreadSheet(2);
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::information(this, "About", "TensionWiz Version 2.1.0b\nMatti Syrjänen\nmatti738@hotmail.com");
}
