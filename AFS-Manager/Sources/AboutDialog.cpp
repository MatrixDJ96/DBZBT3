#include <AboutDialog.h>
#include <ui_AboutDialog.h>

#include <Shared.h>

using namespace Shared;

AboutDialog::AboutDialog(const QString &title, QWidget *parent) : QDialog(parent), ui(new Ui::AboutDialog)
{
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	ui->setupUi(this);

	auto text = ui->about->text();
	text.replace("${TITLE}", title);
	ui->about->setText(text);
}

AboutDialog::~AboutDialog()
{
	delPointer(ui);
}
