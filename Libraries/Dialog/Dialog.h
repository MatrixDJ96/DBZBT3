#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QString>

#include "../Shared/Shared.h"

namespace Ui
{
	class Dialog;
}

enum class Button
{
	Left,
	Center,
	Right
};

enum class Reply
{
	Exit,
	Left,
	Center,
	Right
};

enum class Type
{
	Default,
	Error
};

struct ButtonEvent
{
    ButtonEvent();
    ~ButtonEvent();

	Button* button;
	std::string newText;
	std::string oldText;
	int key;
	bool isEnabled;
	bool isPressed;
};

class Dialog : public QDialog
{
	Q_OBJECT
public:
	Dialog& setTitle(const std::string& title);
	Dialog& setNotice(const std::string& text);

protected:
	Dialog(const std::string& title, const std::string& text, QWidget* parent = 0);
	~Dialog();
	
	Dialog& setLeftButtonText(const std::string& leftButtonText);
	Dialog& setCenterButtonText(const std::string& centerButtonText);
	Dialog& setRightButtonText(const std::string& rightButtonText);

public:
	const Reply& getReply() const;
	void fixSize();

protected:
	Reply reply;
	Ui::Dialog* ui;

private slots:
	void on_leftButton_clicked();
	void on_centerButton_clicked();
	void on_rightButton_clicked();
};

class Message : public Dialog
{
public:
	Message(const std::string& title, const std::string& text, const Type& type = Type::Default);
	~Message();

	Message& setLeftButtonText(const std::string& leftButtonText);
	Message& setRightButtonText(const std::string& rightButtonText);
};

class Warning : public Dialog
{
public:
	Warning(const std::string& title, const std::string& text, const Type& type = Type::Default);
	~Warning();

	const ButtonEvent& getButtonEvent() const;

	Warning& setLeftButtonText(const std::string& leftButtonText);
	Warning& setCenterButtonText(const std::string& centerButtonText);
	Warning& setRightButtonText(const std::string& rightButtonText);

	Warning& setButtonEvent(const Button& button, const int& key, const std::string& buttonText); // safer
	Warning& setButtonEvent(const ButtonEvent& buttonEvent); // no check, copy only

private:
	void keyPressEvent(QKeyEvent* event);
	void keyReleaseEvent(QKeyEvent* event);

private:
	ButtonEvent buttonEvent;
};

class Progress : public Dialog
{
	Q_OBJECT

public:
	Progress(const std::string& title, const std::string text, const QPixmap& pixmap);

	int getProgress() const;

	Dialog& setProgress(const int& value);
	Dialog& setMaximum(const int& maximum);

public slots:
	void next();
};

#endif // DIALOG_H
