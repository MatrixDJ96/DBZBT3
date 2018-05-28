#include "Unpacker.h"

Unpacker::Unpacker(const AFS_File *afs, const QList<uint32_t> &list, const std::string &path) : afs(afs), list(list), path(path), position(0)
{
	this->moveToThread(&thread);

	connect(&thread, SIGNAL(started()), this, SLOT(exportFile()));
	connect(&thread, SIGNAL(finished()), this, SLOT(done()));
}

Unpacker::~Unpacker() = default;

void Unpacker::start()
{
	thread.start();
}

void Unpacker::skip()
{
	++position;
}

void Unpacker::exportFile()
{
	if (position < (uint32_t)this->list.size()) {
		std::string filepath = path + '/' + afs->getFilename(position);
		if (afs->exportFile(list[position], filepath)) {
			emit progressFile(afs->getFilename(position++));
		} else {
			emit errorFile(afs->getFilename(position));
		}
	}
	else {
		thread.quit();
		thread.wait();
	}
}

void Unpacker::done()
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
