#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtWidgets/QDialogButtonBox>

enum class DialogType
{
	Info, Warning, Error, Progress
};

namespace Ui
{
	class Dialog;
}

class Dialog : public QDialog
{
Q_OBJECT

public:
	explicit Dialog(QWidget *parent = nullptr);

	explicit Dialog(DialogType type, QDialogButtonBox::StandardButtons buttons, const char *filename = nullptr, QWidget *parent = nullptr);

	~Dialog();

private:
	Ui::Dialog *ui;
};

#endif // DIALOG_H
