#include "Progress.h"
#include "ui_Dialog.h"

Progress::Progress(const std::string &title, const std::string &text, const QPixmap &pixmap) : Dialog(title, text)
{
	ui->imgDialog->setPixmap(pixmap);
	ui->progressBar->setHidden(false);
}

int Progress::getProgress() const
{
	return ui->progressBar->value();
}

Dialog &Progress::setProgress(const int &value)
{
	ui->progressBar->setValue(value);
	return *this;
}

Dialog &Progress::setMaximum(const int &maximum)
{
	ui->progressBar->setMaximum(maximum);
	return *this;
}

void Progress::next()
{
	int value = ui->progressBar->value();
	ui->progressBar->setValue(++value);
	if (ui->progressBar->maximum() == value) {
		this->close();
	}
}
