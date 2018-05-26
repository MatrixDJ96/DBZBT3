#include <QApplication>
#include <QFileDialog>

#include "VersionInfo.h"
#include "../Libraries/Dialog/Dialog.h"
#include "../Libraries/CTECore/CTECore.h"

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

	if (!Shared::fileExists(inName) || inName.empty()) {
		inName = QFileDialog::getOpenFileName().toLocal8Bit().toStdString();
	}

	if (!inName.empty()) {
		Texture_File texture(inName);

		if (texture.getTexturesCount() > 0) {
			bool overwrite = false;
			std::string outName(texture.getOutName());
			if (Shared::fileExists(outName)) {
				Warning warning(title, "Do you want to overwrite\n'" + Shared::getFilename(outName) + "'?");
				warning.exec();
				Reply reply = warning.getReply();
				if (reply == Reply::Left) {
					overwrite = true;
				}
				else if (reply == Reply::Right) {
					outName = QFileDialog::getSaveFileName().toLocal8Bit().toStdString();
					if (!outName.empty()) {
						if (Shared::fileExists(outName)) {
							overwrite = true;
						}
						texture.setOutName(outName);
					}
					else {
						return EXIT_FAILURE;
					}
				}
				else {
					return EXIT_FAILURE;
				}
			}

			bool result = texture.Convert();
			if (result) {
				Message message(title, "'" + Shared::getFilename(outName) + "'\nsuccessfully " + (overwrite ? "overwritten" : "created"));
				message.exec();
				return EXIT_SUCCESS;
			}
			else {
				Message message(title, "Unable to convert\n'" + Shared::getFilename(inName) + "'", Type::Error);
				message.exec();
				return EXIT_FAILURE;
			}
		}
		else {
			Message message(title, "Nothing to do!");
			message.exec();
			return EXIT_SUCCESS;
		}
	}

	return EXIT_FAILURE;
}
