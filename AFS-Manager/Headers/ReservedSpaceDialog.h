#ifndef RESERVEDSPACEDIALOG_H
#define RESERVEDSPACEDIALOG_H

#include <QDialog>

namespace Ui
{
	class ReservedSpaceDialog;
}

class ReservedSpaceDialog : public QDialog
{
Q_OBJECT

public:
	explicit ReservedSpaceDialog(uint32_t currentReservedSpace, uint32_t newReservedSpace, QWidget *parent = nullptr);

	~ReservedSpaceDialog();

	uint32_t getNewReservedSpace() const;

private:
	Ui::ReservedSpaceDialog *ui;
};

#endif // RESERVEDSPACEDIALOG_H
