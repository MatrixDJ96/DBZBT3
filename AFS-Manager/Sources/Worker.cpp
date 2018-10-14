#include <Worker.h>

#include <QDebug>

#include <MessageBox.h>

using namespace Shared;

Worker::ReservedSpace::ReservedSpace(Worker::ReservedSpace::Status status) : status((uint8_t)status)
{
}

Worker::ReservedSpace::~ReservedSpace() = default;

bool Worker::ReservedSpace::has(Worker::ReservedSpace::Status status)
{
	//qDebug() << this->status << "has" << (uint8_t)status << "=" << ((this->status == (this->status | (uint8_t)status)) ? "true" : "false");
	return (this->status == (this->status | (uint8_t)status));
}

Worker::ReservedSpace &Worker::ReservedSpace::add(Worker::ReservedSpace::Status flag)
{
	//qDebug() << this->status << "add" << (uint8_t)flag << "=" << (this->status | (uint8_t)flag);
	this->status |= (uint8_t)flag;
	return *this;
}

Worker::ReservedSpace &Worker::ReservedSpace::remove(Worker::ReservedSpace::Status flag)
{
	//qDebug() << this->status << "remove" << (uint8_t)flag << "=" << (this->status & ~(uint8_t)flag);
	this->status &= ~(uint8_t)flag;
	return *this;
}

bool Worker::ReservedSpace::operator==(const Worker::ReservedSpace &rs)
{
	//qDebug() << this->status << "==" << rs.status << (this->status == rs.status ? "true" : "false");
	return this->status == rs.status;
}

bool Worker::ReservedSpace::operator!=(const Worker::ReservedSpace &rs)
{
	//qDebug() << this->status << "!=" << rs.status << (this->status != rs.status ? "true" : "false");
	return this->status != rs.status;
}

Worker::ReservedSpace &Worker::ReservedSpace::operator=(Worker::ReservedSpace::Status status)
{
	//qDebug() << this->status << "=" << (uint8_t)status;
	this->status = (uint8_t)status;
	return *this;
}

Worker::Worker(Type type, AFS_File *afs, const std::map<uint32_t, std::string> &list, QObject *parent) : afs(afs),/* buffer(nullptr),*/ content(nullptr), errors(0), list(list), iter(this->list.begin()), rsStatus(ReservedSpace::Status::Ok), skipAll(false), type(type), QThread(parent)
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

void Worker::checkReservedSpace()
{
	rsStatus = ReservedSpace::Status::Ok;

	if (type == Type::Import) {
		for (const auto &item : list) {
			auto size = getFileSize(item.second);

			auto rs = afs->getReservedSpace(item.first);
			auto ors = afs->getOptimizedReservedSpace(size, AFS_File::Type::Size);

			if (size > rs.first) {
				rsStatus.add(ReservedSpace::Status::NoSpace);
			}
			else if (rs.first > ors) {
				rsStatus.add(ReservedSpace::Status::TooMuchSpace);
			}
		}
	}
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
		if (rsStatus.has(ReservedSpace::Status::TooMuchSpace)) {
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

void Worker::terminate()
{
	QThread::terminate();
	emit abort();
}
