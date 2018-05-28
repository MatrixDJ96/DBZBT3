#include "Dialog.h"
#include "ui_Dialog.h"

Dialog::Dialog(const std::string &title, const std::string &text, QWidget *parent) : QDialog(parent), reply(Reply::Exit), ui(new Ui::Dialog)
{
	ui->setupUi(this);

	this->setWindowFlag(Qt::WindowContextHelpButtonHint, false);

	this->setWindowTitle(QString::fromLocal8Bit(title.c_str())); // setTitle
	ui->textDialog->setText(QString::fromLocal8Bit(text.c_str())); // setNotice

	ui->progressBar->hide();
	ui->leftButton->hide();
	ui->centerButton->hide();
	ui->rightButton->hide();

	this->fixSize();
}

Dialog::~Dialog()
{
	delPointer(ui);
}

Dialog &Dialog::setTitle(const std::string &title)
{
	setWindowTitle(QString::fromLocal8Bit(title.c_str()));
	fixSize();
	return *this;
}

Dialog &Dialog::setNotice(const std::string &text)
{
	ui->textDialog->setText(QString::fromLocal8Bit(text.c_str()));
	fixSize();
	return *this;
}

Dialog &Dialog::setLeftButtonText(const std::string &text)
{
	ui->leftButton->setText(QString::fromLocal8Bit(text.c_str()));
	fixSize();
	return *this;
}

Dialog &Dialog::setCenterButtonText(const std::string &text)
{
	ui->centerButton->setText(QString::fromLocal8Bit(text.c_str()));
	fixSize();
	return *this;
}

Dialog &Dialog::setRightButtonText(const std::string &text)
{
	ui->rightButton->setText(QString::fromLocal8Bit(text.c_str()));
	fixSize();
	return *this;
}

void Dialog::fixSize()
{
	adjustSize();
	setMaximumWidth(this->width());
	setMaximumHeight(this->height());
}

const Reply &Dialog::getReply() const
{
	return reply;
}

void Dialog::on_leftButton_clicked()
{
	reply = Reply::Left;
	this->close();
}

void Dialog::on_centerButton_clicked()
{
	reply = Reply::Center;
	this->close();
}

void Dialog::on_rightButton_clicked()
{
	reply = Reply::Right;
	this->close();
}
// ---------- end dialog ----------
