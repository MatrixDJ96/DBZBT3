#include <ReservedSpaceDialog.h>
#include <ui_ReservedSpaceDialog.h>

#include <Shared.h>

using namespace Shared;

ReservedSpaceDialog::ReservedSpaceDialog(uint32_t currentReservedSpace, uint32_t newReservedSpace, QWidget *parent) : QDialog(parent), ui(new Ui::ReservedSpaceDialog)
{
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	ui->setupUi(this);

	ui->spinBoxCurrent->setValue(currentReservedSpace);
	ui->spinBoxNew->setValue(newReservedSpace);
}

ReservedSpaceDialog::~ReservedSpaceDialog()
{
	delPointer(ui);
}

uint32_t ReservedSpaceDialog::getNewReservedSpace() const
{
	return ui->spinBoxNew->value();
}
