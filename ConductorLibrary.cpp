#include "ConductorLibrary.h"
#include "ui_ConductorLibrary.h"
#include "ConductorData.h"
#include <QTextStream>
#include <QKeyEvent>
#include <QFileDialog>
#include <QMessageBox>

ConductorLibrary::ConductorLibrary(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConductorLibrary)
{
    ui->setupUi(this);
    this->setWindowTitle("Conductor Library");
    ui->conductorLibraryTB->setOpenExternalLinks(true);

    connect(ui->closePB, SIGNAL(clicked()), this, SLOT(close()));

    setConductorViews();
}

ConductorLibrary::~ConductorLibrary()
{
    delete ui;
}

void ConductorLibrary::setConductorViews()
{
    const QString currentCategory = ui->conductorLibraryCB->currentText();
    ui->conductorLibraryLW->clear();
    ui->conductorLibraryCB->clear();
    ui->conductorLibraryCB->addItem("All");
    ui->conductorLibraryCB->addItems(ConductorData::getConductorCategories());
    ui->conductorLibraryLW->sortItems();
    ui->conductorLibraryCB->setCurrentText(currentCategory);

    // List view is set from the "Current text changed signal".
}


void ConductorLibrary::on_conductorLibraryCB_currentTextChanged(const QString &arg1)
{
    ui->conductorLibraryLW->clear();
    ui->conductorLibraryLW->addItems(ConductorData::getConductorNames(arg1));
}

void ConductorLibrary::on_addPB_clicked()
{
    newconductor = new AddConductor(ui->conductorLibraryCB->currentText(), this);
    newconductor->setAttribute(Qt::WA_DeleteOnClose);
    newconductor->exec();

    setConductorViews();
}

void ConductorLibrary::on_deletePB_clicked()
{
    const QString conductorName = ui->conductorLibraryLW->currentItem()->text();
    ConductorData::removeConductor(conductorName);
    ui->conductorLibraryTB->clear();
    setConductorViews();
}

void ConductorLibrary::on_modifyPB_clicked()
{
    const QString conductorName = ui->conductorLibraryLW->currentItem()->text();
    modifyConductor = new ModifyConductor(conductorName, this);
    modifyConductor->setAttribute(Qt::WA_DeleteOnClose);
    modifyConductor->exec();

    setConductorViews();
}

void ConductorLibrary::on_fontSB_valueChanged(const int arg1)
{
    QTextDocument temp;
    temp.setPlainText(ui->conductorLibraryTB->toPlainText());
    ui->conductorLibraryTB->setFontPointSize(arg1);
    ui->conductorLibraryTB->setText(temp.toPlainText());
}

void ConductorLibrary::keyPressEvent(QKeyEvent *keyevent)
{
    if((keyevent->key() == Qt::Key_Delete) && ui->conductorLibraryLW)
    {
        if(ui->deletePB->isEnabled())
            on_deletePB_clicked();
    }
}

void ConductorLibrary::on_importPB_clicked()
{
    importConductors(QFileDialog::getOpenFileNames(this, tr("Import Conductor File"), "", "Conductor File (*.cond)"));
}

void ConductorLibrary::on_conductorLibraryLW_itemSelectionChanged()
{
    if(ui->conductorLibraryLW->currentItem()->isSelected())
    {
        bool ok = false;
        const QString conductorName = ui->conductorLibraryLW->currentItem()->text();
        const QStringList conductorDataList = ConductorData::getConductorData();
        const QStringList conductorAttributes = ConductorData::getConductorAttributes(0);
        QStringList cInfo;
        QStringList attributeUnits;
        QString text = "Conductor Information\n";
        const int conductorAttributesSize = conductorAttributes.size();
        int index = 0;

        ui->exportPB->setEnabled(true);
        ui->deletePB->setEnabled(true);
        ui->modifyPB->setEnabled(true);
        ui->conductorLibraryTB->clear();

        for(int i = 1; i < conductorAttributesSize; i++)
        {
            cInfo.append(ConductorData::getAttributeWithoutUnit(conductorDataList, conductorAttributes[i], conductorName));
            attributeUnits.append(ConductorData::getAttributeUnit(conductorDataList, conductorAttributes[i], conductorName));
        }

        text.append("\nStranding & Wire Diameter: " + cInfo[index] + attributeUnits[index]);   index++;
        text.append("\nTotal Diameter: " + cInfo[index] + attributeUnits[index]);   index++;
        text.append("\nCross Sectional Area (A): " + cInfo[index] + attributeUnits[index]);   index++;
        text.append("\nMass: " + QString("%1").arg((cInfo[index].toDouble(&ok) * 1000), 0, 'g', 12) + "kg/km");   index++;
        text.append("\nUTS: " + QString("%1").arg((cInfo[index].toDouble() / 1000), 0, 'g', 12) + "kN");   index++;
        text.append("\nModulus of Elasticity (E): " + QString("%1").arg((cInfo[index].toDouble() / 1000000), 0, 'g', 12) + "GPa");   index++; // Converting kPa to GPa
        text.append("\nCoefficient of Expansion (a): " + cInfo[index] + attributeUnits[index]);   index++;
        text.append("\nResistance at 20°C: " + cInfo[index] + attributeUnits[index]);   index++;
        text.append("\n\n");


        ui->conductorLibraryTB->insertPlainText(text);
        ui->conductorLibraryTB->insertHtml (cInfo[index]);


    }
    else
    {
        ui->conductorLibraryTB->clear();
        ui->exportPB->setDisabled(true);
        ui->modifyPB->setDisabled(true);
        ui->deletePB->setDisabled(true);
    }
}

void ConductorLibrary::importConductors(const QStringList &files)
{
    if(!files.empty())
    {        
        const int filesSize = files.size();

        for(int i = 0; i < filesSize; i++)
        {
            int index = 0;
            QFile cFile(files[i]);
            QTextStream importCond(&cFile);
            QString conductorName;
            const QStringList conductorDataList = ConductorData::getConductorData();
            QStringList newValues;
            QStringList conductorAttributes;
            QStringList importantAttributes;
            QStringList existingConductors;
            const QStringList allAttributes = ConductorData::getConductorAttributes(1);
            const int conductorDataListSize = conductorDataList.size();
            bool isCorrupt = false;

            // Finding out how many conductors is already in the system
            for(int i = 0; i < conductorDataListSize; i++)
            {
                if(conductorDataList[i] == "$CONDUCTOR_NAME")
                {
                    if(i < conductorDataListSize - 1)
                        existingConductors.append(conductorDataList[i + 1]);
                }
            }

            importantAttributes.append("$CONDUCTOR_CATEGORY");
            importantAttributes.append("$CONDUCTOR_NAME");
            importantAttributes.append("$ULTIMATE_TENSILE_STRENGTH");
            importantAttributes.append("$TOTAL_DIAMETER");
            importantAttributes.append("$CROSS_SECTIONAL_AREA");
            importantAttributes.append("$CONDUCTOR_WEIGHT");
            importantAttributes.append("$MODULUS_OF_ELASTICITY");
            importantAttributes.append("$COEFFICIENT_OF_EXPANSION");




            cFile.open(QFile::ReadOnly);

            // Writing text in a string list.
            do
                conductorAttributes.append(importCond.readLine());
            while(!importCond.atEnd());

            cFile.close();          

            // Performing error check
            for(int i = 0; i < importantAttributes.size(); i++)
            {
                index = 0;

                while(conductorAttributes[index] != importantAttributes[i] && index < conductorAttributes.size() - 2)
                    index++;
                if((conductorAttributes[index] != importantAttributes[i] || conductorAttributes[index + 1] == "") && index < conductorAttributes.size() - 2)
                    isCorrupt = true;
            }

            if(isCorrupt == true)
                QMessageBox::critical(this, "File corrupt", "The file is corrupt or not compatible. Cannot import");
            else
            {   
                for(int i = 0; i < allAttributes.size(); i++)
                {
                    index = 0;

                    while(conductorAttributes[index] != allAttributes[i])
                        index++;
                    index++;

                    if(allAttributes[i] == "$COMMENT")
                    {
                        while(conductorAttributes[index] != "$CONDUCTOR_DETAILS_END" && index < conductorAttributes.size())
                        {
                            newValues.append(conductorAttributes[index]);
                            index++;
                        }
                    }
                    else
                        newValues.append(conductorAttributes[index]);
                }

                index = 0;

                bool isExisting = false;

                while(index < conductorAttributes.size() - 1)
                {
                    if(conductorAttributes[index] == "$CONDUCTOR_NAME")
                    {
                        index++;

                        for(int i = 0; i < existingConductors.size(); i++)
                        {
                            if(conductorAttributes[index] == existingConductors[i])
                            {
                                isExisting = true;
                                conductorName = conductorAttributes[index];
                            }
                        }
                    }
                    else
                        index++;
                }

                index = 0;

                if(isExisting == true)
                {
                    QMessageBox::StandardButton sameCond;
                    sameCond = QMessageBox::warning(this, "Existing conductor", "There is already a conductor the name '" + conductorName + "'. Do you want to overwrite it?", QMessageBox::Yes|QMessageBox::No);
                    if(sameCond == QMessageBox::Yes)
                        ConductorData::replaceConductor(newValues, conductorName);
                }             
                else
                    ConductorData::addConductor(newValues);

                setConductorViews();
            }
            conductorAttributes.clear();
        }
    }
}

void ConductorLibrary::on_changeCategoryNamePB_clicked()
{
    QString conductorCategory;

    if(ui->conductorLibraryCB->currentText() != "All")
        conductorCategory = ui->conductorLibraryCB->currentText();

    newCategoryName = new ChangeCategoryName(conductorCategory, this);
    newCategoryName->setAttribute(Qt::WA_DeleteOnClose);
    newCategoryName->exec();

    setConductorViews();

    ui->conductorLibraryCB->setCurrentText(conductorCategory);
}

void ConductorLibrary::on_exportAllPB_clicked()
{
    const QString filePath = QFileDialog::getExistingDirectory(this, "Export Conductors", "", QFileDialog::ShowDirsOnly);
    if(filePath != "")
    {
        QStringList conductorNames;
        QStringList fileNames;

        for(int i = 0; i < ui->conductorLibraryLW->count(); i++)
        {
            conductorNames.append(ui->conductorLibraryLW->item(i)->text());
            fileNames.append(filePath + "/" + convertConductorName(conductorNames[i]) + ".cond");
        }

        exportConductors(fileNames, conductorNames);
    }
}

void ConductorLibrary::on_exportPB_clicked()
{
    const QString conductorName = ui->conductorLibraryLW->currentItem()->text();
    QStringList conductors;
    QStringList fileName;

    conductors.append(conductorName);

    fileName.append( QFileDialog::getSaveFileName(this, tr("Export Conductor File"), convertConductorName(conductorName), "Conductor File (*.cond)") );
    exportConductors(fileName, conductors);
}

QString ConductorLibrary::convertConductorName(const QString &conductorName)
{
    QString conductorNameFile;
    const int conductorNameSize = conductorName.size();
    int index = 0;

    // Using name of the conductor but converting character Windows won't allow
    while(index < conductorNameSize)
    {
        if(conductorName[index] != '/')
            conductorNameFile.append(conductorName[index]);
        else
            conductorNameFile.append('-');
        index++;
    }

    return conductorNameFile;
}

void ConductorLibrary::exportConductors(const QStringList &filePaths, const QStringList &conductorNames)
{
    if(filePaths[0] != "")
    {
        int index;

        QFile cFile;
        QTextStream exportCond;
        const QStringList conductorDataList = ConductorData::getConductorData();

        for(int i = 0; i < conductorNames.size(); i++)
        {
            cFile.setFileName(filePaths[i]);
            exportCond.setDevice(&cFile);
            index = ConductorData::findAttribute(conductorDataList, "$CONDUCTOR_NAME", conductorNames[i]); // Finding the conductor to be exported.

            while(conductorDataList[index] != "$CONDUCTOR_DETAILS_START")
                index--;

            // Writing details to file.
            cFile.open(QFile::WriteOnly);

            do
            {
                exportCond << conductorDataList[index];
                //qDebug() << conductorDataList[index];
                exportCond << "\n";
                index++;
            }
            while(conductorDataList[index - 1] != "$CONDUCTOR_DETAILS_END");

            cFile.close();
        }
    }
}

void ConductorLibrary::on_deleteAllPB_clicked()
{
    const QMessageBox::StandardButton mBox = QMessageBox::warning(this, "Delete Conductors", "This action cannot be undone. Are you sure you want to delete all the conductors in this categry?", QMessageBox::Yes|QMessageBox::No);

    if(mBox == QMessageBox::Yes)
    {
        ui->conductorLibraryLW->clearSelection();
        while(ui->conductorLibraryLW->count() > 0)
        {
            ConductorData::removeConductor(ui->conductorLibraryLW->item(0)->text());
            delete ui->conductorLibraryLW->item(0);
        }
        setConductorViews();
    }
}
