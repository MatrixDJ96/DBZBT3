#include <AboutDialog.h>
#include <ui_AboutDialog.h>

#include <Shared.h>

using namespace Shared;

AboutDialog::AboutDialog(const QString &title, QWidget *parent) : QDialog(parent), ui(new Ui::AboutDialog)
{
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	ui->setupUi(this);

	ui->about->setText(ui->about->text().replace("${TITLE}", title));
}

AboutDialog::~AboutDialog()
{
	delPointer(ui);
}
