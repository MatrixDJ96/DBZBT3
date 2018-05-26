#include <QDragEnterEvent>
#include <QMimeData>
#include <chrono>

#include "MainWindow.h"
#include "ui_MainWindow.h"

using namespace Shared;

enum columnID
{
	filename, size, reservedSpace, dateModified, address
};

MainWindow::MainWindow(const std::string &name, const std::string &version, QWidget *parent) : QMainWindow(parent), afs(nullptr), unpacker(nullptr), ui(new Ui::MainWindow), progressUnpacker(nullptr)
{
	// create actions for context menu
	actionExportSelected = new QAction(this);
	actionExportSelected->setObjectName(QStringLiteral("actionExportSelected"));
	actionExportSelected->setIcon(QIcon(":/Export"));
	actionExportSelected->setText("Export");
	actionImportFile = new QAction(this);
	actionImportFile->setObjectName(QStringLiteral("actionImportFile"));
	actionImportFile->setIcon(QIcon(":/Import"));
	actionImportFile->setText("Import");
	actionModifyReservedSpace = new QAction(this);
	actionModifyReservedSpace->setObjectName(QStringLiteral("actionModifyReservedSpace"));
	actionModifyReservedSpace->setIcon(QIcon(":/Settings"));
	actionModifyReservedSpace->setText("Modify reserved space");

	// connect actions to context menu and setup ui
	ui->setupUi(this);

	// set title and other labels
	this->setWindowTitle(std::string(name + " v" + version + " [WIP]").c_str());
	ui->afsName->setText("Welcome to AFS-Manager!");
	ui->loadingTime->setText("");
	ui->afsSize->setText("");
	ui->usedRam->setText("");
	ui->afsFileCount->setText("");

	// set type of selection
	ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

	// define behaviour on right click
	ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->tableWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));

	// hide vertical header
	//ui->tableWidget->verticalHeader()->hide();

	setAcceptDrops(true);
}

MainWindow::~MainWindow()
{
	delPointer(afs);
	delPointer(ui);
}

void MainWindow::openAFS(const std::string &name)
{
	if (name.empty()) {
		return;
	}

	delPointer(afs);
	afs = new AFS_File(name);

	AFS_Error error = afs->getError();
	if (error.afsSize || error.descSize || error.infoSize || error.notAFS || error.unableToOpen) // should be only -> if (error.notAFS)
	{
		delPointer(afs);
		if (error.notAFS) {
			Message message("Error", "Invalid AFS", Type::Error);
			message.exec();
		}
		else if (error.unableToOpen) {
			Message message("Error", "Unable to open AFS", Type::Error);
			message.exec();
		}
		return;
	}

	if (error.coherency) {
		Warning warning("Error", "This AFS needs corrections\nDo you want to fix them?");
		warning.exec();
		if (warning.getReply() == Reply::Left) {
			if (!afs->commitFileDesc()) {
				delPointer(afs);
				Message message("Error", "Unable to fix AFS!", Type::Error);
				message.exec();
				return;
			}
		}
		else {
			delPointer(afs);
			return;
		}
	}

	ui->actionSave->setEnabled(true);
	ui->menuAFS->setEnabled(true);
	ui->menuAFL->setEnabled(true);

	auto start = std::chrono::steady_clock::now();
	drawFileList();
	auto stop = std::chrono::steady_clock::now();
	ui->loadingTime->setText("Loading time: " + QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() / 1000.0) + " sec");

	// TODO -> remove
	ui->usedRam->setText(("Used RAM: " + getStringSize(getUsedRam())).c_str());
}

inline void MainWindow::connectCellChanged()
{
	this->connectionCellChanged = connect(ui->tableWidget, SIGNAL(cellChanged(int, int)), this, SLOT(slotCellChanged(int, int)));
}

void MainWindow::adjustColumns()
{
	int padding = 20;

	ui->tableWidget->verticalHeader()->setFixedWidth(ui->tableWidget->verticalHeader()->sizeHint().width() + padding / 2);

	ui->tableWidget->resizeColumnsToContents();
	ui->tableWidget->setColumnWidth(columnID::filename, ui->tableWidget->columnWidth(columnID::filename) + padding);
	ui->tableWidget->setColumnWidth(columnID::size, ui->tableWidget->columnWidth(columnID::size) + padding);
	ui->tableWidget->setColumnWidth(columnID::reservedSpace, ui->tableWidget->columnWidth(columnID::reservedSpace) + padding);
	ui->tableWidget->setColumnWidth(columnID::dateModified, ui->tableWidget->columnWidth(columnID::dateModified) + padding);
	ui->tableWidget->setColumnWidth(columnID::address, ui->tableWidget->columnWidth(columnID::address) + padding);
}

void MainWindow::drawFileList()
{
	if (afs == nullptr) {
		return;
	} // prevent possible future errors

	disconnect(connectionCellChanged);

	ui->tableWidget->clear();

	int fileCount = int(afs->getFileCount()); // WARNING -> should be uint32_t

	// set text information
	ui->afsName->setText(afs->afsName.c_str());
	ui->afsFileCount->setText("File count: " + QString::number(fileCount));
	ui->afsSize->setText(("AFS size: " + getStringSize(afs->getAFSSize())).c_str());

	// generate columns
	ui->tableWidget->setColumnCount(5); // this enable also 'Address' columns... useful only for debugging purposes
	ui->tableWidget->setHorizontalHeaderLabels(QString("Filename;Size;Reserved space;Date modified;Address").split(";"));

	// set default row height and alignment
	ui->tableWidget->insertRow(0);
	ui->tableWidget->resizeRowToContents(0);
	int height = ui->tableWidget->rowHeight(0);
	ui->tableWidget->verticalHeader()->setDefaultSectionSize(height);
	ui->tableWidget->verticalHeader()->setDefaultAlignment(Qt::AlignHCenter);

	// generate rows
	ui->tableWidget->setRowCount(fileCount);

	// get raw afl
	const std::vector<FileDesc> vfd = afs->getFileDesc();
	const std::vector<FileInfo> vfi = afs->getFileInfo();

	FileDesc fd;
	FileInfo fi;

	for (int i = 0; i < fileCount; ++i) {
		// get required info
		fd = vfd[i];
		fi = vfi[i];

		// filename
		QTableWidgetItem *item = new QTableWidgetItem(QString::fromLocal8Bit(fd.name));
		populateRowCell(i, columnID::filename, item);
		item->setFlags(item->flags() ^ Qt::ItemIsEditable);
		item->setTextAlignment(item->textAlignment() ^ Qt::AlignHCenter);

		// size
		item = new QTableWidgetItem(QString::number(fd.size));
		populateRowCell(i, columnID::size, item);

		// reservedSpace
		item = new QTableWidgetItem(QString::number(vfi[i + 1].address - fi.address));
		populateRowCell(i, columnID::reservedSpace, item);

		// date
		QString date = fd.day < 10 ? "0" + QString::number(fd.day) : QString::number(fd.day);
		date += "-";
		date += fd.month < 10 ? "0" + QString::number(fd.month) : QString::number(fd.month);
		date += "-";
		date += fd.year < 1000 ? fd.year < 100 ? fd.year < 10 ? "000" + QString::number(fd.year) : "00" + QString::number(fd.year) : "0" + QString::number(fd.year) : QString::number(fd.year);
		date += " ";
		date += fd.hour < 10 ? "0" + QString::number(fd.hour) : QString::number(fd.hour);
		date += ":";
		date += fd.minute < 10 ? "0" + QString::number(fd.minute) : QString::number(fd.minute);
		date += ":";
		date += fd.second < 10 ? "0" + QString::number(fd.second) : QString::number(fd.second);
		item = new QTableWidgetItem(date);
		populateRowCell(i, columnID::dateModified, item);

		// fileAddress
		item = new QTableWidgetItem(QString::number(fi.address));
		populateRowCell(i, columnID::address, item);
	}

	// adjust columns
	adjustColumns();

	connectCellChanged();
}

QList<uint32_t> MainWindow::getSelectedRows() const
{
	QList<uint32_t> ret;

	QModelIndexList selection = ui->tableWidget->selectionModel()->selectedRows();
	int selectionSize = selection.size();

	QModelIndex index;
	for (int i = 0; i < selectionSize; ++i) {
		index = selection.at(i);
		ret.append(index.row());
	}

	return ret;
}

inline void MainWindow::populateRowCell(const int &row, const int &column, QTableWidgetItem *item)
{
	item->setFlags(item->flags() ^ Qt::ItemIsEditable);
	item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	ui->tableWidget->setItem(row, column, item);
}

void MainWindow::startExporting(const QList<uint32_t> &list, const std::string &path) // TODO -> lock function on MainWindow during unpacker execution
{
	unpacker = new Unpacker(afs, list, path);

	progressUnpacker = new Progress("Unpacker", "Exporting files...", QString::fromLocal8Bit(":/Unpack"));
	progressUnpacker->setMaximum(list.size());

	connect(this, SIGNAL(exportFile()), unpacker, SLOT(exportFile()));
	connect(unpacker, SIGNAL(progressFile()), SLOT(progressFile()));
	connect(unpacker, SIGNAL(exportDone()), this, SLOT(exportDone()));

	progressUnpacker->show();
	unpacker->start();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
	event->accept();
}


void MainWindow::dropEvent(QDropEvent *event)
{
	QList<QUrl> urls = event->mimeData()->urls();
	if (urls.size() == 1) {
		std::string name = urls[0].path().toLocal8Bit().toStdString();

#ifdef _WIN32
		if (name[0] == '/') {
			name = name.substr(1, name.size());
		}
#elif __linux
		// TODO
#endif

		openAFS(name);
	}

}

// ---------- menu bar ----------
void MainWindow::on_actionExit_triggered()
{
	this->close();
}

void MainWindow::on_actionExportCommon_triggered()
{
	if (afs == nullptr) {
		return;
	} // prevent possible future errors

	std::string path = QFileDialog::getSaveFileName(this, "Save common AFL file", getFileBaseName(afs->afsName, "afs").c_str(), "Common AFL file (*.afl)").toLocal8Bit().toStdString();
	if (path == "") {
		return;
	}

	if (afs->exportCommon(path)) {
		Message message("Success", "Common AFL exported");
		message.exec();
	}
	else {
		Message message("Error", "Unable to export common AFL!", Type::Error);
		message.exec();
	}
}

void MainWindow::on_actionExportRAW_triggered()
{
	if (afs == nullptr) {
		return;
	} // prevent possible future errors

	std::string path = QFileDialog::getSaveFileName(this, "Save RAW AFL file", getFileBaseName(afs->afsName, "afs").c_str(), "RAW AFL file (*.raw-afl)").toLocal8Bit().toStdString();

	if (path.empty()) {
		return;
	}

	if (afs->exportRAW(path)) {
		Message message("Success", "RAW AFL exported");
		message.exec();
	}
	else {
		Message message("Error", "Unable to export RAW AFL!", Type::Error);
		message.exec();
	}
}

void MainWindow::on_actionImportCommon_triggered()
{
	if (afs == nullptr) {
		return;
	} // prevent possible future errors

	std::string path = QFileDialog::getOpenFileName().toLocal8Bit().toStdString();

	if (path == "") {
		return;
	}

	if (afs->importCommon(path)) {
		Message message("Success", "Common AFL imported");
		message.exec();
		drawFileList();
	}
	else {
		Message message("Error", "Unable to import common AFL!", Type::Error);
		message.exec();
	}
}

void MainWindow::on_actionImportRAW_triggered()
{
	if (afs == nullptr) {
		return;
	} // prevent possible future errors

	std::string path = QFileDialog::getOpenFileName().toLocal8Bit().toStdString();

	if (path == "") {
		return;
	}

	if (afs->importRAW(path)) {
		Message message("Success", "RAW AFL imported");
		message.exec();
		drawFileList();
	}
	else {
		Message message("Error", "Unable to import RAW AFL!", Type::Error);
		message.exec();
	}
}

void MainWindow::on_actionOpen_triggered()
{
	openAFS(QFileDialog::getOpenFileName().toLocal8Bit().toStdString());
}

void MainWindow::on_actionSave_triggered()
{
	if (afs == nullptr) {
		return;
	} // prevent possible future errors

	if (afs->commitFileInfo() && afs->commitFileDesc()) {
		Message message("Success", "AFS saved");
		message.exec();
		drawFileList(); // TODO -> ?
	}
	else {
		Message message("Error", "Unable to save AFS!", Type::Error);
		message.exec();
	}
}
// ---------- end menu bar ----------

void MainWindow::on_actionUnpackAFS_triggered()
{
	if (afs == nullptr) {
		return; // prevent possible future errors
	}

	std::string path = QFileDialog::getExistingDirectory().toLocal8Bit().toStdString();
	if (path.empty()) {
		return;
	}

	QList<uint32_t> list;
	uint32_t size = afs->getFileCount();
	for (uint32_t i = 0; i < size; ++i) {
		list.append(i);
	}

	startExporting(list, path);
}

// ---------- start unpacker connection ----------
void MainWindow::progressFile()
{
	progressUnpacker->next();
	emit exportFile();
}

/*void MainWindow::abort()
{
	Warning warning("Abort", "Cancel operation?");
	warning.exec();

	if (warning.getReply() == Reply::Left) {
		unpacker->terminate();
		done();
	}
	else {
		unpacker->resume();
	}
}*/

void MainWindow::exportDone()
{
	delPointer(unpacker);
}

/*void MainWindow::error(const QString &filename, bool multi)
{
	Warning warning("Error", "Unable to export\n" + filename.toStdString(), Type::Error);
	warning.setLeftButtonText("Abort").setCenterButtonText("Retry").setRightButtonText("Skip");
	if (multi) {
		warning.setButtonEvent(Button::Right, Qt::Key_Control, "Skip all");
	}
	warning.exec();

	Reply reply = warning.getReply();
	if (reply == Reply::Left) {
		abort();
	}
	else if (reply == Reply::Center) {
		unpacker->resume();
	}
	else if (reply == Reply::Right) {
		ButtonEvent be = warning.getButtonEvent();
		if (be.isEnabled && be.isPressed) {
			unpacker->skipAll();
		}
		else {
			unpacker->skip();
		}
	}
}*/
// ---------- end unpacker connection ----------

// ---------- context menu ----------
void MainWindow::showContextMenu(const QPoint &pos)
{
	QMenu contextMenu;
	int size = getSelectedRows().size();
	if (size >= 1) {
		contextMenu.addAction(actionExportSelected);
		if (size == 1) {
			contextMenu.addAction(actionImportFile);
			contextMenu.addAction(actionModifyReservedSpace);
		}
	}
	contextMenu.exec(ui->tableWidget->mapToGlobal(pos));
}

void MainWindow::on_actionExportSelected_triggered()
{
	QList<uint32_t> list = getSelectedRows();

	std::string path;

	if (list.size() == 1) {
		path = QFileDialog::getSaveFileName(this, "Save file", QString::fromLocal8Bit(afs->getFileDesc()[list[0]].name), "File (*)").toLocal8Bit().toStdString();
	}
	else {
		path = QFileDialog::getExistingDirectory().toLocal8Bit().toStdString();
	}

	if (path.empty()) {
		return;
	}

	startExporting(list, path);
}

void MainWindow::on_actionImportFile_triggered()
{
	uint32_t index = getSelectedRows()[0];

	std::string path = QFileDialog::getOpenFileName().toLocal8Bit().toStdString();

	if (path.empty()) {
		return;
	}

	// TODO -> check return value
	uint8_t result = afs->importFile(index, path);
	if (result == 2) {
		Message message("Error", "Before file import, reserved space must be increased...\nFunction not yet implemented", Type::Error);
		message.exec();
	}
	else if (result == 1) {
		Message message("Success", "File imported");
		message.exec();
		drawFileList(); // TODO -> ?
	}
	else if (result == 0) {
		Message message("Error", "Unable to import file!", Type::Error);
		message.exec();
	}
}

void MainWindow::on_actionModifyReservedSpace_triggered()
{
}
// ---------- end context menu ----------

void MainWindow::slotCellChanged(const int &row, const int &column)
{
	QTableWidgetItem *item = ui->tableWidget->item(row, column);
	std::string text = item->text().toLocal8Bit().toStdString();
	if (text.size() > 32) {
		text = text.substr(0, 32);
		disconnect(connectionCellChanged);
		item->setText(text.c_str());
		connectCellChanged();
	}
	afs->changeFilename((uint32_t)row, text);
}


