#include "Unpacker.h"

Unpacker::Unpacker(AFS_File* afs, const QList<int>& list, const std::string& path) : QThread(), afs(afs), list(list), path(path), isInterrupted(false), toSkip(false), toSkipAll(false)
{
	progress = new Progress("Unpacker", "Exporting files...", QString::fromLocal8Bit(":/Unpack"));
	progress->setMaximum(list.size()).show();
	connect(this, SIGNAL(next()), progress, SLOT(next()));
}

Unpacker::~Unpacker()
{
	delPointer(progress);
}

void Unpacker::resume()
{
	if (isInterrupted)
	{
		if (!progress->isVisible())
			progress->setVisible(true);
		isInterrupted = false;
	}
}

void Unpacker::skip()
{
	if (isInterrupted)
	{
		isInterrupted = false;
		toSkip = true;
	}
}

void Unpacker::skipAll()
{
	if (isInterrupted)
	{
		isInterrupted = false;
		toSkipAll = true;
	}
}

void Unpacker::run()
{
	int size = list.size();
	int multi = size > 1 ? true : false;
	std::string filepath = path;
	for (int i = 0; i < size; ++i)
	{
		if (!progress->isVisible())
		{
			if (!isInterrupted)
			{
				isInterrupted = true;
				emit abort();
			}
		}

		if (toSkip)
			toSkip = false;
		else
		{
			if (!isInterrupted)
			{
				if (size > 1)
					filepath = path + '/' + afs->getFileDesc()[list[i]].name;
				if (afs->exportFile(list[i], filepath))
					emit next();
				else
				{
					if (!toSkipAll)
					{
						isInterrupted = true;
						emit error(QString::fromLocal8Bit(afs->getFileDesc()[list[i]].name), multi);
						i--; // wait
					}
					else
						emit next(); // to fill progressBar
				}
			}
			else
				i--; // wait
		}
	}
	emit done();
}
