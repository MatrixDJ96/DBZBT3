#ifndef UNPACKER_H
#define UNPACKER_H

#include <QThread>

#include "../Libraries/AFSCore/AFSCore.h"

class Unpacker : public QObject
{
Q_OBJECT

public:
	Unpacker(const AFS_File *afs, const QList<uint32_t> &list, const std::string &path);

	~Unpacker();

	void start();

	void quit();

	void skip();

private:
	const AFS_File *afs;
	const QList<uint32_t> list;
	const std::string path;
	QThread thread;
	uint32_t position;
	
public slots:
	void exportFile();

	void done();

signals:
	void progressFile(const char* name);

	void errorFile(const char* name);

	void exportDone();
};

#endif // UNPACKER_H
