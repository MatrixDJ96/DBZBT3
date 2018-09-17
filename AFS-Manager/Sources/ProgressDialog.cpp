#include <ProgressDialog.h>
#include <ui_ProgressDialog.h>

#include <QIcon>

using namespace Shared;

ProgressDialog::ProgressDialog(Type type, uint32_t max_value, uint32_t init_value, QWidget *parent) : QDialog(parent), ui(new Ui::ProgressDialog)
{
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	ui->setupUi(this);

	ui->progressBar->setMaximum(max_value);
	ui->progressBar->setValue(init_value);

	if (type == Type::Export) {
		setWindowTitle("Exporter");
		ui->pixmap->setPixmap(QPixmap(":/Exporter"));
		setWindowIcon(QIcon(*ui->pixmap->pixmap()));
	}
	else if (type == Type::Import) {
		setWindowTitle("Importer");
		ui->pixmap->setPixmap(QPixmap(":/Importer"));
		setWindowIcon(QIcon(*ui->pixmap->pixmap()));
	}
	else if (type == Type::Rebuild) {
		setWindowTitle("Rebuilder");
		ui->pixmap->setPixmap(QPixmap(":/Importer"));
		setWindowIcon(QIcon(*ui->pixmap->pixmap()));
		ui->progressBar->setRange(0, 0);
	}

	/*constexpr int size = 128 * 1024 * 1024;
	buffer = new char[size];

	for (int i = 0; i < size; ++i) {
		buffer[i] = nullbyte;
	}*/
}

ProgressDialog::~ProgressDialog()
{
	delPointer(ui);
	//delPointerArray(buffer);
}

void ProgressDialog::next()
{
	ui->progressBar->setValue(ui->progressBar->value() + 1);
}

void ProgressDialog::setLabel(const QString &text)
{
	ui->label->setText(text);
}
