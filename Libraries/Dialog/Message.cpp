#include "Message.h"
#include "ui_Dialog.h"

Message::Message(const std::string &title, const std::string &text, const Type &type) : Dialog(title, text)
{
	if (type == Type::Default) {
		ui->imgDialog->setPixmap(QString::fromUtf8(":Info"));
	}
	else if (type == Type::Error) {
		ui->imgDialog->setPixmap(QString::fromUtf8(":Error"));
	}

	ui->rightButton->setHidden(false);
	ui->rightButton->setText(QString::fromLocal8Bit("OK"));
}

Message::~Message()
{
	delPointer(ui);
}

Message &Message::setLeftButtonText(const std::string &leftButtonText)
{
	Dialog::setLeftButtonText(leftButtonText);
	return *this;
}

Message &Message::setRightButtonText(const std::string &rightButtonText)
{
	Dialog::setRightButtonText(rightButtonText);
	return *this;
}
