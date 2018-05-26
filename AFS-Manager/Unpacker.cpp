#include "Unpacker.h"

Unpacker::Unpacker(const AFS_File *afs, const QList<uint32_t> &list, const std::string &path) : afs(afs), list(list), path(path), position(0)
{
	this->moveToThread(&thread);

	connect(&thread, SIGNAL(started()), this, SLOT(exportFile()));
	connect(&thread, SIGNAL(finished()), this, SLOT(emitDone()));
}

Unpacker::~Unpacker()
{
}

void Unpacker::start()
{
	thread.start();
}

void Unpacker::exportFile()
{
	if (position < (uint32_t)this->list.size()) {
		++position;
		emit progressFile();
	}
	else {
		thread.quit();
		thread.wait();
	}
}

void Unpacker::emitDone()
{
	emit exportDone();
}

/*
void Unpacker::run()
{
	int size = list.size();
	int multi = size > 1 ? true : false;
	std::string filepath = path;
	for (int i = 0; i < size; ++i) {
		if (!progressUnpacker->isVisible()) {
			if (!isInterrupted) {
				isInterrupted = true;
				emit abort();
			}
		}

		if (toSkip) {
			toSkip = false;
		}
		else {
			if (!isInterrupted) {
				if (size > 1) {
					filepath = path + '/' + afs->getFileDesc()[list[i]].name;
				}
				if (afs->exportFile(list[i], filepath)) {
					emit next();
				}
				else {
					if (!toSkipAll) {
						isInterrupted = true;
						emit error(QString::fromLocal8Bit(afs->getFileDesc()[list[i]].name), multi);
						i--; // wait
					}
					else {
						emit next(); // to fill progressBar
					}
				}
			}
			else {
				i--; // wait
			}
		}
	}
	emit done();
}
*/
