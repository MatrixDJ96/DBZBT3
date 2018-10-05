#include <Worker.h>

#include <QDebug>

#include <MessageBox.h>

using namespace Shared;

Worker::Worker(Type type, AFS_File *afs, const std::map<uint32_t, std::string> &list, QObject *parent) : afs(afs),/* buffer(nullptr),*/ content(nullptr), errors(0), list(list), iter(this->list.begin()), status(0), skipAll(false), type(type), QThread(parent)
{
	/*constexpr int size = 64 * 1024 * 1024;
	buffer = new char[size];

	for (int i = 0; i < size; ++i) {
		buffer[i] = nullbyte;
	}*/

	if (type != Type::Export && type != Type::Import && type != Type::Rebuild) {
		throw std::out_of_range("Wrong Worker type!");
	}

	setTerminationEnabled(true);

	checkReservedSpace();

	qDebug() << "Created" << this << "->" << (type == Type::Export ? "export" : (type == Type::Import ? "import" : (type == Type::Rebuild ? "rebuild" : "loading")));
}

Worker::~Worker()
{
	//delPointerArray(buffer);

	if (content != nullptr) {
		free(content);
		qDebug() << "content is now free";
	}

	qDebug() << "Destroyed" << this << "->" << (type == Type::Export ? "export" : (type == Type::Import ? "import" : (type == Type::Rebuild ? "rebuild" : "loading")));
}

int Worker::getErrors() const
{
	return errors;
}

uint32_t Worker::getPosition() const
{
	return iter->first;
}

uint8_t Worker::getStatusRS() const
{
	return status;
}

uint8_t Worker::checkReservedSpace()
{
	// RETURN value:
	// 0 -> ok;
	// 1 -> no space;
	// 2 -> too much space
	// 3 -> no space && too much space.

	status = 0;

	if (type == Type::Import) {
		for (const auto &item : list) {
			auto size = getFileSize(item.second);

			auto rs = afs->getReservedSpace(item.first);
			auto ors = afs->getOptimizedReservedSpace(size, AFS_File::Type::Size);

			if (size > rs.first) {
				status |= 1;
			}
			else if (rs.first > ors) {
				status |= 2;
			}
		}
	}

	qDebug() << "status:" << status;

	return status;
}

std::map<uint32_t, std::string> Worker::getList() const
{
	return list;
}

void Worker::setSkipAll(bool flag)
{
	skipAll = flag;
}

void Worker::updateAFS(AFS_File *afs)
{
	this->afs = afs;
	checkReservedSpace();
	removeStatusRS(2);
}

bool Worker::work(uint32_t index, const std::string &path)
{
	uint8_t result = 0;

	auto filename = afs->getFilename(index);

	qDebug() << "content is" << (content != nullptr ? "working" : "free");

	qDebug() << "index:" << index << "| path:" << path.c_str();

	try {
		if (!isInterruptionRequested()) {
			if (type == Type::Export) {
				emit progressText(QString::fromLocal8Bit(("Exporting '" + filename + "'...").c_str()));
				result = (uint8_t)afs->exportFile(index, path, content);
			}
			else if (type == Type::Import) {
				emit progressText(QString::fromLocal8Bit(("Importing '" + getFileBasename(path) + "' over '" + filename + "'...").c_str()));
				result = afs->importFile(index, path, content);
			}
			else if (type == Type::Rebuild) {
				emit progressText(QString::fromLocal8Bit(("Rebuilding '" + getFileBasename(afs->afsName) + "' to '" + getFileBasename(path) + "'...").c_str()));
				result = (uint8_t)afs->rebuild(path, content);
			}

			if (result == 1 || skipAll) {
				emit next();
				if (type == Type::Import) {
					emit refreshRow(index);
				}
			}
			else {
				if (type == Type::Import && result == 2) {
					emit toAdjust(false);
					result = 0;
				}
				else {
					emit errorFile();
				}
			}
		}
	} catch (std::out_of_range &) {
		emit errorMessage(std::string("Unable to ") + (type == Type::Import ? "import" : "export") + " '" + path + "'\n(out_of_range exception)");
	}

	qDebug() << "result:" << result << "| skipAll:" << skipAll;

	return (bool)result || skipAll;
}

void Worker::run()
{
	if (list.empty() || (type == Type::Rebuild && list.size() != 1)) {
		return;
	}

	if (afs != nullptr) {
		if (status & 2) {
			emit toAdjust(true);
		}
		else {
			qDebug() << "afs address:" << afs;

			for (; iter != list.end(); ++iter) {
				if (!work(iter->first, iter->second)) {
					requestInterruption();
					break;
				}
			}

			if (!isInterruptionRequested()) {
				emit done();
				if (type == Type::Rebuild) {
					emit rebuilded(list.begin()->second);
				}
			}
		}
	}
}

void Worker::skipFile()
{
	if (!isRunning()) {
		emit next();
		++errors;
		if (iter != list.end()) {
			++iter;
		}
		start();
	}
}

void Worker::removeStatusRS(uint8_t flag)
{
	status &= ~flag;
}

void Worker::terminate()
{
	QThread::terminate();
	emit abort();
}
