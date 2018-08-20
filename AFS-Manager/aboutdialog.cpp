#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    QPixmap pixmap(":/Images/AboutImage.png");
    ui->setupUi(this);
    ui->labelImage->setPixmap(pixmap);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
