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

	uint8_t getStatusRS() const;

	uint8_t checkReservedSpace();

	std::map<uint32_t, std::string> getList() const;

	void setSkipAll(bool flag);

	void removeStatusRS(uint8_t flag);

	void updateAFS(AFS_File *afs);

protected:
	void run() override;

private:
	bool work(uint32_t index, const std::string &path);

public:
	const Shared::Type type;

private:
	AFS_File *afs;
	//char *buffer;
	char *content;
	int errors;
	std::map<uint32_t, std::string> list;
	std::map<uint32_t, std::string>::iterator iter;
	bool skipAll;
	uint8_t status;

public slots:
	void skipFile();

	void terminate();

signals:

	void abort();

	void done();

	void errorFile();

	void errorMessage(const std::string &message);

	void next();

	void rebuilded(std::string path);

	void refreshRow(uint32_t index);

	void toAdjust(bool init);

	void progressText(const QString &text);
};

#endif // UNPACKER_H
