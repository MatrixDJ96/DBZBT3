#include <QKeyEvent>

#include "Warning.h"
#include "ui_Dialog.h"

Warning::Warning(const std::string &title, const std::string &text, const Type &type) : Dialog(title, text), buttonEvent()
{
	if (type == Type::Default) {
		ui->imgDialog->setPixmap(QString::fromUtf8(":Warning"));
	}
	else if (type == Type::Error) {
		ui->imgDialog->setPixmap(QString::fromUtf8(":Error"));
	}

	ui->leftButton->setHidden(false);
	ui->leftButton->setText(QString::fromLocal8Bit("Yes"));
	ui->rightButton->setHidden(false);
	ui->rightButton->setText(QString::fromLocal8Bit("No"));
}

Warning::~Warning()
{
}

const ButtonEvent &Warning::getButtonEvent() const
{
	return buttonEvent;
}

Warning &Warning::setLeftButtonText(const std::string &leftButtonText)
{
	Dialog::setLeftButtonText(leftButtonText);
	return *this;
}

Warning &Warning::setCenterButtonText(const std::string &centerButtonText)
{
	ui->centerButton->setHidden(false);
	Dialog::setCenterButtonText(centerButtonText);
	return *this;
}

Warning &Warning::setRightButtonText(const std::string &rightButtonText)
{
	Dialog::setRightButtonText(rightButtonText);
	return *this;
}

Warning &Warning::setButtonEvent(const Button &button, const int &key, const std::string &buttonText)
{
	buttonEvent.button = new Button(button);
	buttonEvent.newText = buttonText;

	if (button == Button::Left) {
		buttonEvent.oldText = ui->leftButton->text().toLocal8Bit().constData();
	}
	else if (button == Button::Center) {
		buttonEvent.oldText = ui->centerButton->text().toLocal8Bit().constData();
	}
	else if (button == Button::Right) {
		buttonEvent.oldText = ui->rightButton->text().toLocal8Bit().constData();
	}

	buttonEvent.key = key;
	buttonEvent.isEnabled = true;

	return *this;
}

Warning &Warning::setButtonEvent(const ButtonEvent &buttonEvent)
{
	this->buttonEvent = buttonEvent;
	return *this;
}

void Warning::keyPressEvent(QKeyEvent *event)
{
	if (buttonEvent.isEnabled) {
		if (event->key() == buttonEvent.key) {
			buttonEvent.isPressed = true;
			if (*buttonEvent.button == Button::Left) {
				setLeftButtonText(buttonEvent.newText);
			}
			else if (*buttonEvent.button == Button::Center) {
				setCenterButtonText(buttonEvent.newText);
			}
			else if (*buttonEvent.button == Button::Right) {
				setRightButtonText(buttonEvent.newText);
			}
		}
	}
}

void Warning::keyReleaseEvent(QKeyEvent *event)
{
	if (buttonEvent.isEnabled) {
		if (event->key() == buttonEvent.key) {
			buttonEvent.isPressed = false;
			if (*buttonEvent.button == Button::Left) {
				setLeftButtonText(buttonEvent.oldText);
			}
			else if (*buttonEvent.button == Button::Center) {
				setCenterButtonText(buttonEvent.oldText);
			}
			else if (*buttonEvent.button == Button::Right) {
				setRightButtonText(buttonEvent.oldText);
			}
		}
	}
}
