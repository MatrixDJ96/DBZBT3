#include <QKeyEvent>

#include "Dialog.h"
#include "ui_Dialog.h"

// ---------- buttonevent ----------
ButtonEvent::ButtonEvent() : button(NULL), key(0), isEnabled(false), isPressed(false)
{}

ButtonEvent::~ButtonEvent()
{
	delPointer(button);
}
// ---------- end buttonevent ----------

// ---------- dialog ----------
Dialog::Dialog(const std::string& title, const std::string& text, QWidget* parent) : QDialog(parent), reply(Reply::Exit), ui(new Ui::Dialog)
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

Dialog& Dialog::setTitle(const std::string& title)
{
	setWindowTitle(QString::fromLocal8Bit(title.c_str()));
	fixSize();
	return *this;
}

Dialog& Dialog::setNotice(const std::string& text)
{
	ui->textDialog->setText(QString::fromLocal8Bit(text.c_str()));
	fixSize();
	return *this;
}

Dialog& Dialog::setLeftButtonText(const std::string& leftButtonText)
{
	ui->leftButton->setText(QString::fromLocal8Bit(leftButtonText.c_str()));
	fixSize();
	return *this;
}

Dialog& Dialog::setCenterButtonText(const std::string& centerButtonText)
{
	ui->centerButton->setText(QString::fromLocal8Bit(centerButtonText.c_str()));
	fixSize();
	return *this;
}

Dialog& Dialog::setRightButtonText(const std::string& rightButtonText)
{
	ui->rightButton->setText(QString::fromLocal8Bit(rightButtonText.c_str()));
	fixSize();
	return *this;
}

const Reply& Dialog::getReply() const
{
	return reply;
}

void Dialog::fixSize()
{
	adjustSize();
	setMaximumWidth(this->width());
	setMaximumHeight(this->height());
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

// ---------- message ----------
Message::Message(const std::string& title, const std::string& text, const Type& type) : Dialog(title, text)
{
	if (type == Type::Default)
		ui->imgDialog->setPixmap(QString::fromUtf8(":Info"));
	else if (type == Type::Error)
		ui->imgDialog->setPixmap(QString::fromUtf8(":Error"));

	ui->rightButton->setHidden(false);
	ui->rightButton->setText(QString::fromLocal8Bit("OK"));
}

Message::~Message()
{
	delPointer(ui);
}

Message& Message::setLeftButtonText(const std::string& leftButtonText)
{
	Dialog::setLeftButtonText(leftButtonText);
	return *this;
}

Message& Message::setRightButtonText(const std::string & rightButtonText)
{
	Dialog::setRightButtonText(rightButtonText);
	return *this;
}
// ---------- end message ----------

// ---------- warning ----------
Warning::Warning(const std::string& title, const std::string& text, const Type& type) : Dialog(title, text), buttonEvent()
{
	if (type == Type::Default)
		ui->imgDialog->setPixmap(QString::fromUtf8(":Warning"));
	else if (type == Type::Error)
		ui->imgDialog->setPixmap(QString::fromUtf8(":Error"));

	ui->leftButton->setHidden(false);
	ui->leftButton->setText(QString::fromLocal8Bit("Yes"));
	ui->rightButton->setHidden(false);
	ui->rightButton->setText(QString::fromLocal8Bit("No"));
}

Warning::~Warning()
{}

const ButtonEvent& Warning::getButtonEvent() const
{
	return buttonEvent;
}

Warning& Warning::setLeftButtonText(const std::string& leftButtonText)
{
	Dialog::setLeftButtonText(leftButtonText);
	return *this;
}

Warning& Warning::setCenterButtonText(const std::string& centerButtonText)
{
	ui->centerButton->setHidden(false);
	Dialog::setCenterButtonText(centerButtonText);
	return *this;
}

Warning& Warning::setRightButtonText(const std::string& rightButtonText)
{
	Dialog::setRightButtonText(rightButtonText);
	return *this;
}

Warning& Warning::setButtonEvent(const Button& button, const int& key, const std::string& buttonText)
{
	buttonEvent.button = new Button(button);
	buttonEvent.newText = buttonText;

	if (button == Button::Left)
		buttonEvent.oldText = ui->leftButton->text().toLocal8Bit().constData();
	else if (button == Button::Center)
		buttonEvent.oldText = ui->centerButton->text().toLocal8Bit().constData();
	else if (button == Button::Right)
		buttonEvent.oldText = ui->rightButton->text().toLocal8Bit().constData();

	buttonEvent.key = key;
	buttonEvent.isEnabled = true;

	return *this;
}

Warning& Warning::setButtonEvent(const ButtonEvent& buttonEvent)
{
	this->buttonEvent = buttonEvent;
	return *this;
}

void Warning::keyPressEvent(QKeyEvent* event)
{
	if (buttonEvent.isEnabled)
	{
		if (event->key() == buttonEvent.key)
		{
			buttonEvent.isPressed = true;
			if (*buttonEvent.button == Button::Left)
				setLeftButtonText(buttonEvent.newText);
			else if (*buttonEvent.button == Button::Center)
				setCenterButtonText(buttonEvent.newText);
			else if (*buttonEvent.button == Button::Right)
				setRightButtonText(buttonEvent.newText);
		}
	}
}

void Warning::keyReleaseEvent(QKeyEvent* event)
{
	if (buttonEvent.isEnabled)
	{
		if (event->key() == buttonEvent.key)
		{
			buttonEvent.isPressed = false;
			if (*buttonEvent.button == Button::Left)
				setLeftButtonText(buttonEvent.oldText);
			else if (*buttonEvent.button == Button::Center)
				setCenterButtonText(buttonEvent.oldText);
			else if (*buttonEvent.button == Button::Right)
				setRightButtonText(buttonEvent.oldText);
		}
	}
}
// ---------- end warning ----------

// ---------- progress ----------
Progress::Progress(const std::string& title, const std::string text, const QPixmap& pixmap) : Dialog(title, text)
{
	ui->imgDialog->setPixmap(pixmap);
	ui->progressBar->setHidden(false);
}

int Progress::getProgress() const
{
	return ui->progressBar->value();
}

Dialog & Progress::setProgress(const int& value)
{
	ui->progressBar->setValue(value);
	return *this;
}

Dialog& Progress::setMaximum(const int& maximum)
{
	ui->progressBar->setMaximum(maximum);
	return *this;
}

void Progress::next()
{
	int value = ui->progressBar->value();
	ui->progressBar->setValue(value + 1);
	if (ui->progressBar->maximum() == value)
		this->close();
}
// ---------- end progress ----------
