#include <MainWindow.h>
#include <ui_MainWindow.h>

#include <QFileDialog>
#include <QMimeData>

#include <algorithm>
#include <chrono>

#include <AboutDialog.h>
#include <MessageBox.h>
#include <ProgressDialog.h>
#include <ReservedSpaceDialog.h>

using namespace Shared;

enum columnID
{
	number, filename, size, reservedSpace, afterRebuild, dateModified, address
};

MainWindow::MainWindow(const std::string &name, const std::string &version, QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), afs(nullptr), enableCellChanged(false), oldWorker(nullptr)
{
	// connect actions to context menu and setup ui
	ui->setupUi(this);

	// set title
	std::string title = name + " v" + version + " [WIP]";
#ifdef DBZBT3_DEBUG
	title += " DEBUG";
#endif
	this->setWindowTitle(title.c_str());

	// disable menu tools
	ui->menuTools->setEnabled(false);

	setAcceptDrops(true);
}

MainWindow::~MainWindow()
{
	delPointer(afs);
	delPointer(ui);
}

void MainWindow::openAFS(const std::string &path, bool firstCall)
{
	if (path.empty()) {
		return;
	}

	auto *afs = new AFS_File(path);
	auto error = afs->getError();

	if (error.badStream || error.invalidAddress || error.notAFS || error.unableToOpen) {
		delPointer(afs);
		if (error.badStream) {
			ShowError(this, "Error", "Error while reading file");
		}
		else if (error.notAFS || error.invalidAddress) {
			ShowError(this, "Error", "This is not a valid AFS");
		}
		else if (error.unableToOpen) {
			ShowError(this, "Error", "Unable to open AFS");
		}
		return;
	}

	if (error.corruptedContent && ShowError(this, "Error", "This AFS seems to be corrupted...\nDo you want to load anyway?", QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
		delPointer(afs);
		return;
	}

	if (error.coherency || error.invalidDesc || error.overSize) {
		if (!firstCall || ShowWarning(this, "Error", "This AFS needs to be fixed\nDo you want to do it?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
			bool result = false;

			if (error.invalidDesc) {
				if ((result = afs->fixInvalidDesc())) {
					delPointer(afs);
					ShowInfo(this, "Success", "AFS descriptor regenerated successfully!");
					openAFS(path, false);
					return;
				}
			}
			else {
				if (error.coherency) {
					result = afs->commitFileInfo() && afs->commitFileDesc();
				}
				if (error.overSize && (error.coherency ? result : true)) {
					result = afs->fixOverSize();
				}
			}

			if (result && firstCall) {
				ShowInfo(this, "Success", "AFS fixed successfully!");
			}
			else if (!result) {
				delPointer(afs);
				ShowError(this, "Error", "Unable to fix AFS...");
				return;
			}
		}
		else {
			delPointer(afs);
			return;
		}
	}


	ui->menuTools->setEnabled(false);
	delPointer(this->afs);
	this->afs = afs;
	ui->menuTools->setEnabled(true);

	auto start = std::chrono::steady_clock::now();

	//setCursor(QCursor(Qt::WaitCursor));

	drawFileList();
	updateFreeSpaceLabel();

	//setCursor(QCursor(Qt::ArrowCursor));

	auto end = std::chrono::steady_clock::now();

	ui->loadingTime->setText("Loading time: " + QString::number((double)std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0) + " sec");
}

bool MainWindow::rebuildAFS(AFS_File *afs)
{
	bool newAFS = this->afs != afs;

	if (afs == nullptr) {
		if (this->afs != nullptr) {
			afs = this->afs;
		}
		else {
			return false; // prevent possible future errors
		}
	}

	std::string path;
	do {
		path = QFileDialog::getSaveFileName(this, "Save rebuilded AFS file", getFilename(afs->afsName).c_str(), "AFS file (*.afs)").toLocal8Bit().toStdString();
		if (afs->afsName == path) {
			ShowError(this, "Error", "Unable to rebuild AFS over the original!\nSelect another location and try again");
		}
	} while (afs->afsName == path);

	if (path.empty()) {
		return false;
	}

	if (newAFS) {
		startWorker(Type::Rebuild, {{0, path}}, afs);
	}
	else {
		startWorker(Type::Rebuild, {{0, path}});
	}

	return true;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
	event->acceptProposedAction();
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
	auto pos = event->pos();
	pos.ry() -= ui->tableWidget->horizontalHeader()->height();
	pos = ui->tableWidget->mapFrom(this, pos);

	int row = ui->tableWidget->rowAt(pos.y());
	int column = ui->tableWidget->columnAt(pos.x());

	ui->tableWidget->clearSelection();

	if (row != -1 && column != -1) {
		QTableWidgetSelectionRange range(row, 0, row + event->mimeData()->urls().size() - 1, ui->tableWidget->columnCount() - 1);
		ui->tableWidget->setRangeSelected(range, true);
	}

	event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
	auto urls = event->mimeData()->urls();
	auto size = (uint32_t)urls.size();

	auto pos = event->pos();
	pos.ry() -= ui->tableWidget->horizontalHeader()->height();
	pos = ui->tableWidget->mapFrom(this, pos);

	int row = ui->tableWidget->rowAt(pos.y());
	int column = ui->tableWidget->columnAt(pos.x());

	if (afs == nullptr || (size == 1 && (row == -1 || column == -1))) {
		std::string path = urls[0].path().toLocal8Bit().toStdString();

#ifdef _WIN32
		if (path[0] == '/') {
			path = path.substr(1, path.size());
		}
#endif

		openAFS(path);
	}
	else {
		if (row != -1 && column != -1) {
			std::map<uint32_t, std::string> list;
			std::string path;

			for (uint32_t i = 0; i < size; ++i) {
				path = urls[i].path().toLocal8Bit().toStdString();

#ifdef _WIN32
				if (path[0] == '/') {
					path = path.substr(1, path.size());
				}
#endif

				list.insert({getIndexFromRow(row + i), path});
			}

			startWorker(Type::Import, list);
		}
	}

	event->acceptProposedAction();
}

void MainWindow::populateRowCell(int row, int column, QTableWidgetItem *item)
{
	item->setFlags(item->flags() ^ Qt::ItemIsEditable);
	item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	ui->tableWidget->setItem(row, column, item);
}

void MainWindow::adjustColumns()
{
	/*ui->tableWidget->horizontalHeader()->setResizeContentsPrecision(-1);
	ui->tableWidget->resizeColumnsToContents();*/

	ui->tableWidget->setColumnWidth(columnID::number, ui->tableWidget->columnWidth(columnID::number) + 15);
	ui->tableWidget->setColumnWidth(columnID::filename, ui->tableWidget->columnWidth(columnID::filename) + 60);
	ui->tableWidget->setColumnWidth(columnID::size, ui->tableWidget->columnWidth(columnID::size) + 26);
	ui->tableWidget->setColumnWidth(columnID::reservedSpace, ui->tableWidget->columnWidth(columnID::reservedSpace) + 27);
	ui->tableWidget->setColumnWidth(columnID::afterRebuild, ui->tableWidget->columnWidth(columnID::afterRebuild) + 27);
	ui->tableWidget->setColumnWidth(columnID::dateModified, ui->tableWidget->columnWidth(columnID::dateModified) + 40);
	ui->tableWidget->setColumnWidth(columnID::address, ui->tableWidget->columnWidth(columnID::address) + 20);
}

void MainWindow::drawFileList()
{
	if (afs == nullptr) {
		return; // prevent possible future errors
	}

	enableCellChanged = false;

	ui->tableWidget->setSortingEnabled(false);

	ui->tableWidget->clear();

	// get afs info
	auto vfi = afs->getFileInfo();
	auto vfd = afs->getFileDesc();
	auto fileCount = afs->getFileCount();

	// set text information
	ui->afsName->setText(afs->afsName.c_str());
	ui->afsFileCount->setText("File count: " + QString::number(fileCount));
	ui->afsSize->setText(("AFS size: " + getStringSize(afs->getAFSSize())).c_str());

#ifdef DBZBT3_DEBUG
	vfd.emplace_back();
	fileCount++;
#endif

	// generate columns
	auto columns = QString("N.;Filename;Size;Reserved space;Reserved space\n(after rebuild);Date modified;Address").split(";");
	ui->tableWidget->setColumnCount(columns.size());
	ui->tableWidget->setHorizontalHeaderLabels(columns);

	// set default row height and alignment
	ui->tableWidget->insertRow(0);
	ui->tableWidget->verticalHeader()->setDefaultSectionSize(ui->tableWidget->rowHeight(0));
	ui->tableWidget->verticalHeader()->setDefaultAlignment(Qt::AlignHCenter);
	ui->tableWidget->resizeRowToContents(0);
	ui->tableWidget->resizeColumnsToContents();

	// generate rows
	ui->tableWidget->setRowCount(fileCount);

	for (uint32_t i = 0; i < fileCount; ++i) {
		// element
		QTableWidgetItem *item = new TableWidgetItem(QString::number(i + 1), TableWidgetItem::Type::Integer);
		populateRowCell(i, columnID::number, item);

		// filename
		item = new TableWidgetItem(QString::fromLocal8Bit(afs->getFilename(i).c_str()));
		populateRowCell(i, columnID::filename, item);
		item->setFlags(item->flags() ^ Qt::ItemIsEditable);
		item->setTextAlignment(item->textAlignment() ^ Qt::AlignHCenter);

		// size
		//item = new TableWidgetItem(QString::number(vfi[i].size), TableWidgetItem::Type::Integer);
		item = new TableWidgetItem(getStringSize(vfi[i].size).c_str(), TableWidgetItem::Type::Integer);
		populateRowCell(i, columnID::size, item);

		auto hasOverSpace = afs->hasOverSpace(i);

		// reservedSpace
		//item = new TableWidgetItem(QString::number(vfi[i].reservedSpace), TableWidgetItem::Type::Integer);
		item = new TableWidgetItem(getStringSize(vfi[i].reservedSpace).c_str(), TableWidgetItem::Type::Integer);
		populateRowCell(i, columnID::reservedSpace, item);
		if (!hasOverSpace.first) {
			item->setTextColor(Qt::GlobalColor::green);
		}

		// afterRebuild
		//item = new TableWidgetItem(QString::number(vfi[i].reservedSpaceRebuild), TableWidgetItem::Type::Integer);
		item = new TableWidgetItem(getStringSize(vfi[i].reservedSpaceRebuild).c_str(), TableWidgetItem::Type::Integer);
		populateRowCell(i, columnID::afterRebuild, item);
		if (!hasOverSpace.second) {
			item->setTextColor(Qt::GlobalColor::green);
		}

		// date
		item = new TableWidgetItem(QString::number(vfd[i].day).rightJustified(2, '0') + "-" + QString::number(vfd[i].month).rightJustified(2, '0') + "-" + QString::number(vfd[i].year).rightJustified(4, '0') + " " + QString::number(vfd[i].hour).rightJustified(2, '0') + ":" + QString::number(vfd[i].min).rightJustified(2, '0') + ":" + QString::number(vfd[i].sec).rightJustified(2, '0'));
		populateRowCell(i, columnID::dateModified, item);

		// fileAddress
		item = new TableWidgetItem(QString::number(vfi[i].address), TableWidgetItem::Type::Integer);
		populateRowCell(i, columnID::address, item);
	}

	// adjust columns
	adjustColumns();

	ui->tableWidget->setSortingEnabled(true);

	enableCellChanged = true;
}

uint32_t MainWindow::getIndexFromRow(int row) const
{
	return (ui->tableWidget->item(row, columnID::number)->text().toUInt() - 1);
}

std::vector<uint32_t> MainWindow::getSelectedIndexes() const
{
	QModelIndexList selection = ui->tableWidget->selectionModel()->selectedRows();
	uint32_t size = selection.size();

	std::vector<uint32_t> rows;
	rows.reserve(size);

	for (uint32_t i = 0; i < size; ++i) {
		rows.emplace_back(getIndexFromRow(selection[i].row()));
	}

	return rows;
}

void MainWindow::startWorker(Type type, const std::map<uint32_t, std::string> &list, AFS_File *afs)
{
	if (afs == nullptr) {
		if (this->afs != nullptr) {
			afs = this->afs;
		}
		else {
			return; // prevent possible future errors
		}
	}

	if (list.empty() || (type == Type::Rebuild && list.size() != 1)) {
		return;
	}

	auto *worker = new Worker(type, afs, list, this);
	auto *progressDialog = new ProgressDialog(type, list.size(), 0, this);

	connect(worker, &Worker::started, progressDialog, &ProgressDialog::show); // show dialog when thread start
	connect(worker, &Worker::done, progressDialog, &ProgressDialog::accept); // close dialog when thread finish

	connect(this, &MainWindow::skipFile, worker, &Worker::skipFile); // send skip signal to worker
	connect(worker, &Worker::errorFile, this, &MainWindow::errorFile); // show warning on error

	if (type != Type::Rebuild) {
		connect(worker, &Worker::next, progressDialog, &ProgressDialog::next); // updated bar on progress

		if (type == Type::Import) {
			connect(worker, &Worker::toAdjust, this, &MainWindow::toAdjust_p1); // ask to rebuild
			connect(worker, &Worker::refreshRow, this, &MainWindow::refreshRow); // refresh row of imported file
			connect(worker, &Worker::refreshRow, this, &MainWindow::updateFreeSpaceLabel); // update free space label on imported file
		}
	}
	else {
		if (oldWorker != nullptr) {
			connect(worker, &Worker::rebuilded, this, &MainWindow::toAdjust_p2); // complete import
		}
	}

	connect(worker, &Worker::progressText, progressDialog, &ProgressDialog::setLabel); // update label on progress

	connect(progressDialog, &ProgressDialog::rejected, worker, &Worker::terminate); // kill thread on dialog rejected
	connect(worker, &Worker::abort, this, &MainWindow::abort); // are you sure that you want to abort?

	connect(this, &MainWindow::done, worker, &Worker::deleteLater); // clean worker on abort accepted

	connect(worker, &Worker::done, worker, &Worker::deleteLater); // clean worker on finish
	connect(worker, &Worker::destroyed, progressDialog, &ProgressDialog::deleteLater); // clean dialog after thread cleaned

	worker->start();
}

// ---------- menu bar ----------
void MainWindow::on_actionOpen_triggered()
{
	openAFS(QFileDialog::getOpenFileName(this).toLocal8Bit().toStdString());
}

void MainWindow::on_actionExit_triggered()
{
	this->close();
}

void MainWindow::on_actionSettings_triggered()
{
	ShowError(this, "Error", "Not yet implemented...");
}

void MainWindow::on_actionExportToFolder_triggered()
{
	if (afs == nullptr) {
		return; // prevent possible future errors
	}

	std::string path = QFileDialog::getExistingDirectory(this).toLocal8Bit().toStdString();

	if (path.empty()) {
		return;
	}

	auto size = afs->getFileCount();
	std::map<uint32_t, std::string> list;

	for (uint32_t i = 0; i < size; ++i) {
		list.insert({i, path + '/' + afs->getFilename(i)});
	}

	startWorker(Type::Export, list);
}

void MainWindow::on_actionImportFromFolder_triggered()
{
	QString path = QFileDialog::getExistingDirectory(this);

	if (path.isEmpty()) {
		return;
	}

	QStringList files = QDir(path).entryList(QDir::Files);
	uint32_t size = files.size();

	auto fileCount = afs->getFileCount();
	std::vector<std::string> afsList;
	afsList.reserve(fileCount);

	for (uint32_t i = 0; i < fileCount; ++i) {
		afsList.emplace_back(afs->getFilename(i));
	}

	std::map<uint32_t, std::string> list;

	for (uint32_t i = 0; i < size; ++i) {
		auto iter = std::find(afsList.begin(), afsList.end(), files[i].toLocal8Bit().toStdString());
		if (iter != afsList.end()) {
			uint32_t index = iter - afsList.begin();
			list.insert({index, path.toLocal8Bit().toStdString() + '/' + afsList[index]});
		}
	}

	startWorker(Type::Import, list);
}

void MainWindow::on_actionExportAFLCommon_triggered()
{
	if (afs == nullptr) {
		return; // prevent possible future errors
	}

	std::string path = QFileDialog::getSaveFileName(this, "Save AFL file", getFilename(afs->afsName).c_str(), "AFL file (*.afl)").toLocal8Bit().toStdString();
	if (path.empty()) {
		return;
	}

	if (afs->exportAFLCommon(path)) {
		ShowInfo(this, "Success", "AFL successfully exported");
	}
	else {
		ShowError(this, "Error", "Unable to export AFL!");
	}
}

void MainWindow::on_actionImportAFLCommon_triggered()
{
	if (afs == nullptr) {
		return; // prevent possible future errors
	}

	std::string path = QFileDialog::getOpenFileName(this).toLocal8Bit().toStdString();

	if (path.empty()) {
		return;
	}

	auto result = afs->importAFLCommon(path);
	if (result == 1) {
		ShowInfo(this, "Success", "AFL successfully imported");
		drawFileList();
	}
	else {
		QString error = "Unable to import AFL";
		ShowError(this, "Error", error + (result == 2 ? " (incompatible)" : ""));
	}
}

void MainWindow::on_actionOptimize_triggered()
{
	ShowInfo(this, "Optimizer", "This option will optimize all reserved space to fit files size");

	afs->optimize();

	drawFileList();
	updateFreeSpaceLabel();

	ShowInfo(this, "Optimizer", "Operation completed!\nNow you should rebuild AFS for optimization to take effect");
}

void MainWindow::on_actionRebuild_triggered()
{
	rebuildAFS();
}

void MainWindow::on_actionAbout_triggered()
{
	AboutDialog(windowTitle(), this).exec();
}
// ---------- end menu bar ----------

// ---------- various slots ----------
void MainWindow::toAdjust_p1()
{
	auto worker = (Worker *)QObject::sender();

	worker->wait();

	auto index = worker->getPosition();
	auto errors = worker->getErrors();

	auto buttons = QMessageBox::Yes | QMessageBox::No;

	if (errors > 2) {
		buttons |= QMessageBox::NoToAll;
	}

	auto reply = ShowError(this, "Error", "Not enought space to import over '" + QString::fromLocal8Bit(afs->getFilename(index).c_str()) + "'...\nDo you to want to auto-adjust reserved space?", buttons);

	if (reply == QMessageBox::Yes) {
		auto *afs = new AFS_File(*this->afs);

		for (auto item : worker->getList()) {
			auto size = getFileSize(item.second);
			auto rs = afs->getReservedSpace(item.first);

			if (size > rs.first) {
				afs->changeReservedSpace(item.first, afs->getOptimizedReservedSpace(size, AFS_File::Type::Size));
			}
		}

		oldWorker = worker; // the magic

		if (!rebuildAFS(afs)) {
			oldWorker = nullptr;
			emit done();
		}
	}
	else {
		if (reply == QMessageBox::NoToAll) {
			worker->setSkipAll(true);
		}
		worker->skipFile();
	}
}

void MainWindow::toAdjust_p2(const std::string &path)
{
	if (oldWorker != nullptr) {
		auto *afs = this->afs;
		openAFS(path);
		if (afs != this->afs) {
			oldWorker->updateAFS(this->afs);
			oldWorker->start();
		}
		else {
			emit done();
		}
		oldWorker = nullptr;
	}
}

void MainWindow::updateFreeSpaceLabel()
{
	auto fileCount = afs->getFileCount();

	uint64_t freeSpace = afs->getFileInfo(0).address - afs->getOptimizedReservedSpace(16 + 8 * fileCount, AFS_File::Type::Size);
	uint64_t freeSpaceRebuild = 0;


	for (uint32_t i = 0; i <= fileCount; ++i) {
		auto rs = afs->getReservedSpace(i);
		auto ors = afs->getOptimizedReservedSpace(i);

		if (rs.first > ors) {
			freeSpace += (rs.first - ors);
		}
		if (rs.second > ors) {
			freeSpaceRebuild += (rs.second - ors);
		}
	}

	ui->freeSpace->setText(("Free space: " + getStringSize(freeSpace) + " (" + getStringSize(freeSpaceRebuild) + ")").c_str());
}

void MainWindow::abort()
{
	auto worker = (Worker *)QObject::sender();

	worker->wait();

	QString type = worker->type == Type::Import ? " import " : (worker->type == Type::Export ? " export" : (worker->type == Type::Rebuild ? " rebuild" : ""));

	if (ShowWarning(this, "Abort", "Are you sure that you want to abort" + type + "?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
		if (oldWorker != nullptr) {
			oldWorker = nullptr;
		}

		emit done();
	}
	else {
		worker->start();
	}
}

void MainWindow::errorFile()
{
	auto worker = (Worker *)QObject::sender();

	worker->wait();

	auto index = worker->getPosition();
	auto errors = worker->getErrors();

	auto buttons = QMessageBox::Yes | QMessageBox::No;

	if (errors > 2) {
		buttons |= QMessageBox::NoToAll;
	}

	QMessageBox::StandardButton reply = QMessageBox::No;

	if (worker->type == Type::Export) {
		reply = ShowError(this, "Error", "Error while extracting '" + QString::fromLocal8Bit(afs->getFilename(index).c_str()) + "'...\nDo you to want to retry?", buttons);
	}
	else if (worker->type == Type::Import) {
		reply = ShowError(this, "Error", "Error while importing over '" + QString::fromLocal8Bit(afs->getFilename(index).c_str()) + "'...\nDo you to want to retry?", buttons);
	}
	else if (worker->type == Type::Rebuild) {
		reply = ShowError(this, "Error", "Error while rebuilding AFS file...\nDo you to want to retry?", buttons);
	}

	if (reply == QMessageBox::Yes) {
		worker->start();
	}
	else {
		if (reply == QMessageBox::NoToAll) {
			worker->setSkipAll(true);
		}
		emit skipFile();
	}
}

void MainWindow::refreshRow(uint32_t index)
{
	auto list = ui->tableWidget->findItems(QString::number(index + 1), Qt::MatchExactly);

	int row = -1;

	for (auto item : list) {
		if (item->column() == columnID::number) {
			row = item->row();
			break;
		}
	}

	if (row != -1) {
		auto fileInfo = afs->getFileInfo(index);
		auto fileDesc = (index != afs->getFileCount() ? afs->getFileDesc(index) : AFS_File::FileDesc());

		// size
		auto item = ui->tableWidget->item(row, columnID::size);
		//item->setText(QString::number(fileInfo.size));
		item->setText(getStringSize(fileInfo.size).c_str());

		auto hasOverSpace = afs->hasOverSpace(index);

		// reservedSpace
		item = ui->tableWidget->item(row, columnID::reservedSpace);
		//item->setText(QString::number(fileInfo.reservedSpace));
		item->setText(getStringSize(fileInfo.reservedSpace).c_str());
		if (!hasOverSpace.first) {
			item->setTextColor(Qt::GlobalColor::green);
		}
		else {
			if (fileInfo.size > fileInfo.reservedSpace) {
				item->setTextColor(Qt::GlobalColor::red);
			}
			else {
				item->setTextColor(Qt::GlobalColor::black);
			}
		}

		// afterRebuild
		item = ui->tableWidget->item(row, columnID::afterRebuild);
		//item->setText(QString::number(fileInfo.reservedSpaceRebuild));
		item->setText(getStringSize(fileInfo.reservedSpaceRebuild).c_str());
		if (!hasOverSpace.second) {
			item->setTextColor(Qt::GlobalColor::green);
		}
		else {
			if (fileInfo.size > fileInfo.reservedSpaceRebuild) {
				item->setTextColor(Qt::GlobalColor::red);
			}
			else {
				item->setTextColor(Qt::GlobalColor::black);
			}
		}

		// date
		item = ui->tableWidget->item(row, columnID::dateModified);
		item->setText(QString::number(fileDesc.day).rightJustified(2, '0') + "-" + QString::number(fileDesc.month).rightJustified(2, '0') + "-" + QString::number(fileDesc.year).rightJustified(4, '0') + " " + QString::number(fileDesc.hour).rightJustified(2, '0') + ":" + QString::number(fileDesc.min).rightJustified(2, '0') + ":" + QString::number(fileDesc.sec).rightJustified(2, '0'));
	}
}

void MainWindow::on_actionExportSelection_triggered()
{
	if (afs == nullptr) {
		return; // prevent possible future errors
	}

	auto listIndex = getSelectedIndexes();
	uint32_t size = listIndex.size();

	std::map<uint32_t, std::string> list;

	std::string path;
	if (size == 1) {
		path = QFileDialog::getSaveFileName(this, "Save file", QString::fromLocal8Bit(afs->getFilename(listIndex[0]).c_str()), "File (*)").toLocal8Bit().toStdString();

		if (path.empty()) {
			return;
		}

		list.insert({listIndex[0], path});
	}
	else if (size > 1) {
		path = QFileDialog::getExistingDirectory(this).toLocal8Bit().toStdString();

		if (path.empty()) {
			return;
		}

		for (uint32_t i = 0; i < size; ++i) {
			list.insert({listIndex[i], path + '/' + afs->getFilename(listIndex[i])});
		}
	}

	startWorker(Type::Export, list);
}

void MainWindow::on_actionImportFile_triggered()
{
	if (afs == nullptr) {
		return; // prevent possible future errors
	}

	auto listIndex = getSelectedIndexes();

	if (listIndex.size() == 1) {
		std::string path = QFileDialog::getOpenFileName(this).toLocal8Bit().toStdString();

		if (path.empty()) {
			return;
		}

		startWorker(Type::Import, {{listIndex[0], path}});
	}
}

void MainWindow::on_actionModifyReservedSpace_triggered()
{
	if (afs == nullptr) {
		return; // prevent possible future errors
	}

	auto listIndex = getSelectedIndexes();

	if (listIndex.size() == 1) {
		auto reservedSpace = afs->getReservedSpace(listIndex[0]);

		ReservedSpaceDialog rsd(reservedSpace.first, reservedSpace.second, this);

		auto result = rsd.exec();

		if (result) {
			if (afs->changeReservedSpace(listIndex[0], rsd.getNewReservedSpace())) {
				refreshRow(listIndex[0]);
				updateFreeSpaceLabel();
			}
			else {
				ShowError(this, "Error", "Unable to set '" + QString::number(rsd.getNewReservedSpace()) + "' has reserved space");
			}
		}
	}

}

void MainWindow::on_tableWidget_cellChanged(int row, int column)
{
	if (enableCellChanged) {
		auto item = ui->tableWidget->item(row, column);

		if (column == columnID::filename) {
			auto text = item->text();
			if (text.size() > FILENAME_SIZE) {
				text.resize(FILENAME_SIZE);
				enableCellChanged = false;
				item->setText(text);
				enableCellChanged = true;
			}
			afs->changeFilename(row, text.toLocal8Bit());
			if (!afs->commitFileDesc()) {
				ShowError(this, "Error", "Unable to save AFS");
			}
		}
	}
}

void MainWindow::on_tableWidget_customContextMenuRequested(QPoint pos)
{
	QMenu contextMenu;
	auto size = ui->tableWidget->selectionModel()->selectedRows().size();
	if (size >= 1) {
		contextMenu.addAction(ui->actionExportSelection);
		if (size == 1) {
			ui->actionExportSelection->setText("Export file");
			contextMenu.addAction(ui->actionImportFile);
			contextMenu.addAction(ui->actionModifyReservedSpace);
		}
		else {
			ui->actionExportSelection->setText("Export selection");
		}
	}
	ui->actionExportSelection->setToolTip(ui->actionExportSelection->text());
	contextMenu.exec(ui->tableWidget->mapToGlobal(pos));
}
// ---------- end various slots ----------
