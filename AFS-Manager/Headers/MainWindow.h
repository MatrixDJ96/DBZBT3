#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDragEnterEvent>

#include <AFSCore.h>
#include <TableWidgetItem.h>
#include <Worker.h>

namespace Ui
{
	class MainWindow;
}

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
	explicit MainWindow(const std::string &name, const std::string &version, QWidget *parent = nullptr);

	~MainWindow() override;

private:
	void dragEnterEvent(QDragEnterEvent *event) override;

	void dragMoveEvent(QDragMoveEvent *event) override;

	void dropEvent(QDropEvent *event) override;

	void populateRowCell(int row, int column, QTableWidgetItem *item);

	void adjustColumns();

	void drawFileList();

	uint32_t getIndexFromRow(int row) const;

	std::vector<uint32_t> getSelectedIndexes() const;

	void startWorker(Shared::Type type, const std::map<uint32_t, std::string> &list, AFS_File *afs = nullptr);

private:
	Ui::MainWindow *ui;

	AFS_File *afs;

	bool enableCellChanged;

	Worker *oldWorker;

public slots:
	void openAFS(const std::string &path, bool firstCall = true);

	bool rebuildAFS(AFS_File *afs = nullptr);

private slots:
	void toAdjust_p1(bool init);

	void toAdjust_p2(std::string path);

	void updateFreeSpaceLabel();

	void abort();

	void errorFile();

	void errorMessage(const std::string& message);

	void refreshRow(uint32_t index);

	void on_actionOpen_triggered();

	void on_actionExit_triggered();

	void on_actionSettings_triggered();

	void on_actionExportToFolder_triggered();

	void on_actionImportFromFolder_triggered();

	void on_actionExportAFLCommon_triggered();

	void on_actionImportAFLCommon_triggered();

	void on_actionOptimize_triggered();

	void on_actionRebuild_triggered();

	void on_actionAbout_triggered();

	void on_actionExportSelection_triggered();

	void on_actionImportFile_triggered();

	void on_actionModifyReservedSpace_triggered();

	void on_tableWidget_cellChanged(int, int);

	void on_tableWidget_customContextMenuRequested(QPoint);

signals:
	void done();

	void skipFile();
};

#endif // MAINWINDOW_H
