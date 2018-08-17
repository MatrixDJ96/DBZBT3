#include <QApplication>

#include <MainWindow.h>
#include <VersionInfo.h>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	MainWindow window(PRODUCTNAME_STR, FILEVERSION_STR);
	if (argc > 1) {
		window.openAFS(argv[1]);
	}
	else {
		window.openAFS("");
	}
	window.show();

	return app.exec();
}
