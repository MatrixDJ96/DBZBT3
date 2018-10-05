#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QMessageBox>

namespace Shared
{
	QMessageBox::StandardButton ShowInfo(QWidget *parent, const QString &title, const QString &text, QMessageBox::StandardButtons buttons = QMessageBox::Ok, QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

	QMessageBox::StandardButton ShowWarning(QWidget *parent, const QString &title, const QString &text, QMessageBox::StandardButtons buttons = QMessageBox::Ok, QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

	QMessageBox::StandardButton ShowError(QWidget *parent, const QString &title, const QString &text, QMessageBox::StandardButtons buttons = QMessageBox::Ok, QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);
}

#endif // MESSAGEBOX_H
