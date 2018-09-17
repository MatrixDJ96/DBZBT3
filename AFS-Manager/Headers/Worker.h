#ifndef UNPACKER_H
#define UNPACKER_H

#include <QThread>

#include <AFSCore.h>

class Worker : public QThread
{
Q_OBJECT
public:
	Worker(Shared::Type type, AFS_File *afs, const std::map<uint32_t, std::string> &list, QObject *parent = nullptr);

	~Worker() override;

	int getErrors() const;

	uint32_t getPosition() const;

	void setSkipAll(bool flag);

protected:
	void run() override;

private:
	uint8_t work(uint32_t index, const std::string &path);

public:
	const Shared::Type type;

private:
	AFS_File *afs;
	int errors;
	std::map<uint32_t, std::string> list;
	std::map<uint32_t, std::string>::iterator iter;
	bool skipAll;
	//char *buffer;

public slots:
	void terminate();

	void skipFile();

signals:
	void abort();

	void done();

	void next();

	void errorFile(uint8_t result);

	void refreshRow(uint32_t index);

	void progressText(const QString &text);
};

#endif // UNPACKER_H
