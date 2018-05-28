#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QString>

#include "../Shared/Shared.h"

namespace Ui
{
	class Dialog;
}

enum class Button
{
	Left, Center, Right
};

enum class Reply
{
	Exit, Left, Center, Right
};

enum class Type
{
	Default, Error
};

class Dialog : public QDialog
{
Q_OBJECT
public:
	Dialog &setTitle(const std::string &title);

	Dialog &setNotice(const std::string &text);

protected:
	Dialog(const std::string &title, const std::string &text, QWidget *parent = nullptr);

	~Dialog();

	virtual Dialog &setLeftButtonText(const std::string &text);

	virtual Dialog &setCenterButtonText(const std::string &text);

	virtual Dialog &setRightButtonText(const std::string &text);

	void fixSize();

public:
	const Reply &getReply() const;

protected:
	Reply reply;
	Ui::Dialog *ui;

private slots:
	void on_leftButton_clicked();

	void on_centerButton_clicked();

	void on_rightButton_clicked();
};

#endif // DIALOG_H
