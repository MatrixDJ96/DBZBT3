#include "../Headers/Dialog.h"
#include <ui_Dialog.h>

#include <Shared.h>

Dialog::Dialog(QWidget *parent) : QDialog(parent), ui(new Ui::Dialog)
{
	ui->setupUi(this);
}

Dialog::Dialog(const char *message, const char *title, QDialogButtonBox::StandardButtons buttons, QWidget *parent) : Dialog(parent)
{
	setWindowTitle(QString::fromUtf8(title != nullptr ? title : ""));
	ui->textDialog->setText(QString::fromUtf8(message != nullptr ? message : ""));
	ui->buttonBox->setStandardButtons(buttons);
}

Dialog::Dialog(const char *message, const char *title, QDialogButtonBox::StandardButtons buttons, DialogType icon, QWidget *parent) : Dialog(message, title, buttons, parent)
{
	switch (icon) {
		case DialogType::Info:
			ui->imgDialog->setPixmap(QString::fromUtf8(":/Info"));
			break;
		case DialogType::Warning:
			ui->imgDialog->setPixmap(QString::fromUtf8(":/Warning"));
			break;
		case DialogType::Error:
			ui->imgDialog->setPixmap(QString::fromUtf8(":/Error"));
			break;
	}

}

Dialog::Dialog(const char *message, const char *title, QDialogButtonBox::StandardButtons buttons, const char *iconName, QWidget *parent) : Dialog(message, title, buttons, parent)
{
	ui->imgDialog->setPixmap(QString::fromUtf8(iconName != nullptr ? iconName : ""));
}

Dialog::~Dialog()
{
	delete ui;
}

int Dialog::Show(const char *message, const char *title, QDialogButtonBox::StandardButtons buttons, DialogType icon)
{
	return Dialog(message, title, buttons, icon).exec();
}


void Dialog::on_buttonBox_clicked(QAbstractButton *button)
{
	QPushButton *btn = (QPushButton *)button;
	auto stdBtn = QDialogButtonBox::StandardButton::NoButton;

	if (btn == ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)) {
		stdBtn = QDialogButtonBox::StandardButton::Ok;
	}
	else if (btn == ui->buttonBox->button(QDialogButtonBox::StandardButton::Save)) {
		stdBtn = QDialogButtonBox::StandardButton::Save;
	}
	else if (btn == ui->buttonBox->button(QDialogButtonBox::StandardButton::SaveAll)) {
		stdBtn = QDialogButtonBox::StandardButton::SaveAll;
	}
	else if (btn == ui->buttonBox->button(QDialogButtonBox::StandardButton::Open)) {
		stdBtn = QDialogButtonBox::StandardButton::Open;
	}
	else if (btn == ui->buttonBox->button(QDialogButtonBox::StandardButton::Yes)) {
		stdBtn = QDialogButtonBox::StandardButton::Yes;
	}
	else if (btn == ui->buttonBox->button(QDialogButtonBox::StandardButton::YesToAll)) {
		stdBtn = QDialogButtonBox::StandardButton::YesToAll;
	}
	else if (btn == ui->buttonBox->button(QDialogButtonBox::StandardButton::No)) {
		stdBtn = QDialogButtonBox::StandardButton::No;
	}
	else if (btn == ui->buttonBox->button(QDialogButtonBox::StandardButton::NoToAll)) {
		stdBtn = QDialogButtonBox::StandardButton::NoToAll;
	}
	else if (btn == ui->buttonBox->button(QDialogButtonBox::StandardButton::Abort)) {
		stdBtn = QDialogButtonBox::StandardButton::Abort;
	}
	else if (btn == ui->buttonBox->button(QDialogButtonBox::StandardButton::Retry)) {
		stdBtn = QDialogButtonBox::StandardButton::Retry;
	}
	else if (btn == ui->buttonBox->button(QDialogButtonBox::StandardButton::Ignore)) {
		stdBtn = QDialogButtonBox::StandardButton::Ignore;
	}
	else if (btn == ui->buttonBox->button(QDialogButtonBox::StandardButton::Close)) {
		stdBtn = QDialogButtonBox::StandardButton::Close;
	}
	else if (btn == ui->buttonBox->button(QDialogButtonBox::StandardButton::Cancel)) {
		stdBtn = QDialogButtonBox::StandardButton::Cancel;
	}
	else if (btn == ui->buttonBox->button(QDialogButtonBox::StandardButton::Discard)) {
		stdBtn = QDialogButtonBox::StandardButton::Discard;
	}
	else if (btn == ui->buttonBox->button(QDialogButtonBox::StandardButton::Help)) {
		stdBtn = QDialogButtonBox::StandardButton::Help;
	}
	else if (btn == ui->buttonBox->button(QDialogButtonBox::StandardButton::Apply)) {
		stdBtn = QDialogButtonBox::StandardButton::Apply;
	}
	else if (btn == ui->buttonBox->button(QDialogButtonBox::StandardButton::Reset)) {
		stdBtn = QDialogButtonBox::StandardButton::Reset;
	}
	else if (btn == ui->buttonBox->button(QDialogButtonBox::StandardButton::RestoreDefaults)) {
		stdBtn = QDialogButtonBox::StandardButton::RestoreDefaults;
	}

	setResult((int)stdBtn);
	close();
}
