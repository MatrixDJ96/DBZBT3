#ifndef UNPACKER_H
#define UNPACKER_H

#include <QThread>

#include <AFSCore.h>

class Worker : public QThread
{
Q_OBJECT
public:
	class ReservedSpace
	{
	public:
		enum class Status : uint8_t
		{
			Ok = 0x0001, NoSpace = 0x0001, TooMuchSpace = 0x0010, Both = 0x0011
		};
	public:
		ReservedSpace(Status status);

		~ReservedSpace();

		bool has(Status status);

		ReservedSpace &add(Status flag);

		ReservedSpace &remove(Status flag);

		bool operator==(const ReservedSpace &rs);

		bool operator!=(const ReservedSpace &rs);

		ReservedSpace& operator=(Status status);

	private:
		Status status;
	};

	Worker(Shared::Type type, AFS_File *afs, const std::map<uint32_t, std::string> &list, QObject *parent = nullptr);

	~Worker() override;

	int getErrors() const;

	uint32_t getPosition() const;

	void checkReservedSpace();

	std::map<uint32_t, std::string> getList() const;

	void setSkipAll(bool flag);

	void updateAFS(AFS_File *afs);

protected:
	void run() override;

private:
	bool work(uint32_t index, const std::string &path);

public:
	const Shared::Type type;
	ReservedSpace rsStatus;

private:
	AFS_File *afs;
	//char *buffer;
	char *content;
	int errors;
	std::map<uint32_t, std::string> list;
	std::map<uint32_t, std::string>::iterator iter;
	bool skipAll;

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
