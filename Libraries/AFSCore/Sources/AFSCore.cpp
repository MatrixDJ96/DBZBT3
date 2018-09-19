#include <AFSCore.h>

#include <chrono>
#include <cstring>
#include <direct.h>
#include <sstream>

#include <QString>
#include <MessageBox.h>

using namespace Shared;

AFS_File::Error::Error() : afsSize(false), badStream(false), blackFilename(false), coherency(false), corruptedContent(false), invalidAddress(false), invalidDesc(false), notAFS(false), overSize(false), unableToOpen(false)
{
}

AFS_File::FileInfoExtended::FileInfoExtended() : reservedSpace(0), reservedSpaceRebuild(0)
{
}

AFS_File::FileInfo::FileInfo() : address(0), size(0)
{
}

AFS_File::FileDesc::FileDesc() : name(""), year(0), month(0), day(0), hour(0), min(0), sec(0), size(0)
{
}

AFS_File::AFS_File(const std::string &afsName) : afsName(backSlashtoSlash(afsName)), afsSize(0), fileCount(0)
{
	/*char str[512];
	getcwd(str, 512);*/

	std::fstream inFile;
	if (!openAFS(inFile, std::ios::in)) {
		error.unableToOpen = true; // could not open afs
		return;
	}

	uint32_t header = 0;
	inFile.read(reinterpret_cast<char *>(&header), 4);
	if (header != AFS_File::getHeader()) {
		inFile.close();
		error.notAFS = true; // this is not an afs
		return;
	}

	inFile.read(reinterpret_cast<char *>(&fileCount), 4); // get fileCount
	if (fileCount == 0) {
		// TODO -> create emptyAFS error
		inFile.close();
		error.notAFS = true; // this is an empty afs (?)
		return;
	}

	inFile.seekg(0, std::ios::end);
	afsSize = (uint32_t)inFile.tellg();

	fileInfo = std::vector<FileInfoExtended>(fileCount + 1);

	if (!loadFileInfo(inFile)) {
		inFile.close();
		error.badStream = true;
		return; // read/writing error on i/o operation
	}

	/* Calculate descriptor size */
	uint32_t descSize = sizeof(FileDesc) * fileCount;
	if (fileInfo[fileCount].size != descSize) {
		fileInfo[fileCount].size = descSize;
		error.coherency = true;
	}

	/* An AFS is divided in 3 part
	 *  1) File information (size and address)
	 *  2) File data
	 *  3) File descriptions (name, date of last modification)
	 */

	fileDesc = std::vector<FileDesc>(fileCount);

	if (!loadFileDesc(inFile, true)) {
		inFile.close();
		if (!error.invalidAddress && !error.invalidDesc) {
			error.badStream = true;
		}
		return; // descriptor error
	}

	/* Process the information (only in memory) */
	for (uint32_t i = 0; i < fileCount; ++i) {
		fileInfo[i].reservedSpace = fileInfo[i + 1].address - fileInfo[i].address;
		fileInfo[i].reservedSpaceRebuild = fileInfo[i].reservedSpace;

		if (fileDesc[i].size != fileInfo[i].size) {
			fileDesc[i].size = fileInfo[i].size;
			error.coherency = true;
		}
	}

	if (afsSize % 2048 != 0) {
		error.overSize = true;
	}
	else {
		fileInfo[fileCount].reservedSpace = afsSize - fileInfo[fileCount].address;
		fileInfo[fileCount].reservedSpaceRebuild = getOptimizedReservedSpace(fileCount);
	}


	inFile.close();
}

AFS_File::AFS_File(const AFS_File &afs) : afsName(afs.afsName), error(afs.error), afsSize(afs.afsSize), fileCount(afs.fileCount), fileInfo(afs.fileInfo), fileDesc(afs.fileDesc)
{
}

AFS_File::~AFS_File() = default;

uint32_t AFS_File::getHeader()
{
	return 0x00534641U;
}

bool AFS_File::openAFS(std::fstream &inFile, const std::ios::openmode &mode) const
{
	if (inFile.is_open()) {
		inFile.close();
	}

	inFile.open(afsName, mode | std::ios::binary);
	return inFile.is_open();
}

bool AFS_File::loadFileInfo(std::fstream &inFile)
{
	inFile.seekg(8, std::ios::beg);

	uint32_t i, fileCount = this->fileCount + 1; // + 1 is required to store afl descriptor

	for (i = 0; i < fileCount && !inFile.fail() && !inFile.eof(); ++i) {
		inFile.read(reinterpret_cast<char *>(&fileInfo[i].address), sizeof(FileInfo::address));
		inFile.read(reinterpret_cast<char *>(&fileInfo[i].size), sizeof(FileInfo::size));
	}

	return !inFile.fail() && !inFile.eof();
}

bool AFS_File::loadFileDesc(std::fstream &inFile, bool constructor)
{
	if (constructor) {
		for (uint32_t i = 0; i < fileCount; ++i) {
			if (fileInfo[i].address == 0 || fileInfo[i].address % 2048 != 0 || (i + 1 != fileCount && (fileInfo[i + 1].address % 2048 != 0 || fileInfo[i].address > fileInfo[i + 1].address || fileInfo[i].size > fileInfo[i + 1].address - fileInfo[i].address))) {
				error.invalidAddress = true;
				return false;
			}

			if (fileInfo[i].address + fileInfo[i].size > afsSize) {
				error.corruptedContent = true;
			}
		}

		if (fileInfo[fileCount].address == 0 || fileInfo[fileCount].address % 2048 != 0 || fileInfo[fileCount - 1].address > fileInfo[fileCount].address || fileInfo[fileCount - 1].size > fileInfo[fileCount].address - fileInfo[fileCount - 1].address || fileInfo[fileCount].address + ((fileInfo[fileCount].size / 2048) * 2048 + (fileInfo[fileCount].size % 2048 != 0 ? 2048 : 0)) > afsSize) {
			error.invalidDesc = true;
			return false;
		}
	}

	inFile.seekg(fileInfo[fileCount].address, std::ios::beg);

	for (uint32_t i = 0; i < fileCount && !inFile.fail() && !inFile.eof(); ++i) {
		inFile.read(reinterpret_cast<char *>(&fileDesc[i].name), sizeof(FileDesc::name));

		for (auto c : fileDesc[i].name) {
			if (c == nullbyte) {
				if (std::string(fileDesc[i].name).empty()) {
					std::stringstream filename;
					filename << "blank_" << (i + 1);
					strncpy(fileDesc[i].name, filename.str().c_str(), sizeof(FileDesc::name));
					if (constructor) {
						error.blackFilename = true;
					}
				}
				break;
			}
		}

		inFile.read(reinterpret_cast<char *>(&fileDesc[i].year), sizeof(FileDesc::year));
		inFile.read(reinterpret_cast<char *>(&fileDesc[i].month), sizeof(FileDesc::month));
		inFile.read(reinterpret_cast<char *>(&fileDesc[i].day), sizeof(FileDesc::day));
		inFile.read(reinterpret_cast<char *>(&fileDesc[i].hour), sizeof(FileDesc::hour));
		inFile.read(reinterpret_cast<char *>(&fileDesc[i].min), sizeof(FileDesc::min));
		inFile.read(reinterpret_cast<char *>(&fileDesc[i].sec), sizeof(FileDesc::sec));
		inFile.read(reinterpret_cast<char *>(&fileDesc[i].size), sizeof(FileDesc::size));
	}

	return !inFile.fail() && !inFile.eof();
}

AFS_File::Error AFS_File::getError() const
{
	return error;
}

uint32_t AFS_File::getAFSSize() const
{
	return afsSize;
}

uint32_t AFS_File::getFileCount() const
{
	return fileCount;
}

AFS_File::FileInfoExtended AFS_File::getFileInfo(uint32_t index) const
{
	if (index > fileCount) {
		throw std::out_of_range(OOF_STRING);
	}

	return fileInfo[index];
}

const std::vector<AFS_File::FileInfoExtended> &AFS_File::getFileInfo() const
{
	return fileInfo;
}

AFS_File::FileDesc AFS_File::getFileDesc(uint32_t index) const
{
	if (index >= fileCount) {
		throw std::out_of_range(OOF_STRING);
	}

	return fileDesc[index];
}

const std::vector<AFS_File::FileDesc> &AFS_File::getFileDesc() const
{
	return fileDesc;
}

std::string AFS_File::getFilename(uint32_t index) const
{
	if (index >= fileCount) {
		if (index > fileCount) {
			throw std::out_of_range(OOF_STRING);
		}
		else {
			return "FILE DESCRIPTOR";
		}
	}

	bool isValid = false;

	for (auto c : fileDesc[index].name) {
		if (c == nullbyte) {
			isValid = true;
			break;
		}
	}

	std::string filename;

	if (isValid) {
		filename = fileDesc[index].name;
	}
	else {
		filename = std::string(fileDesc[index].name, sizeof(FileDesc::name));
	}

	return filename;
}

std::pair<uint32_t, uint32_t> AFS_File::getReservedSpace(uint32_t index) const
{
	if (index > fileCount) {
		throw std::out_of_range(OOF_STRING);
	}
	else {
		return {fileInfo[index].reservedSpace, fileInfo[index].reservedSpaceRebuild};
	}
}

uint32_t AFS_File::getOptimizedReservedSpace(uint32_t num, Type type) const
{
	if (type == Type::Index) {
		if (num > fileCount) {
			throw std::out_of_range(OOF_STRING);
		}
		return ((fileInfo[num].size / 2048) * 2048 + (fileInfo[num].size % 2048 != 0 ? 2048 : 0));
	}
	else {
		return ((num / 2048) * 2048 + (num % 2048 != 0 ? 2048 : 0));
	}
}

std::pair<bool, bool> AFS_File::hasOverSpace(uint32_t index) const
{
	if (index > fileCount) {
		throw std::out_of_range(OOF_STRING);
	}

	auto ors = getOptimizedReservedSpace(index);

	return {(fileInfo[index].reservedSpace != ors), (fileInfo[index].reservedSpaceRebuild != ors)};
}

bool AFS_File::commitFileInfo() const
{
	std::fstream outFile;
	if (!openAFS(outFile, std::ios::in | std::ios::out)) {
		return false;
	}

	outFile.seekp(8, std::ios::beg);

	for (uint32_t i = 0; i <= fileCount; ++i) {
		outFile.write(reinterpret_cast<const char *>(&fileInfo[i].address), sizeof(FileInfo::address));
		outFile.write(reinterpret_cast<const char *>(&fileInfo[i].size), sizeof(FileInfo::size));
	}

	outFile.close();

	return !outFile.fail();
}

bool AFS_File::commitFileDesc() const
{
	std::fstream outFile;
	if (!openAFS(outFile, std::ios::in | std::ios::out)) {
		return false;
	}

	outFile.seekp(fileInfo[fileCount].address, std::ios::beg);

	for (uint32_t i = 0; i < fileCount; ++i) {
		outFile.write(reinterpret_cast<const char *>(&fileDesc[i].name), sizeof(FileDesc::name));
		outFile.write(reinterpret_cast<const char *>(&fileDesc[i].year), sizeof(FileDesc::year));
		outFile.write(reinterpret_cast<const char *>(&fileDesc[i].month), sizeof(FileDesc::month));
		outFile.write(reinterpret_cast<const char *>(&fileDesc[i].day), sizeof(FileDesc::day));
		outFile.write(reinterpret_cast<const char *>(&fileDesc[i].hour), sizeof(FileDesc::hour));
		outFile.write(reinterpret_cast<const char *>(&fileDesc[i].min), sizeof(FileDesc::min));
		outFile.write(reinterpret_cast<const char *>(&fileDesc[i].sec), sizeof(FileDesc::sec));
		outFile.write(reinterpret_cast<const char *>(&fileDesc[i].size), sizeof(FileDesc::size));
	}

	outFile.close();

	return !outFile.fail();
}

bool AFS_File::exportFile(uint32_t index, const std::string &path, char *&content) const
{
	if (index > fileCount) {
		throw std::out_of_range(OOF_STRING);
	}

	std::fstream inFile;
	if (!openAFS(inFile, std::ios::in | std::ios::out)) {
		return false;
	}

	std::ofstream outFile(path, std::ios::out | std::ios::binary);
	if (!outFile.is_open()) {
		inFile.close();
		return false; // unable to write file;
	}

	auto result = writeContent((std::ifstream &)inFile, fileInfo[index].address, outFile, 0, fileInfo[index].size, content);

	inFile.close();
	outFile.close();

	return result;
}

bool AFS_File::exportAFLCommon(const std::string &path) const
{
	std::ofstream outFile(path, std::ios::out | std::ios::binary);
	if (!outFile.is_open()) {
		return false; // unable to write file;
	}

	for (auto i : AFL_File::getHeader()) {
		outFile.write(reinterpret_cast<const char *>(&i), 4); // write AFL header
	}

	outFile.write(reinterpret_cast<const char *>(&fileCount), 4);

	for (uint32_t i = 0; i < fileCount; ++i) {
		outFile.write(reinterpret_cast<const char *>(&fileDesc[i].name), 32);
	}

	outFile.close();

	return !outFile.fail();
}

uint8_t AFS_File::importFile(uint32_t index, const std::string &path, char *&content)
{
	// RETURN value:
	// 0 -> error;
	// 1 -> success;
	// 2 -> to resize.

	if (index >= fileCount) {
		throw std::out_of_range(OOF_STRING);
	}

	std::fstream outFile;
	if (!openAFS(outFile, std::ios::in | std::ios::out)) {
		return 0;
	}

	std::ifstream inFile(path, std::ios::in | std::ios::binary);
	if (!inFile.is_open()) {
		outFile.close();
		return 0; // unable to read file
	}

	inFile.seekg(0, std::ios::end);

	auto size = (uint32_t)inFile.tellg();

	if (size > fileInfo[index + 1].address - fileInfo[index].address) {
		inFile.close();
		outFile.close();
		return 2; // to resize
	}
	else {
		/* Clean old file inside AFS */
		eraseContent((std::ofstream &)outFile, fileInfo[index].address, fileInfo[index].size, content);  // should be added check of return value

		/* Import new file inside AFS */
		writeContent(inFile, 0, (std::ofstream &)outFile, fileInfo[index].address, size, content);

		/* Set new file size */
		fileInfo[index].size = size;
		fileDesc[index].size = size;

		auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		auto lt = localtime(&now);

		fileDesc[index].year = lt->tm_year + 1900;
		fileDesc[index].month = lt->tm_mon + 1;
		fileDesc[index].day = lt->tm_mday;
		fileDesc[index].hour = lt->tm_hour;
		fileDesc[index].min = lt->tm_min;
		fileDesc[index].sec = lt->tm_sec;

		commitFileInfo() && commitFileDesc();

		inFile.close();
		outFile.close();

		return (uint8_t)!outFile.fail();
	}
}

uint8_t AFS_File::importAFLCommon(const std::string &path)
{
	// RETURN value:
	// 0 -> error;
	// 1 -> success;
	// 2 -> incompatible afl.

	if (!(std::ifstream(path).is_open())) {
		return 0;  // unable to read file
	}

	AFL_File afl(path);

	if (afl.getFileCount() != fileCount) {
		return 2;  // incompatible afl
	}

	auto list = afl.getFileList();
	for (uint32_t i = 0; i < fileCount; ++i) {
		changeFilename(i, list[i].c_str());
	}

	return (uint8_t)commitFileDesc();
}

void AFS_File::changeFilename(uint32_t index, const char *newFilename)
{
	if (index >= fileCount) {
		throw std::out_of_range(OOF_STRING);
	}

	strncpy(fileDesc[index].name, newFilename, 32);
}

bool AFS_File::changeReservedSpace(uint32_t index, uint32_t newReservedSpace)
{
	if (index > fileCount) {
		throw std::out_of_range(OOF_STRING);
	}

	if (newReservedSpace % 2048 != 0) {
		return false;
	}
	else {
		fileInfo[index].reservedSpaceRebuild = newReservedSpace;
		return true;
	}
}

bool AFS_File::fixInvalidDesc()
{
	std::fstream outFile;
	if (!openAFS(outFile, std::ios::in | std::ios::out)) {
		return false;
	}

	uint32_t descRealSize = getOptimizedReservedSpace(fileCount);
	fileInfo[fileCount].address = (fileInfo[fileCount - 1].size != 0 ? (fileInfo[fileCount - 1].address + (fileInfo[fileCount - 1].size / 2048) * 2048 + 2048) : fileInfo[fileCount - 1].address);

	bool result = eraseContent((std::ofstream &)outFile, (afsSize < fileInfo[fileCount].address ? afsSize : fileInfo[fileCount].address), (afsSize < fileInfo[fileCount].address ? fileInfo[fileCount].address - afsSize : 0) + descRealSize);

	if (result) {
		outFile.seekp(sizeof(FileInfo) * (fileCount + 1));

		outFile.write(reinterpret_cast<const char *>(&fileInfo[fileCount].address), sizeof(FileInfo::address));
		outFile.write(reinterpret_cast<const char *>(&fileInfo[fileCount].size), sizeof(FileInfo::size));

		outFile.close();

		result = !outFile.fail();
	}

	return result;
}

bool AFS_File::fixOverSize()
{
	if (afsSize % 2048 != 0) {
		return truncateFile(afsName, (afsSize / 2048) * 2048);
	}
	else {
		return true;
	}
}

void AFS_File::optimize()
{
	for (uint32_t i = 0; i <= fileCount; ++i) {
		fileInfo[i].reservedSpaceRebuild = getOptimizedReservedSpace(i);
	}
}

bool AFS_File::rebuild(const std::string &path, char *&content)
{
	if (afsName == path) {
		return false;
	}

	auto header = AFS_File::getHeader();
	auto fileInfo = this->fileInfo;

	// update info
	uint32_t size = 16 + 8 * fileCount;
	fileInfo[0].address = getOptimizedReservedSpace(size, Type::Size);
	for (uint32_t i = 0; i <= fileCount; ++i) {
		fileInfo[i].reservedSpace = fileInfo[i].reservedSpaceRebuild;
		if (i != 0) {
			fileInfo[i].address = fileInfo[i - 1].address + fileInfo[i - 1].reservedSpace;
			if ((uint64_t)fileInfo[i - 1].address + (uint64_t)fileInfo[i - 1].reservedSpace != fileInfo[i].address) {
				return false; // unable to set correctly address
			}
		}
		if (fileInfo[i].size > fileInfo[i].reservedSpace) {
			fileInfo[i].size = fileInfo[i].reservedSpace;
		}
	}
	if ((uint64_t)fileInfo[fileCount].address + (uint64_t)fileInfo[fileCount].size != fileInfo[fileCount].address + fileInfo[fileCount].size) {
		return false; // unable to rebuild afs (too big)
	}

	std::fstream inFile;
	if (!openAFS(inFile, std::ios::in)) {
		return false; // unable to open afs
	}

	std::fstream outFile(path, std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
	if (!outFile.is_open()) {
		inFile.close();
		return false; // unable to create new afs
	}

	// write header and fileCount
	outFile.write(reinterpret_cast<const char *>(&header), sizeof(header));
	outFile.write(reinterpret_cast<const char *>(&fileCount), sizeof(fileCount));

	// write fileInfo
	for (uint32_t i = 0; i <= fileCount; ++i) {
		outFile.write(reinterpret_cast<const char *>(&fileInfo[i].address), sizeof(FileInfo::address));
		outFile.write(reinterpret_cast<const char *>(&fileInfo[i].size), sizeof(FileInfo::size));
	}

	eraseContent((std::ofstream &)outFile, size, fileInfo[0].address - size, content);

	for (uint32_t i = 0; i < fileCount; ++i) {
		writeContent((std::ifstream &)inFile, this->fileInfo[i].address, (std::ofstream &)outFile, fileInfo[i].address, fileInfo[i].size, content);
		eraseContent((std::ofstream &)outFile, fileInfo[i].address + fileInfo[i].size, fileInfo[i].reservedSpace - fileInfo[i].size, content);
	}

	for (uint32_t i = 0; i < fileCount; ++i) {
		outFile.write(reinterpret_cast<const char *>(&fileDesc[i].name), sizeof(FileDesc::name));
		outFile.write(reinterpret_cast<const char *>(&fileDesc[i].year), sizeof(FileDesc::year));
		outFile.write(reinterpret_cast<const char *>(&fileDesc[i].month), sizeof(FileDesc::month));
		outFile.write(reinterpret_cast<const char *>(&fileDesc[i].day), sizeof(FileDesc::day));
		outFile.write(reinterpret_cast<const char *>(&fileDesc[i].hour), sizeof(FileDesc::hour));
		outFile.write(reinterpret_cast<const char *>(&fileDesc[i].min), sizeof(FileDesc::min));
		outFile.write(reinterpret_cast<const char *>(&fileDesc[i].sec), sizeof(FileDesc::sec));

		outFile.write(reinterpret_cast<const char *>(&fileInfo[i].size), sizeof(FileInfo::size)); // to have an updated value
	}

	eraseContent((std::ofstream &)outFile, fileInfo[fileCount].address + fileInfo[fileCount].size, fileInfo[fileCount].reservedSpace - fileInfo[fileCount].size, content);

	inFile.close();
	outFile.close();

	return !outFile.fail();
}

