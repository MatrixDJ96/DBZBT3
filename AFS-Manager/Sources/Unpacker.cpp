#include "Unpacker.h"

Unpacker::Unpacker(const AFS_File *afs, const QList<uint32_t> &list, const std::string &path) : afs(afs), list(list), path(path), position(0)
{
	this->moveToThread(&thread);

	connect(&thread, SIGNAL(started()), this, SLOT(exportFile()));
	//connect(&thread, SIGNAL(finished()), this, SLOT(done()));
	//connect(&thread, &QThread::finished, this, &QObject::deleteLater);
}

Unpacker::~Unpacker()
{
	if (thread.isRunning()) {
		thread.quit();
		thread.wait();
	}
};

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
		std::string path = this->path;
		if (this->list.size() > 1) {
			path += std::string("/") + afs->getFilename(position);
		}

		if (afs->exportFile(list[position], path)) {
			emit progressFile(afs->getFilename(position++));
		}
		else {
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

void Unpacker::quit()
{
	if (thread.isRunning()) {
		thread.quit();
		thread.wait();
	}
}
