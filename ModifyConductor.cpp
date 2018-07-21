#include "ModifyConductor.h"
#include "ConductorData.h"
#include <QMessageBox>

ModifyConductor::ModifyConductor(const QString &conductorName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddConductor)
{
    bool ok = false;
    QStringList values;
    const QStringList conductorDataList = ConductorData::getConductorData();
    const QStringList conductorAttributes = ConductorData::getConductorAttributes(1);
    int index = 0;

    oldConductorName = conductorName;

    for(int i = 0; i < conductorAttributes.size(); i++)
        values.append(ConductorData::getAttributeWithoutUnit(conductorDataList, conductorAttributes[i], oldConductorName));

    uiSetup = new AddModifyUi(ui, values[index], this); index++;
    uiSetup->setUserInterface(this);
    this->setWindowTitle("Modify Conductor");
    connect(ui->cancelPB, SIGNAL(clicked()), SLOT(close()));

    ui->nameLE->setText(values[index]); index++;

    ui->swDiaLE->setText(values[index]);
    ui->swDiaCB->setCurrentText(ConductorData::getAttributeUnit(conductorDataList, conductorAttributes[index], oldConductorName)); index++; // these two must be together

    ui->totalDiaLE->setText(values[index]); index++;
    ui->csaLE->setText(values[index]); index++;

    ui->massLE->setText( QString("%1").arg(values[index].toDouble(&ok) * 1000, 0, 'g', 12) ); index++;

    ui->utsLE->setText( QString("%1").arg(values[index].toDouble(&ok) / 1000, 0, 'g', 12) ); index++;
    ui->moeLE->setText( QString("%1").arg(values[index].toDouble(&ok) / 1000000, 0, 'g', 12) ); index++;
    ui->coeLE->setText(values[index]); index++;
    ui->r20LE->setText(values[index]); index++;
    ui->commentTE->setPlainText (values[index]);
}

ModifyConductor::~ModifyConductor()
{
    delete ui;
}

void ModifyConductor::on_okPB_clicked()
{
    const QString conductorName = ui->nameLE->text();

    if(uiSetup->verifyInputs(this) == 0) // Checking if the user has entered all the necessary details
    {
        if(isExistingCond(conductorName)) // If existing
        {
            QMessageBox::StandardButton sameCond;
            sameCond = QMessageBox::warning(this, "Existing conductor", "There is already a conductor the name '" + conductorName + "'. Do you want to overwrite it?", QMessageBox::Yes|QMessageBox::No);
            if(sameCond == QMessageBox::Yes)
            {
                ConductorData::removeConductor(oldConductorName);
                ConductorData::replaceConductor(uiSetup->getNewValues(), conductorName);
                close();
            }
        }

        else
        {
            ConductorData::replaceConductor(uiSetup->getNewValues(), oldConductorName);
            close();
        }
    }
}

bool ModifyConductor::isExistingCond(const QString &conductorName)
{
    const QStringList conductorDataList = ConductorData::getConductorData();
    const QString conductorNameFile = conductorDataList[ConductorData::findAttribute(conductorDataList, "$CONDUCTOR_NAME", conductorName)].toUpper();
    bool isExisting = false;

    if(conductorDataList.size() > 0)
    {
        if(conductorNameFile == conductorName.toUpper() && conductorNameFile != oldConductorName.toUpper())
            isExisting = true;
    }
    return isExisting;
}

void ModifyConductor::on_categoryLE_textChanged(const QString &arg1)
{
    if(arg1 != "")
        ui->newCatRB->setChecked(true);
    else
        ui->existingCatRB->setChecked(true);
}

void ModifyConductor::on_cdataCategoryCB_currentTextChanged()
{
    ui->existingCatRB->setChecked(true);
}
