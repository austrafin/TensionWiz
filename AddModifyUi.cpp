#include "AddModifyUi.h"
#include "ConductorData.h"
#include <QMessageBox>

AddModifyUi::AddModifyUi(Ui_AddConductor *uiReceive, const QString &currentCategoryReceive, QObject *parent) :
    QObject(parent)
{
    ui = uiReceive;
    timerIndex = 0;
    timer = new QTimer;
    currentCategory = currentCategoryReceive;
    connect(timer, SIGNAL(timeout()), this, SLOT(blinkLabel()));

}

AddModifyUi::~AddModifyUi()
{
    delete timer;
}

void AddModifyUi::setUserInterface(QDialog *dialog)
{
    ui->setupUi(dialog);

    connect(ui->massCB, SIGNAL(currentTextChanged(QString)), this, SLOT(changeValidator(QString)));
    connect(ui->utsCB, SIGNAL(currentTextChanged(QString)), this, SLOT(changeValidator(QString)));
    connect(ui->moeCB, SIGNAL(currentTextChanged(QString)), this, SLOT(changeValidator(QString)));

    QDoubleValidator *utsValidator = new QDoubleValidator(0, 999.999999999, 9, this);
    QDoubleValidator *totalDiaValidator = new QDoubleValidator(0, 99.9999999999, 10, this);
    QDoubleValidator *massValidator = new QDoubleValidator(0, 9999.99999999, 8, this);

    totalDiaValidator->setNotation(QDoubleValidator::StandardNotation);
    massValidator->setNotation(QDoubleValidator::StandardNotation);
    utsValidator->setNotation(QDoubleValidator::StandardNotation);

    ui->utsLE->setValidator(utsValidator);
    ui->r20LE->setValidator(utsValidator);
    ui->totalDiaLE->setValidator(totalDiaValidator);
    ui->csaLE->setValidator(utsValidator);
    ui->massLE->setValidator(massValidator);
    ui->moeLE->setValidator(utsValidator);
    ui->coeLE->setValidator(utsValidator);
    ui->nameLE->setFocus();
    ui->cdataCategoryCB->addItems(ConductorData::getConductorCategories());
    ui->cdataCategoryCB->setCurrentText(currentCategory);

    if(ui->cdataCategoryCB->currentText() == "")
    {
        ui->existingCatRB->setDisabled(true);
        ui->cdataCategoryCB->setDisabled(true);
        ui->newCatRB->setChecked(true);
    }
}

QString AddModifyUi::getConductorCategory()
{
    if(ui->newCatRB->isChecked()) // If using new Category
        return ui->categoryLE->text();
    else
        return ui->cdataCategoryCB->currentText();
}

QStringList AddModifyUi::getNewValues()
{
    bool ok = false;
    QStringList newValues;

    newValues.append(getConductorCategory());
    newValues.append(ui->nameLE->text());

    if(ui->swDiaLE->text() != "")
        newValues.append(ui->swDiaLE->text() + "$" + ui->swDiaCB->currentText());
    else
        newValues.append(ui->swDiaLE->text());

    newValues.append(ui->totalDiaLE->text() + "$mm");
    newValues.append(ui->csaLE->text() + "$mm^2");

    if(ui->massCB->currentText() == "kg/km")
        newValues.append( QString("%1").arg(ui->massLE->text().toDouble(&ok) / 1000, 0, 'g', 12) ); // Converting kg/km to kg/m
    else
        newValues.append(ui->massLE->text());

    if(ui->utsCB->currentText() == "kN")
        newValues.append( QString("%1").arg(ui->utsLE->text().toDouble(&ok) * 1000, 0, 'g', 12) ); // Converting kN to newtons
    else
        newValues.append(ui->utsLE->text());

    if(ui->moeCB->currentText() == "GPa")
        newValues.append( QString("%1").arg(ui->moeLE->text().toDouble(&ok) * 1000000, 0, 'g', 12) ); // Converting GPa to kPa
    else
        newValues.append(ui->moeLE->text());

    newValues.append(ui->coeLE->text() + "$/ºCx10^-6");

    if(ui->r20LE->text() != "")
        newValues.append(ui->r20LE->text() + "$Ohms/km");
    else
        newValues.append(ui->r20LE->text());

    newValues.append(ui->commentTE->toPlainText());

    return newValues;
}

bool AddModifyUi::verifyInputs(QWidget *w)
{
    condition_name = false;
    bool condition_category = false;
    bool condition_2 = false; // For checking that the text doesn't have character '$'
    bool returnValue = 1;
    int index = 0;
    QString conductorCategory;
    const QString conductorName = ui->nameLE->text();

    if(ui->newCatRB->isChecked()) // If using new Category
    {
        conductorCategory = ui->categoryLE->text();

        if(conductorCategory != "")
        {
            while(index < conductorCategory.size())
            {
                if(conductorCategory[index] == ' ')
                    condition_category = true;
                if(conductorCategory[index] != ' ')
                    condition_category = false;
                if(conductorCategory[index] == '$')
                    condition_2 = true;
                index++;
            }

        }
        index = 0;
    }
    else
         conductorCategory = ui->cdataCategoryCB->currentText();

    if(conductorName != "" && condition_2 == false)
    {
        do
        {
            if(conductorName[index] == ' ')
                condition_name = true;
            if(conductorName[index] != ' ')
                condition_name = false;
            if(conductorName[index] == '$')
                condition_2 = true;
            index++;
        }
        while(index < conductorName.size() - 1);
    }
    index = 0;

    if(ui->swDiaLE->text() != "" && condition_name == false && condition_2 == false)
    {
        do
        {
            if(ui->swDiaLE->text()[index] == '$')
                condition_2 = true;
            index++;
        }
        while(index < ui->swDiaLE->text().size() - 1);
    }
    index = 0;

    if(ui->commentTE != NULL && condition_name == false && condition_2 == false) // CommentTE pointer has to be NULL, not ""
    {
        do
        {
            if(ui->commentTE->toPlainText()[index] == '$')
                condition_2 = true;
            index++;
        }
        while(index < ui->commentTE->toPlainText().size() - 1);
    }

    if(ui->newCatRB->isChecked() == true && (ui->categoryLE->text().toUpper() == "ALL" || ui->categoryLE->text() == "" || condition_category == true))
        QMessageBox::critical(w, "Invalid input", "Invalid category name.");
    else if((ui->nameLE->text() == "") || ui->utsLE->text().toDouble() <= 0 || ui->totalDiaLE->text().toDouble() <= 0 || ui->csaLE->text().toDouble() <= 0 || ui->massLE->text().toDouble() <= 0 || ui->moeLE->text().toDouble() <= 0 || ui->coeLE->text().toDouble() <= 0)
        timer->start(150);
    else if(condition_2 == true)
        QMessageBox::critical(w, "Invalid input", "Symbol '$' is forbidden.");
    else
        returnValue = 0;

    return returnValue; // Returning 1 if everything is ok and 0 if not
}

void AddModifyUi::blinkLabel()
{
    const QColor emptyInputColour (255,160,160);
    const QColor white(Qt::white);
    QPalette palette;
    QPalette palette2;
    QVector<QLabel*> labels;
    QVector<QLineEdit*> lineEdits;

    labels.push_back(ui->utsLabel);
    labels.push_back(ui->totalDiaLabel);
    labels.push_back(ui->csaLabel);
    labels.push_back(ui->massLabel);
    labels.push_back(ui->moeLabel);
    labels.push_back(ui->coeLabel);

    lineEdits.push_back(ui->utsLE);
    lineEdits.push_back(ui->totalDiaLE);
    lineEdits.push_back(ui->csaLE);
    lineEdits.push_back(ui->massLE);
    lineEdits.push_back(ui->moeLE);
    lineEdits.push_back(ui->coeLE);

    if(timerIndex < 4) // Blinking four times
    {
        if(condition_name == true || ui->nameLE->text() == "")
        {
            ui->nameLabel->setStyleSheet("QLabel { color : red; }");
            palette.setColor(ui->nameLE->backgroundRole(), emptyInputColour);
            ui->nameLE->setPalette(palette);

            if(ui->nameLabel->isVisible())
                ui->nameLabel->setVisible(0);
            else
                ui->nameLabel->setVisible(1);
        }
        else
        {
            ui->nameLabel->setStyleSheet("QLabel { color : black; }");
            palette.setColor(ui->nameLE->backgroundRole(), white);
            ui->nameLE->setPalette(palette);
        }

        for(int i = 0; i < lineEdits.size(); i++)
        {
            if(lineEdits[i]->text().toDouble() <= 0)
            {
                labels[i]->setStyleSheet("QLabel { color : red; }");
                palette.setColor(lineEdits[i]->backgroundRole(), emptyInputColour);
                lineEdits[i]->setPalette(palette);

                if(labels[i]->isVisible())
                    labels[i]->setVisible(0);
                else
                    labels[i]->setVisible(1);
            }
            else
            {
                labels[i]->setStyleSheet("QLabel { color : black; }");
                palette2.setColor(lineEdits[i]->backgroundRole(), white);
                lineEdits[i]->setPalette(palette2);
            }
        }
        timerIndex++;
    }
    else
    {
        timer->stop();
        timerIndex = 0;
    }
}

void AddModifyUi::changeValidator(const QString &cbText)
{
    QLineEdit *le;
    QDoubleValidator *validator = new QDoubleValidator(this);

    validator->setNotation(QDoubleValidator::StandardNotation);
    if(cbText == "kg/m")
    {
        le = ui->massLE;
        le->clear();
        delete le->validator();

        validator->setRange(0, 9.99999999999, 11);

        le->setValidator(validator);
    }
    else if(cbText == "kg/km")
    {
        le = ui->massLE;
        le->clear();
        delete le->validator();

        validator->setRange(0, 9999.99999999, 8);
        le->setValidator(validator);
    }
    else if(cbText == "N")
    {
        le = ui->utsLE;
        le->clear();
        delete le->validator();

        validator->setRange(0, 999999.999999, 6);
        le->setValidator(validator);
    }
    else if(cbText == "kN")
    {
        le = ui->utsLE;
        le->clear();
        delete le->validator();

        validator->setRange(0, 999.999999999, 9);
        le->setValidator(validator);
    }
    else if(cbText == "kPa")
    {
        le = ui->moeLE;
        le->clear();
        delete le->validator();

        validator->setRange(0, 999999999.999, 3);
        le->setValidator(validator);
    }
    else if(cbText == "GPa")
    {
        le = ui->moeLE;
        le->clear();
        delete le->validator();

        validator->setRange(0, 999.999999999, 9);
        le->setValidator(validator);
    }
}
