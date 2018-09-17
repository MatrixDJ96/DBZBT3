#include <QApplication>
#include <QFileDialog>

#include <AFLCore.h>
#include <MessageBox.h>
#include <VersionInfo.h>

using namespace Shared;
using Reply = QMessageBox::StandardButton;

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

	if (!fileExists(inName) || inName.empty()) {
		inName = QFileDialog::getOpenFileName().toLocal8Bit().toStdString();
	}

	if (!inName.empty()) {
		AFL_File afl(inName);

		if (afl.getFileCount() > 0) {
			bool overwrite = false;
			std::string outName = afl.getOutName();
			if (fileExists(outName)) {
				Reply reply =  ShowWarning(nullptr, title.c_str(), ("Do you want to overwrite '" + getFileBasename(outName) + "'?").c_str(), QMessageBox::Yes | QMessageBox::No);
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

			auto result = afl.Convert();
			if (result) {
				ShowInfo(nullptr, title.c_str(), ("'" + getFileBasename(outName) + "' successfully " + (overwrite ? "overwritten" : "created")).c_str());
				return EXIT_SUCCESS;
			}
			else {
				ShowError(nullptr, title.c_str(), ("Unable to convert '" + getFileBasename(inName) + "'").c_str());
				return EXIT_FAILURE;
			}
		}
		else {
			ShowInfo(nullptr, title.c_str(), "Nothing to do!");
			return EXIT_SUCCESS;
		}
	}

	return EXIT_FAILURE;
}
