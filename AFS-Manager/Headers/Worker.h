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

	bool checkReservedSpace();

	std::map<uint32_t, std::string> getList() const;

	void setSkipAll(bool flag);

	void updateAFS(AFS_File *afs);

protected:
	void run() override;

private:
	bool work(uint32_t index, const std::string &path);

public:
	const Shared::Type type;

private:
	AFS_File *afs;
	int errors;
	bool isValid;
	std::map<uint32_t, std::string> list;
	std::map<uint32_t, std::string>::iterator iter;
	bool skipAll;
	char *buffer;
	char *content;

public slots:
	void skipFile();

	void terminate();

signals:
	void abort();

	void done();

	void errorFile();

	void next();

	void rebuilded(const std::string& path);

	void refreshRow(uint32_t index);

	void toAdjust();

	void progressText(const QString &text);
};

#endif // UNPACKER_H
