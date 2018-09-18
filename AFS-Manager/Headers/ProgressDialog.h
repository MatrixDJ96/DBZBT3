#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>

#include <Shared.h>

namespace Ui
{
	class ProgressDialog;
}

class ProgressDialog : public QDialog
{
Q_OBJECT
public:
	explicit ProgressDialog(Shared::Type type, uint32_t max_value, uint32_t init_value = 0, QWidget *parent = nullptr);

	~ProgressDialog() override;

private:
	Ui::ProgressDialog *ui;
	char *buffer;
	Shared::Type type;

public slots:
	void next();

	void setLabel(const QString &text);
};

#endif // PROGRESSDIALOG_H
