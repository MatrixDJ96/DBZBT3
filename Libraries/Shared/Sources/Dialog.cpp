#include <QtCore/QVarLengthArray>
#include <QtCore/QFloat16>
#include <QtGui/QtGui>
#include "Dialog.h"
#include "ui_Dialog.h"
#include "Shared.h"

Dialog::Dialog(QWidget *parent) : QDialog(parent), ui(new Ui::Dialog)
{
	ui->setupUi(this);
}

Dialog::Dialog(DialogType type, QDialogButtonBox::StandardButtons buttons, const char *filename, QWidget *parent) : Dialog(parent)
{
	switch (type) {
		case DialogType::Info:
			ui->imgDialog->setPixmap(QString::fromUtf8(":/Info"));
			ui->progressBar->hide();
			break;
		case DialogType::Warning:
			ui->imgDialog->setPixmap(QString::fromUtf8(":/Warning"));
			ui->progressBar->hide();
			break;
		case DialogType::Error:
			ui->imgDialog->setPixmap(QString::fromUtf8(":/Error"));
			ui->progressBar->hide();
			break;
		case DialogType::Progress:
			ui->imgDialog->setPixmap(QString::fromUtf8(filename != nullptr ? filename : "")));
			ui->progressBar->hide();
			break;
		default:
			throw;
	}

	ui->buttonBox->setStandardButtons(buttons);
}

Dialog::~Dialog()
{
	delete ui;
}