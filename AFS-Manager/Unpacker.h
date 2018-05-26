#ifndef UNPACKER_H
#define UNPACKER_H

#include <QThread>

#include "../Libraries/AFSCore/AFSCore.h"
#include "../Libraries/Dialog/Dialog.h"

class Unpacker : public QObject
{
Q_OBJECT

public:
	Unpacker(const AFS_File *afs, const QList<uint32_t> &list, const std::string &path);

	~Unpacker();

	void start();

private:
	const AFS_File *afs;
	const QList<uint32_t> list;
	const std::string path;
	QThread thread;
	uint32_t position;
	
public slots:
	void exportFile();

	void emitDone();

signals:
	void progressFile();

	void exportDone();
};

#endif // UNPACKER_H
