#include "ButtonEvent.h"

ButtonEvent::ButtonEvent() : button(nullptr), key(0), isEnabled(false), isPressed(false)
{
}

ButtonEvent::~ButtonEvent()
{
	delPointer(button);
}