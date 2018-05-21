#include <QApplication>
#include <QFileDialog>

#include "../Headers/CTECore.h"
#include "../Headers/VersionInfo.h"
#include "../../Shared/Headers/Dialog.h"

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	Dialog dialog(app, PRODUCTNAME_STR, FILEVERSION_STR);

	std::string name;
	if (argc < 2)
		name = std::string(QFileDialog::getOpenFileName().toStdString());
	else
		name = argv[1];

	if (name != "")
	{
		std::ifstream in(name, std::ios::in | std::ios::binary);
		if (in.is_open())
		{
			Texture_File file(name, in);
			if (file.getTexturesCount() > 0)
			{
				uint8_t result = file.Convert();
				if (result == 0)
					dialog.setLabelText("File '" + file.getOutName() + ".ini' written!");
				else if (result == 1)
					dialog.setLabelText("File '" + file.getOutName() + "' already exists");
				else if (result == 2)
					dialog.setLabelText("Unable to write '" + file.getOutName() + "'\n(No permission?)");
			}
			else
				dialog.setLabelText("Nothing to do! (Empty file?)");
		}
		else
			dialog.setLabelText("File '" + name + "' not found!");
	}
	else
		app.exit();

	return app.exec();
}
