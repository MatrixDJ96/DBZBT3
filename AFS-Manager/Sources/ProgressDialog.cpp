#include <ProgressDialog.h>
#include <ui_ProgressDialog.h>

#include <QIcon>
#include <QDebug>

using namespace Shared;

ProgressDialog::ProgressDialog(Type type, uint32_t max_value, uint32_t init_value, QWidget *parent) : QDialog(parent), ui(new Ui::ProgressDialog), type(type)
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
	else {
		setWindowTitle("AFS-Manager");
		ui->progressBar->setRange(0, 0);
	}

	/*constexpr int size = 64 * 1024 * 1024;
	buffer = new char[size];

	for (int i = 0; i < size; ++i) {
		buffer[i] = nullbyte;
	}*/

	qDebug() << "Created" << this << "->" << (type == Type::Export ? "export" : (type == Type::Import ? "import" : (type == Type::Rebuild ? "rebuild" : "loading")));
}

ProgressDialog::~ProgressDialog()
{
	delPointer(ui);
	//delPointerArray(buffer);

	qDebug() << "Destroyed" << this << "->" << (type == Type::Export ? "export" : (type == Type::Import ? "import" : (type == Type::Rebuild ? "rebuild" : "loading")));
}

void ProgressDialog::next()
{
	ui->progressBar->setValue(ui->progressBar->value() + 1);
}

void ProgressDialog::setLabel(const QString &text)
{
	ui->label->setText(text);
}
