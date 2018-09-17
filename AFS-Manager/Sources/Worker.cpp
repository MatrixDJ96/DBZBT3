#include <Worker.h>

#include <MessageBox.h>

using namespace Shared;

Worker::Worker(Type type, AFS_File *afs, const std::map<uint32_t, std::string> &list, QObject *parent) : type(type), afs(afs), errors(0), list(list), iter(this->list.begin()), skipAll(false), QThread(parent)
{
	if (type != Type::Export && type != Type::Import && type != Type::Rebuild) {
		throw std::out_of_range("Wrong Worker type!");
	}

	setTerminationEnabled(true);

	/*constexpr int size = 128 * 1024 * 1024;
	buffer = new char[size];

	for (int i = 0; i < size; ++i) {
		buffer[i] = nullbyte;
	}*/
}

Worker::~Worker()
{
	//delPointerArray(buffer);
}

int Worker::getErrors() const
{
	return errors;
}

uint32_t Worker::getPosition() const
{
	return iter->first;
}

void Worker::setSkipAll(bool flag)
{
	skipAll = flag;
}

uint8_t Worker::work(uint32_t index, const std::string &path)
{
	uint8_t result = 0;

	auto filename = afs->getFilename(index);

	if (!isInterruptionRequested()) {
		if (type == Type::Export) {
			emit progressText(QString::fromLocal8Bit(("Exporting '" + filename + "'...").c_str()));
			result = (uint8_t)afs->exportFile(index, path);
		}
		else if (type == Type::Import) {
			emit progressText(QString::fromLocal8Bit(("Importing '" + getFileBasename(path) + "' over '" + filename + "'...").c_str()));
			result = afs->importFile(index, path);
		}
		else if (type == Type::Rebuild) {
			emit progressText(QString::fromLocal8Bit(("Rebuilding '" + getFileBasename(afs->afsName) + "' to '" + getFileBasename(path) + "'...").c_str()));
			result = (uint8_t)afs->rebuild(path);
		}

		if (result == 1 || skipAll) {
			emit next();
			if (type == Type::Import) {
				emit refreshRow(index);
			}
		}
		else {
			emit errorFile(result);
		}
	}

	return (uint8_t)(skipAll ? 1 : result);
}

void Worker::run()
{
	for (; iter != list.end(); ++iter) {
		if (work(iter->first, iter->second) != 1) {
			requestInterruption();
			break;
		}
	}

	if (!isInterruptionRequested()) {
		emit done();
	}
}

void Worker::terminate()
{
	QThread::terminate();
	emit abort();
}

void Worker::skipFile()
{
	if (!isRunning()) {
		emit next();
		++errors;
		++iter;
		start();
	}
}
