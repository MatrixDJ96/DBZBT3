#include <QDragEnterEvent>
#include <QMimeData>
#include <chrono>
#include <QMessageBox>
#include <QMetaObject>

#ifdef DBZBT3_DEBUG
#include <QDebug>
#endif

#include <MainWindow.h>
#include <ui_MainWindow.h>

#include "modifyreservedspacesdialog.h"
#include "aboutdialog.h"

using namespace Shared;


MainWindow::RebuilderThread::RebuilderThread(std::string newFilePath, std::vector<uint32_t> newReservedSpaces, AFS_File *afs) {
    this->newFilePath = newFilePath;
    this->newReservedSpaces = newReservedSpaces;
    this->afs = afs;
}

void MainWindow::RebuilderThread::run() {
    writtenBytes = afs->rebuild(newFilePath, newReservedSpaces);
}


enum columnID
{
    filename, size, reservedSpace, dateModified, address, afterRebuild
};

<<<<<<< HEAD:AFS-Manager/MainWindow.cpp
MainWindow::MainWindow(const std::string &name, const std::string &version, QWidget *parent) : QMainWindow(parent), afs(nullptr), unpacker(nullptr), ui(new Ui::MainWindow), progressUnpacker(nullptr), loadingDialog(new LoadingDialog)
=======
MainWindow::MainWindow(const std::string &name, const std::string &version, QWidget *parent) : QMainWindow(parent), afs(nullptr), unpacker(nullptr), ui(new Ui::MainWindow)//, progressUnpacker(nullptr)
>>>>>>> origin/testing:AFS-Manager/Sources/MainWindow.cpp
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

	// set title
	std::string title = name + " v" + version + " [WIP]";
#ifdef DBZBT3_DEBUG
	title += " DEBUG";
#endif
	this->setWindowTitle(title.c_str());

	// set other labels
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
    loadingDialog->deleteLater();
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
	auto end = std::chrono::steady_clock::now();

	ui->loadingTime->setText("Loading time: " + QString::number((double)std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0) + " sec");

	// TODO -> remove
	ui->usedRam->setText(("Used RAM: " + getStringSize(getUsedRam())).c_str());

    // Enable actions
    ui->actionRebuild->setEnabled(true);
    ui->actionImportFromFolder->setEnabled(true);
}

void MainWindow::connectCellChanged()
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
    ui->tableWidget->setColumnCount(6); // this enable also 'Address' columns... useful only for debugging purposes
    ui->tableWidget->setHorizontalHeaderLabels(QString("Filename;Size;Reserved space;Date modified;Address;Res space after rebuild").split(";"));

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
        QColor colorGreen(0,255,0);
        uint32_t reservedSpace = vfi[i + 1].address - fi.address;
        bool resSpaceTooBig = true;
        if(fi.size == reservedSpace || (fi.size % 2048 != 0 && fi.size / 2048 * 2048 + 2048 == reservedSpace))
            resSpaceTooBig = false;
        item = new QTableWidgetItem(QString::number(reservedSpace));
        if(resSpaceTooBig)
            populateRowCell(i, columnID::reservedSpace, item, &colorGreen);
        else
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

        // afterRebuild
        item = new QTableWidgetItem(QString::number(reservedSpace));
        if(resSpaceTooBig)
            populateRowCell(i, columnID::afterRebuild, item, &colorGreen);
        else
            populateRowCell(i, columnID::afterRebuild, item);
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
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

inline void MainWindow::populateRowCell(const int &row, const int &column, QTableWidgetItem *item, const QColor* color)
{
	item->setFlags(item->flags() ^ Qt::ItemIsEditable);
	item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    if(color) {
        item->setTextColor(*color);
    }
	ui->tableWidget->setItem(row, column, item);
}

void MainWindow::startExporting(const QList<uint32_t> &list, const std::string &path)
{
    /*
     * 1 element in the list -> no progress bar
     * 2+ elements in the list -> progress bar
     */

    if(list.size() == 0)
        return;

    if(list.size() == 1)
    {
        afs->exportFile(list[0],path);
        return;
    }

    /* With more than 1 element in the list */
	//unpacker = new Unpacker(afs, list, path);

	progressUnpacker = new Progress("Unpacker", std::string("Exporting ") + afs->getFilename(list[0]) + "...", QString::fromLocal8Bit(":/Unpack"));
	progressUnpacker->setMaximum(list.size());

	//connect(this, SIGNAL(exportFile()), unpacker, SLOT(exportFile()));
	//connect(unpacker, SIGNAL(progressFile(const char*)), SLOT(progressFile(const char*)));
	//connect(unpacker, SIGNAL(errorFile(const char*)), SLOT(errorFile(const char*)));
	//connect(unpacker, SIGNAL(exportDone()), this, SLOT(exportDone()));

	connect(progressUnpacker, SIGNAL(finished(int)), this, SLOT(exportAbort(int)));

	//unpacker->start();
	progressUnpacker->show();
	//unpacker->exportDone();


	/*Warning warning("Abort", "Cancel operation?");
	warning.exec();

	if (warning.getReply() == Reply::Left) {
		unpacker->terminate();
		done();
	}
	else {
		unpacker->resume();
	}*/
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
	if (path.empty()) {
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

	if (path.empty()) {
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

	if (path.empty()) {
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

void MainWindow::on_actionImportFromFolder_triggered()
{
    QString dirPath = QFileDialog::getExistingDirectory();
    if(dirPath == "")
        return;
    QDir dir(dirPath);
    QStringList filesList = dir.entryList(QDir::Files);
    std::vector<std::string> filesVector(filesList.size());
    int i = 0;
    for(QString s : filesList) {
        filesVector[i++] = s.toLocal8Bit().toStdString();
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
void MainWindow::progressFile(const char *name)
{
	progressUnpacker->next();
	progressUnpacker->setNotice(std::string("Exporting ") + name + "...");
	emit exportFile();
}

void MainWindow::errorFile(const char *name)
{
	Warning warning("Error", std::string("Unable to export\n") + name, Type::Error);
	warning.setLeftButtonText("Abort").setCenterButtonText("Retry").setRightButtonText("Skip");
	warning.exec();

	Reply reply = warning.getReply();
	if (reply == Reply::Left) {
		Message message("Abort", "TO DO");
		message.exec();
		emit exportFile();
	}
	else if (reply == Reply::Center) {
		emit exportFile();
	}
	else {
		unpacker->skip();
		emit exportFile();
	}
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
	delPointer(progressUnpacker);

	Message message("Unpacker", "Extraction completed!");
	message.exec();
}

void MainWindow::exportAbort(int i)
{
#ifdef DBZBT3_DEBUG
	qDebug() << "Risultato chiusura: " << i;
#endif


    //delete progressUnpacker;
    //progressUnpacker = nullptr;

    delPointer(progressUnpacker);
	//unpacker->quit();

	/*Warning warning("Abort", "Cancel operation?");
	warning.exec();

	if (warning.getReply() == Reply::Left) {
		//delPointer(unpacker);
		delPointer(progressUnpacker);
	}
	else {
		unpacker->start();
		progressUnpacker->show();
	}*/
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
		path = QFileDialog::getSaveFileName(this, "Save file", QString::fromLocal8Bit(afs->getFilename(list[0])), "File (*)").toLocal8Bit().toStdString();

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
        Message message("Error", "Before file import, reserved space must be increased...", Type::Error);
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
	int index = getSelectedRows()[0];

    AvailableSpaces as = afs->getAvailableSpaces(index);

    std::cout << as.before << " | " << as.after << '\n';
    std::cout.flush();

    if(as.before < 2048 && as.after < 2048) {
        QMessageBox qmb;
        qmb.setWindowTitle("Not enough space");
        qmb.setText("Not enough space from the previous and the following file.");
        qmb.exec();
        return;
    }

    QMessageBox qmb;
    qmb.setWindowTitle("Warning");
    qmb.setText("This action will automatically save the AFS file!");
    qmb.exec();

    ModifyReservedSpacesDialog mrsd(as.before, as.after);
    mrsd.exec();

    if(mrsd.spaceFromPrevious == 0 && mrsd.spaceFromFollowing == 0)
        return;

    std::cout << mrsd.spaceFromPrevious << " | " << mrsd.spaceFromFollowing << " (*)\n";
    std::cout.flush();

    if(mrsd.spaceFromFollowing != 0)
    {
        afs->enlargeFileBottom(index,mrsd.spaceFromFollowing);
    }

    if(mrsd.spaceFromPrevious != 0)
    {
        afs->enlargeFileTop(index,mrsd.spaceFromPrevious);
    }

    drawFileList();
}
// ---------- end context menu ----------

void MainWindow::slotCellChanged(const int &row, const int &column)
{
	QTableWidgetItem *item = ui->tableWidget->item(row, column);

    if(column == columnID::filename) {
        std::string text = item->text().toLocal8Bit().toStdString();
        if (text.size() > 32) {
            text = text.substr(0, 32);
            disconnect(connectionCellChanged);
            item->setText(text.c_str());
            connectCellChanged();
        }
        afs->changeFilename((uint32_t)row, text);
    } else if(column == columnID::afterRebuild) {
        QColor defaultColor(0,0,0);
        QColor colorGreen(0,255,0);
        QColor redColor(255,0,0);

        QTableWidgetItem *itemResSpace = ui->tableWidget->item(row, columnID::reservedSpace);
        uint32_t resSpace = itemResSpace->text().toUInt();
        uint32_t newResSpace = item->text().toUInt();
        if(newResSpace % 2048)
            newResSpace = newResSpace / 2048 * 2048 + 2048;
        item->setText(QString::number(newResSpace));
        item->setTextColor( newResSpace == resSpace ? defaultColor : (newResSpace > resSpace ? colorGreen : redColor));
    }
}

void MainWindow::on_actionAbout_triggered()
{
    AboutDialog ad;
    ad.exec();
}

void MainWindow::on_actionRebuild_triggered()
{
    std::string newFilePath = QFileDialog::getSaveFileName(this, "Save AFS file", "", "AFS file (*.afs)").toLocal8Bit().toStdString();
    if(newFilePath == "")
        return;

    uint32_t fileCount = ui->tableWidget->rowCount();
    std::vector<uint32_t> newReservedSpaces(fileCount);
    for(auto i=0; i<fileCount; ++i) {
        QTableWidgetItem* item = ui->tableWidget->item(i,columnID::afterRebuild);
        if(!item)
            std::cerr << "AHHHHHHHHHHHH!\n";
        newReservedSpaces[i] = item->text().toUInt();
    }

    loadingDialog->show();

    rebuilderThread = new RebuilderThread(newFilePath, newReservedSpaces, afs);
    connect(rebuilderThread,SIGNAL(finished()),this,SLOT(rebuildCompleted()));
    rebuilderThread->start();
}

void MainWindow::rebuildCompleted()
{
    QMessageBox qmb;
    uint32_t wb = rebuilderThread->writtenBytes;
    if(wb == 0) {
        qmb.setWindowTitle("Error");
        qmb.setText("Error saving the new afs.");
    } else {
        qmb.setWindowTitle("Rebuild complete");
        qmb.setText(QStringLiteral("Rebuild complete without errors.\n")+QString::number(wb)+" bytes written.");
    }
    loadingDialog->close();
    qmb.exec();
    disconnect(rebuilderThread,SIGNAL(finished()),this,SLOT(rebuildCompleted()));
    rebuilderThread->deleteLater();
}
