#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>

#include <AFLCore.h>
#include <VersionInfo.h>

using namespace Shared;

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	std::string title(PRODUCTNAME_STR);
	title += " v";
	title += PRODUCTVERSION_STR;

	std::string inName;
	if (argc > 1) {
		inName = argv[1];
	}

	QMessageBox mb;


	if (!fileExists(inName) || inName.empty()) {
		inName = QFileDialog::getOpenFileName().toLocal8Bit().toStdString();
	}

	if (!inName.empty()) {
		AFL_File afl(inName);

		if (afl.getFileCount() > 0) {
			bool overwrite = false;
			std::string outName(afl.getOutName());
			if (fileExists(outName)) {
				QMessageBox::StandardButton reply;
				reply = QMessageBox::warning(nullptr, title.c_str(), ("Do you want to overwrite '" + getFilename(outName) + "'?").c_str(), QMessageBox::Yes | QMessageBox::No);
				if (reply == QMessageBox::Yes) {
					overwrite = true;
				}
				else {
					outName = QFileDialog::getSaveFileName().toLocal8Bit().toStdString();
					if (!outName.empty()) {
						if (fileExists(outName)) {
							overwrite = true;
						}
						afl.setOutName(outName);
					}
					else {
						return EXIT_FAILURE;
					}
				}
			}

			bool result = afl.Convert();
			if (result) {
				QMessageBox::information(nullptr, title.c_str(), ("'" + getFilename(outName) + "' successfully " + (overwrite ? "overwritten" : "created")).c_str(), QMessageBox::Ok);
				return EXIT_SUCCESS;
			}
			else {
				QMessageBox::critical(nullptr, title.c_str(), ("Unable to convert '" + getFilename(inName) + "'").c_str(), QMessageBox::Ok);
				return EXIT_FAILURE;
			}
		}
		else {
			QMessageBox::information(nullptr, title.c_str(), "Nothing to do!", QMessageBox::Ok);
			return EXIT_SUCCESS;
		}
	}

	return EXIT_FAILURE;
}
