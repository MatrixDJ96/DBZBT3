#ifndef BUTTONEVENT_H
#define BUTTONEVENT_H

#include "Dialog.h"

class ButtonEvent
{
public:
	ButtonEvent();

	~ButtonEvent();

public:
	Button *button;
	std::string newText;
	std::string oldText;
	int key;
	bool isEnabled;
	bool isPressed;
};

#endif //BUTTONEVENT_H
