#include <QApplication>

#include <MainWindow.h>
#include <VersionInfo.h>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	qRegisterMetaType<uint8_t>("uint8_t");
	qRegisterMetaType<uint32_t>("uint32_t");

	MainWindow window(PRODUCTNAME_STR, FILEVERSION_STR);
	if (argc > 1) {
		window.openAFS(argv[1]);
	}

	window.show();

	return app.exec();
}