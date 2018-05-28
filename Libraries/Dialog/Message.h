#ifndef MESSAGE_H
#define MESSAGE_H

#include "Dialog.h"

class Message : public Dialog
{
Q_OBJECT

public:
	Message(const std::string &title, const std::string &text, const Type &type = Type::Default);

	~Message();

	Message &setLeftButtonText(const std::string &leftButtonText);

	Message &setRightButtonText(const std::string &rightButtonText);
};

#endif //MESSAGE_H
