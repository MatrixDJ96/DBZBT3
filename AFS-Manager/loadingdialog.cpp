#include "loadingdialog.h"
#include "ui_loadingdialog.h"
#include <QMovie>

LoadingDialog::LoadingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadingDialog)
{
    QMovie *movie = new QMovie(":/Images/Loading.gif");
    ui->setupUi(this);
    setStyleSheet("background-color: #e5eff1;");
    ui->labelLoading->setMovie(movie);
    movie->start();
}

LoadingDialog::~LoadingDialog()
{
    delete ui;
}
