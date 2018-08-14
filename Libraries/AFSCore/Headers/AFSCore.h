#ifndef AFSCORE_H
#define AFSCORE_H

#include "AFLCore.h"

struct FileInfo
{
	FileInfo() : address(0), size(0)
	{
	}

	uint32_t address, size;
};

struct FileDesc
{
	FileDesc() : name(""), year(0), month(0), day(0), hour(0), minute(0), second(0), size(0)
	{
	}

	char name[33]; // name[32] equals to '\0'
	uint16_t year, month, day, hour, minute, second;
	uint32_t size;
};

/* Default AFS header */
const uint32_t afsHeader = 0x00534641;

struct AFS_Error
{
	AFS_Error() : unableToOpen(false), notAFS(false), tooLarge(false), infoSize(false), descSize(false), afsSize(false), coherency(false), blankNames(false)/*, voidZone(false)*/ // TODO -> to implement
	{
	}

	bool unableToOpen, notAFS, tooLarge, infoSize, descSize, afsSize, coherency, blankNames/*, voidZone*/; // TODO -> to implement
};

class AFS_File
{
public:
	explicit AFS_File(const std::string &afsName);

	~AFS_File();

	const AFS_Error &getError() const;

	const uint32_t &getFileCount() const;

	const std::vector<FileInfo> &getFileInfo() const;

	const std::vector<FileDesc> &getFileDesc() const;

	const uint32_t &getAFSSize() const;

	void changeFilename(const uint32_t &index, const std::string &name);

	const char *getFilename(const uint32_t &index) const;

	bool exportFile(const uint32_t &index, const std::string &path) const; // TODO -> check if file already exists

	bool exportCommon(const std::string &path) const; // TODO -> check if file already exists

	bool exportRAW(const std::string &path) const; // TODO -> remove, only for testing purpose

	uint8_t importFile(const uint32_t &index, const std::string &path);

	bool importCommon(const std::string &path);

	bool importRAW(const std::string &path); // TODO -> remove, only for testing purpose

	bool commitFileInfo() const;

	bool commitFileDesc() const;

private:
	bool getFile(const uint32_t &index, char *&file) const;

	bool openAFS(std::fstream &afs, const std::ios::openmode &mode) const;

	void loadFileInfo(std::fstream &inFile);

	void loadFileDesc(std::fstream &inFile);

public:
	const std::string afsName;

private:
	AFS_Error error;
	uint32_t afsSize;
	uint32_t fileCount;
	std::vector<FileInfo> fileInfo;
	std::vector<FileDesc> fileDesc;
};

#endif // AFSCORE_H