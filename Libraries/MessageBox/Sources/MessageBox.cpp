#include <MessageBox.h>

QMessageBox::StandardButton Shared::ShowInfo(QWidget *parent, const QString &title, const QString &text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
	QMessageBox msb(QMessageBox::NoIcon, title, text, buttons, parent, Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
	msb.setDefaultButton(defaultButton);
	msb.setIconPixmap(QString::fromUtf8(":/Info"));
	return (QMessageBox::StandardButton)msb.exec();
}

QMessageBox::StandardButton Shared::ShowWarning(QWidget *parent, const QString &title, const QString &text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
	QMessageBox msb(QMessageBox::NoIcon, title, text, buttons, parent, Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
	msb.setDefaultButton(defaultButton);
	msb.setIconPixmap(QString::fromUtf8(":/Warning"));
	return (QMessageBox::StandardButton)msb.exec();
}

QMessageBox::StandardButton Shared::ShowError(QWidget *parent, const QString &title, const QString &text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
	QMessageBox msb(QMessageBox::NoIcon, title, text, buttons, parent, Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
	msb.setDefaultButton(defaultButton);
	msb.setIconPixmap(QString::fromUtf8(":/Error"));
	return (QMessageBox::StandardButton)msb.exec();
}
