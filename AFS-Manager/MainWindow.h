#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileDialog>
#include <QMainWindow>
#include <QTableWidget>

#include "Unpacker.h"
#include "../Libraries/Dialog/Message.h"
#include "../Libraries/Dialog/Progress.h"
#include "../Libraries/Dialog/Warning.h"

namespace Ui
{
	class MainWindow;
}

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
	MainWindow(const std::string &name, const std::string &version, QWidget *parent = 0);

	~MainWindow();

private:
	void connectCellChanged(); // create connection for cellChanged

	void adjustColumns();

	void drawFileList();

	QList<uint32_t> getSelectedRows() const;

	void populateRowCell(const int &row, const int &column, QTableWidgetItem *item);

	void startExporting(const QList<uint32_t> &list, const std::string &path);

	virtual void dragEnterEvent(QDragEnterEvent *event);

	virtual void dropEvent(QDropEvent *event);

private:
	QAction *actionExportSelected;
	QAction *actionImportFile;
	QAction *actionModifyReservedSpace;
	AFS_File *afs;
	QMetaObject::Connection connectionCellChanged;
	Progress *progressUnpacker;
	Unpacker *unpacker;
	Ui::MainWindow *ui;

public slots:

	void openAFS(const std::string &name);

private slots:
	// menu bar
	void on_actionExit_triggered();

	void on_actionExportCommon_triggered();

	void on_actionExportRAW_triggered(); // TODO -> remove, only for testing

	void on_actionImportCommon_triggered();

	void on_actionImportRAW_triggered(); // TODO -> remove, only for testing

	void on_actionOpen_triggered();

	void on_actionSave_triggered();

	void on_actionUnpackAFS_triggered();

	// unpacker connection
	void progressFile(const char* name);

	void errorFile(const char* name);

	void exportDone();

	// context menu
	void showContextMenu(const QPoint &point);

	void on_actionExportSelected_triggered();

	void on_actionImportFile_triggered();

	void on_actionModifyReservedSpace_triggered();

	void slotCellChanged(const int &row, const int &column);

signals:
	void exportFile();
};

#endif // MAINWINDOW_H
