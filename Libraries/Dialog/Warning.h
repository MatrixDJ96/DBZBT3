#ifndef WARNING_H
#define WARNING_H

#include "ButtonEvent.h"

class Warning : public Dialog
{
Q_OBJECT

public:
	Warning(const std::string &title, const std::string &text, const Type &type = Type::Default);

	~Warning();

	const ButtonEvent &getButtonEvent() const;

	Warning &setLeftButtonText(const std::string &leftButtonText);

	Warning &setCenterButtonText(const std::string &centerButtonText);

	Warning &setRightButtonText(const std::string &rightButtonText);

	Warning &setButtonEvent(const Button &button, const int &key, const std::string &buttonText); // safer

	Warning &setButtonEvent(const ButtonEvent &buttonEvent); // no check, copy only

private:
	void keyPressEvent(QKeyEvent *event);

	void keyReleaseEvent(QKeyEvent *event);

private:
	ButtonEvent buttonEvent;
};

#endif //WARNING_H
