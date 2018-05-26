#include <QApplication>

#include "VersionInfo.h"
#include "MainWindow.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	MainWindow window(PRODUCTNAME_STR, FILEVERSION_STR);
	if (argc > 1) {
		window.openAFS(argv[1]);
	}
	else {
		window.openAFS("D:/Giochi/DragonBall Z - Budokai Tenkaichi 3/PZS3EU1_EXPLORER.AFS");
		//window.openAFS("");
	}
	window.show();

	return app.exec();
}
