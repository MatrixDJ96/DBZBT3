#ifndef AFSCORE_H
#define AFSCORE_H

#include <AFLCore.h>

class AFS_File
{
private:
	class FileInfo
	{
	public:
		FileInfo();

	public:
		uint32_t address;
		uint32_t size;
	};

public:
	class Error
	{
	public:
		Error();

	public:
		bool afsSize;
		bool badStream;
		bool blackFilename;
		bool coherency;
		bool corruptedContent;
		bool invalidAddress;
		bool invalidDesc;
		bool notAFS;
		bool overSize;
		bool unableToOpen;
	};

	class FileInfoExtended : public FileInfo
	{
	public:
		FileInfoExtended();

	public:
		uint32_t reservedSpace;
		uint32_t reservedSpaceRebuild;
	};

	class FileDesc
	{
	public:
		FileDesc();

		friend AFS_File;

	private:
		char name[FILENAME_SIZE];

	public:
		uint16_t year;
		uint16_t month;
		uint16_t day;
		uint16_t hour;
		uint16_t min;
		uint16_t sec;
		uint32_t size;
	};

	enum class Type
	{
		Index, Size
	};

public:
	AFS_File(const std::string &afsName);

	AFS_File(const AFS_File &afs);

	~AFS_File();

	Error getError() const;

	uint32_t getAFSSize() const;

	uint32_t getFileCount() const;

	FileInfoExtended getFileInfo(uint32_t index) const;

	const std::vector<FileInfoExtended> &getFileInfo() const;

	FileDesc getFileDesc(uint32_t index) const;

	const std::vector<FileDesc> &getFileDesc() const;

	std::string getFilename(uint32_t index) const;

	std::pair<uint32_t, uint32_t> getReservedSpace(uint32_t index) const;

	uint32_t getOptimizedReservedSpace(uint32_t num, Type type = Type::Index) const;

	std::pair<bool, bool> hasOverSpace(uint32_t index) const;

	bool commitFileInfo() const;

	bool commitFileDesc() const;

	bool exportFile(uint32_t index, const std::string &path, char *&content) const; // TODO -> check if file already exists

	bool exportAFLCommon(const std::string &path) const; // TODO -> check if file already exists

	uint8_t importFile(uint32_t index, const std::string &path, char *&content);

	uint8_t importAFLCommon(const std::string &path);

	void changeFilename(uint32_t index, const char *newFilename);

	bool changeReservedSpace(uint32_t index, uint32_t newReservedSpace);

	bool fixInvalidDesc();

	bool fixOverSize();

	void optimize();

	bool rebuild(const std::string &path, char *&content);

private:
	bool openAFS(std::fstream &inFile, const std::ios::openmode &mode) const;

	bool loadFileInfo(std::fstream &inFile);

	bool loadFileDesc(std::fstream &inFile, bool constructor = false);

public:
	static uint32_t getHeader();

	const std::string afsName;

private:
	Error error;
	uint32_t afsSize;
	uint32_t fileCount;
	std::vector<FileInfoExtended> fileInfo;
	std::vector<FileDesc> fileDesc;
};

#endif // AFSCORE_H
