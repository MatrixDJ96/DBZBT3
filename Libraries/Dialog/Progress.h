#ifndef PROGRESS_H
#define PROGRESS_H

#include "Dialog.h"

class Progress : public Dialog
{
Q_OBJECT

public:
	Progress(const std::string &title, const std::string &text, const QPixmap &pixmap);

	int getProgress() const;

	Dialog &setProgress(const int &value);

	Dialog &setMaximum(const int &maximum);

public slots:
	void next();
};

#endif //PROGRESS_H
