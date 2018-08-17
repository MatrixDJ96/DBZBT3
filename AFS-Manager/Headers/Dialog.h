#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtWidgets/QDialogButtonBox>

enum class DialogType
{
	Info, Warning, Error
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
	explicit Dialog(const char* message, const char* title, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::StandardButton::NoButton, QWidget *parent = nullptr);
	explicit Dialog(const char* message, const char* title, QDialogButtonBox::StandardButtons buttons, DialogType icon, QWidget *parent = nullptr);
	explicit Dialog(const char* message, const char* title, QDialogButtonBox::StandardButtons buttons, const char *iconName, QWidget *parent = nullptr);

	int static Show(const char* message, const char* title, QDialogButtonBox::StandardButtons buttons, DialogType icon);

	~Dialog();

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

private:
	Ui::Dialog *ui;
};

#endif // DIALOG_H
