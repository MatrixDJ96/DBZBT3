#include <Worker.h>

#include <QDebug>

#include <MessageBox.h>

using namespace Shared;

Worker::Worker(Type type, AFS_File *afs, const std::map<uint32_t, std::string> &list, QObject *parent) : type(type), afs(afs), errors(0), list(list), iter(this->list.begin()), isValid(true), skipAll(false), content(nullptr), QThread(parent)
{
	if (type != Type::Export && type != Type::Import && type != Type::Rebuild) {
		throw std::out_of_range("Wrong Worker type!");
	}

	setTerminationEnabled(true);

	checkReservedSpace();

	/*constexpr int size = 64 * 1024 * 1024;
	buffer = new char[size];

	for (int i = 0; i < size; ++i) {
		buffer[i] = nullbyte;
	}*/

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

bool Worker::checkReservedSpace()
{
	isValid = true;

	if (type == Type::Import) {
		for (auto item : list) {
			auto size = getFileSize(item.second);
			if (size > afs->getReservedSpace(item.first).first) {
				isValid = false;
				break;
			}
		}
	}

	return isValid;
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
}

bool Worker::work(uint32_t index, const std::string &path)
{
	bool result = false;

	auto filename = afs->getFilename(index);

	qDebug() << "content is" << (content != nullptr ? "working" : "free");

	try {
		if (!isInterruptionRequested()) {
			if (type == Type::Export) {
				emit progressText(QString::fromLocal8Bit(("Exporting '" + filename + "'...").c_str()));
				result = afs->exportFile(index, path, content);
			}
			else if (type == Type::Import) {
				emit progressText(QString::fromLocal8Bit(("Importing '" + getFileBasename(path) + "' over '" + filename + "'...").c_str()));
				result = afs->importFile(index, path, content);
			}
			else if (type == Type::Rebuild) {
				emit progressText(QString::fromLocal8Bit(("Rebuilding '" + getFileBasename(afs->afsName) + "' to '" + getFileBasename(path) + "'...").c_str()));
				result = afs->rebuild(path, content);
			}

			if (result == 1 || skipAll) {
				emit next();
				if (type == Type::Import) {
					emit refreshRow(index);
				}
			}
			else {
				emit errorFile();
			}
		}
	} catch (std::out_of_range) {
		emit errorMessage(std::string("Unable to ") + (type == Type::Import ? "import" : "export") + " '" + path + "'\n(out_of_range exception)");
	}

	return result || skipAll;
}

void Worker::run()
{
	if (list.empty() || (type == Type::Rebuild && list.size() != 1)) {
		return;
	}

	if (afs != nullptr) {
		if (!isValid && iter != list.end() && !skipAll) {
			emit toAdjust();
		}
		else {
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

void Worker::terminate()
{
	QThread::terminate();
	emit abort();
}
