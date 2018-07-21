#include "mainwindow.h"
#include "ConductorData.h"
#include <QMessageBox>

AddConductor::AddConductor(const QString &currentCategory, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddConductor)
{
    uiSetup = new AddModifyUi(ui, currentCategory, this);
    uiSetup->setUserInterface(this);
    this->setWindowTitle("Add Conductor");
    connect(ui->cancelPB, SIGNAL(clicked()), SLOT(close()));   
}

AddConductor::~AddConductor()
{
    delete ui;
}

bool AddConductor::isExistingCond(const QString &conductorName) // Checks if the conductor exists in file
{
    const QStringList conductorDataList = ConductorData::getConductorData();
    bool isExisting = false;

    if(conductorDataList.size() > 0)
    {
        if(conductorDataList[ConductorData::findAttribute(conductorDataList, "$CONDUCTOR_NAME", conductorName)].toUpper() == conductorName.toUpper())
            isExisting = true;
    }
    return isExisting;
}

void AddConductor::on_okPB_clicked()
{
    const QString conductorName = ui->nameLE->text();

    if(uiSetup->verifyInputs(this) == 0) // Checking if the user has entered all the necessary details
    {
        if(isExistingCond(conductorName)) // If there's already a conductor with this name
        {
            const QMessageBox::StandardButton sameCond = QMessageBox::warning(this, "Existing conductor", "There is already a conductor the name '" + conductorName + "'. Do you want to overwrite it?", QMessageBox::Yes|QMessageBox::No);
            if(sameCond == QMessageBox::Yes)
            {
                ConductorData::replaceConductor(uiSetup->getNewValues(), conductorName);
                close();
            }
        }
        else
        {
            ConductorData::addConductor(uiSetup->getNewValues());
            close();
        }
    }
}

void AddConductor::on_categoryLE_textChanged(const QString &arg1)
{
    if(arg1 != "")
        ui->newCatRB->setChecked(true);
    else if(ui->cdataCategoryCB->currentText() != "")
        ui->existingCatRB->setChecked(true);
}

void AddConductor::on_cdataCategoryCB_currentTextChanged()
{
    ui->existingCatRB->setChecked(true);
}
