#include "ChangeCategoryName.h"
#include "ui_ChangeCategoryName.h"
#include "ConductorData.h"
#include <QFile>
#include <QTextStream>

ChangeCategoryName::ChangeCategoryName(const QString &conductorCategory, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangeCategoryName)
{
    ui->setupUi(this);
    connect(ui->cancelPB, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->applyPB, SIGNAL(clicked()), this, SLOT(changeCategoryName()));

    setConductorViews();

    QListWidgetItem *currentItem;
    const int conductorsTotal = ui->conductorCategoriesLW->count();
    for(int i = 0; i < conductorsTotal; i++)
    {
        currentItem = ui->conductorCategoriesLW->item(i);
        if(currentItem->text() == conductorCategory)
        {
            ui->conductorCategoriesLW->setCurrentItem(currentItem);
            currentItem->setSelected(true);
        }
    }

    if(conductorCategory != "All")
        ui->newNameLE->setText(conductorCategory);

    ui->newNameLE->setFocus();
}

ChangeCategoryName::~ChangeCategoryName()
{
    delete ui;
}

void ChangeCategoryName::setConductorViews()
{
    ui->conductorCategoriesLW->clear();   
    ui->conductorCategoriesLW->addItems(ConductorData::getConductorCategories());
}

void ChangeCategoryName::on_conductorCategoriesLW_itemClicked(QListWidgetItem *item)
{
    ui->newNameLE->setText(item->text());
    ui->newNameLE->setFocus();
    ui->newNameLE->selectAll();
}

void ChangeCategoryName::on_okPB_clicked()
{
    changeCategoryName();
    close();
}

void ChangeCategoryName::changeCategoryName()
{
    if(ui->conductorCategoriesLW->count() != 0 && ui->conductorCategoriesLW->currentItem() != NULL)
    {
        QFile cdata( ConductorData::getFilePath() );
        QTextStream input(&cdata);
        const QString oldCategoryName = ui->conductorCategoriesLW->currentItem()->text();
        const QString newCategoryName = ui->newNameLE->text();
        QStringList conductorDataList = ConductorData::getConductorData();
        const int conductorDataListSize = conductorDataList.size();
        int index = 0;

        while(index < conductorDataListSize)
        {
            if(conductorDataList[index] == "$CONDUCTOR_CATEGORY")
            {
                index++;
                if(conductorDataList[index] == oldCategoryName)
                    conductorDataList.replace(index, newCategoryName);
            }
            else
                index++;
        }

        cdata.open(QFile::WriteOnly | QFile::Truncate);

        for(int i = 0; i < conductorDataListSize; i++)
        {
            input << conductorDataList[i];
            input << "\n";
        }

        cdata.close();

        setConductorViews();
    }
}
