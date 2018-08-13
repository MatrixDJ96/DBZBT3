#include "modifyreservedspacesdialog.h"
#include "ui_modifyreservedspacesdialog.h"

ModifyReservedSpacesDialog::ModifyReservedSpacesDialog(const uint32_t previous, const uint32_t following, QWidget *parent) :
    QDialog(parent),
    spaceFromPrevious(0),
    spaceFromFollowing(0),
    ui(new Ui::ModifyReservedSpacesDialog)
{
    ui->setupUi(this);

    ui->spinBoxSpacePrevious->setMaximum(previous);
    ui->spinBoxSpaceFollowing->setMaximum(following);

    if(previous < 2048)
        ui->spinBoxSpacePrevious->setEnabled(false);

    if(following < 2048)
        ui->spinBoxSpaceFollowing->setEnabled(false);
}

ModifyReservedSpacesDialog::~ModifyReservedSpacesDialog()
{
    delete ui;
}

void ModifyReservedSpacesDialog::on_pushButtonCancel_clicked()
{
    spaceFromPrevious = 0;
    spaceFromFollowing = 0;
    close();
}

void ModifyReservedSpacesDialog::on_pushButtonSave_clicked()
{
    spaceFromPrevious = ui->spinBoxSpacePrevious->value();
    spaceFromFollowing = ui->spinBoxSpaceFollowing->value();
    close();
}

void ModifyReservedSpacesDialog::on_spinBoxSpacePrevious_editingFinished()
{
    int oldValue = ui->spinBoxSpacePrevious->value();
    if(oldValue % 2048)
    {
        int newValue = oldValue - (oldValue % 2048);
        ui->spinBoxSpacePrevious->setValue(newValue < 0 ? 0 : newValue);
    }
}

void ModifyReservedSpacesDialog::on_spinBoxSpaceFollowing_editingFinished()
{
    int oldValue = ui->spinBoxSpaceFollowing->value();
    if(oldValue % 2048)
    {
        int newValue = oldValue - (oldValue % 2048);
        ui->spinBoxSpaceFollowing->setValue(newValue < 0 ? 0 : newValue);
    }
}
