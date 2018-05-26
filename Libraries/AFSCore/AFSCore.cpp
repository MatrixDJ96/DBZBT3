#include "AFSCore.h"

#include <sstream>

using namespace Shared;

AFS_File::AFS_File(const std::string &afsName) : afsName(afsName), fileCount(0)
{
	std::fstream inFile;
	if (!openAFS(inFile, std::ios::in)) {
		error.unableToOpen = true; // could not open afs
		return;
	}

	uint32_t firstUint = 0;
	inFile.read(reinterpret_cast<char *>(&firstUint), 4);
	if (firstUint != afsHeader) {
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

	fileInfo = std::vector<FileInfo>(fileCount + 1);
	loadFileInfo(inFile);

	if (error.infoSize) {
		inFile.close();
		return; // this is not a valid an afs
	}

	/* An AFS is divided in 3 part
	 *  1) File information (size and address)
	 *  2) File data
	 *  3) File descriptions (name, date of last modification)
	 */

	inFile.seekg(0, std::ios::end);
	if (inFile.tellg() > std::numeric_limits<uint32_t>::max()) {
		error.tooLarge = true;
		inFile.close();
		return;
	}

	afsSize = (uint32_t)inFile.tellg();

	/*if (afsSize % 2048 != 0) {
		error.afsSize = true;
	}
	if (fileInfo[0].address % 2048 != 0) {
		error.infoSize = true;
	}
	if ((afsSize - fileInfo[fileCount].address) % 2048 != 0) {
		error.descSize = true;
	}*/

	fileDesc = std::vector<FileDesc>(fileCount);
	loadFileDesc(inFile);

	if (error.descSize) {
		inFile.close();
		return; // this is not a valid an afs
	}

	/* Fix file sizes (only in memory) */
	for (uint32_t i = 0; i < fileCount; i++) {
		if (fileInfo[i].size != fileDesc[i].size) {
			/* TODO -> change size check (seek each file to check real size) */
			fileDesc[i].size = fileInfo[i].size;
			error.coherency = true;
		}
	}

	inFile.close();
}

AFS_File::~AFS_File() = default;

const AFS_Error &AFS_File::getError() const
{
	return error;
}

const uint32_t &AFS_File::getFileCount() const
{
	return fileCount;
}

const std::vector<FileInfo> &AFS_File::getFileInfo() const
{
	return fileInfo;
}

const std::vector<FileDesc> &AFS_File::getFileDesc() const
{
	return fileDesc;
}

const uint32_t &AFS_File::getAFSSize() const
{
	return afsSize;
}

void AFS_File::changeFilename(const uint32_t &index, const std::string &name)
{
	std::string newName = name;
	if (name.size() > 32) {
		newName = newName.substr(0, 32);
	}

	uint8_t size = (uint8_t)name.size();

	for (uint8_t i = 0; i < size; i++) {
		fileDesc[index].name[i] = newName[i];
	}

	for (uint8_t i = size; i <= 32; ++i) {
		fileDesc[index].name[i] = nullbyte; // define '\0'
	}
}

bool AFS_File::exportFile(const uint32_t &index, const std::string &path) const
{
	std::ofstream outFile(path, std::ios::out | std::ios::binary);
	if (!outFile.is_open()) {
		return false;
	} // unable to write file;

	char *file = nullptr;
	if (!getFile(index, file)) {
		delPointer(file);
		return false; // unable to get file
	}

	outFile.write(reinterpret_cast<const char *>(file), fileInfo[index].size);
	outFile.close();

	delPointerArray(file);

	return !outFile.fail();
}

bool AFS_File::exportCommon(const std::string &path) const
{
	std::ofstream outFile(path, std::ios::out | std::ios::binary);
	if (!outFile.is_open()) {
		return false; // unable to write file;
	}

	for (const uint32_t &i : aflHeader) {
		outFile.write(reinterpret_cast<const char *>(&i), 4); // write AFL header
	}

	outFile.write(reinterpret_cast<const char *>(&fileCount), 4);

	for (uint32_t i = 0; i < fileCount; ++i) {
		outFile.write(reinterpret_cast<const char *>(&fileDesc[i].name), 32);
	}

	outFile.close();

	return !outFile.fail();
}

bool AFS_File::exportRAW(const std::string &path) const
{
	return exportFile(fileCount, path);
}

uint8_t AFS_File::importFile(const uint32_t &index, const std::string &path)
{
	// 0 -> error;
	// 1 -> success;
	// 2 -> to resize

	std::fstream outFile;
	if (!openAFS(outFile, std::ios::in | std::ios::out)) {
		return 0;  // unable to open afs
	}

	std::ifstream inFile(path, std::ios::in | std::ios::binary);
	if (!fileExists(path)) {
		return 0;// unable to read file
	}


	inFile.seekg(0, std::ios::end);
	if (inFile.tellg() > std::numeric_limits<uint32_t>::max()) {
		return 0; // too large
	}

	uint32_t size = (uint32_t)inFile.tellg();
	inFile.seekg(0, std::ios::beg);

	if (size > fileInfo[index + 1].address - fileInfo[index].address) {
		return 2; // to resize
	}
	else {
		/* Clean old file inside AFS */
		outFile.seekg(fileInfo[index].address, std::ios::beg);
		eraseContent((std::ofstream &)outFile, fileInfo[index].size);  // should be added check of return value*/
	
		/* Import new file inside AFS */
		outFile.seekg(fileInfo[index].address, std::ios::beg);
		writeContent(inFile, (std::ofstream &)outFile, size);

		/* Set new file size */
		fileInfo[index].size = size;
		fileDesc[index].size = size;

		// TODO -> change date modified

		commitFileInfo(); // TODO -> add return value check
		commitFileDesc(); // TODO -> add return value check

		outFile.close();

		return (uint8_t)!outFile.fail();
	}

	// TODO -> complete this function
}

bool AFS_File::importCommon(const std::string &path)
{
	if (!fileExists(path)) {
		return false;  // unable to read file
	}

	AFL_File afl(path);

	if (afl.getFileCount() != fileCount) {
		return false;  // incompatible afl
	}

	const std::vector<std::string> *list = &afl.getFileList();
	for (uint32_t i = 0; i < fileCount; ++i) {
		changeFilename(i, (*list)[i]);
	}

	return commitFileDesc();
}

bool AFS_File::importRAW(const std::string &path)
{
	std::fstream outFile;
	if (!openAFS(outFile, std::ios::in | std::ios::out)) {
		return false;
	} // unable to open afs

	std::ifstream inFile(path, std::ios::in | std::ios::binary);
	if (!fileExists(path)) {
		return false;
	} // unable to open file

	/* Read RAW AFL */
	inFile.seekg(0, std::ios::end);
	uint32_t size = inFile.tellg();
	inFile.seekg(0, std::ios::beg);
	char *file = new char[size];
	inFile.read(file, size);
	inFile.close();

	if (outFile.fail()) {
		return false;
	} // unable to read file content

	/* Clean old RAW AFL inside AFS */
	uint32_t old_size = fileInfo[fileCount].size;
	outFile.seekg(fileInfo[fileCount].address, std::ios::beg);

	eraseContent((std::ofstream &)outFile, old_size); // could be added check of return value

	/* Import new RAW AFL inside AFS */
	outFile.seekg(fileInfo[fileCount].address, std::ios::beg);
	outFile.write(file, size);
	delPointerArray(file);

	/* Set new RAW AFL size */
	fileInfo[fileCount].size = size;

	commitFileInfo(); // TODO -> add return value check

	/* Load in memory new RAW AFL */
	loadFileDesc(outFile);

	outFile.close();

	return !outFile.fail();
}

bool AFS_File::commitFileInfo() const
{
	std::fstream outFile;
	if (!openAFS(outFile, std::ios::in | std::ios::out)) {
		return false; // unable to open afs
	}

	outFile.seekg(8, std::ios::beg);

	uint32_t fileCount = this->fileCount + 1; // + 1 is required to store afl adress and size
	for (uint32_t i = 0; i < fileCount; ++i) {
		outFile.write(reinterpret_cast<const char *>(&fileInfo[i].address), 4);
		outFile.write(reinterpret_cast<const char *>(&fileInfo[i].size), 4);
	}

	outFile.close();

	return !outFile.fail();
}

bool AFS_File::commitFileDesc() const
{
	std::fstream outFile;
	if (!openAFS(outFile, std::ios::in | std::ios::out)) {
		return false;
	} // unable to open afs

	outFile.seekg(fileInfo[fileCount].address, std::ios::beg);

	for (uint32_t i = 0; i < fileCount; ++i) {
		outFile.write(reinterpret_cast<const char *>(&fileDesc[i].name), 32);
		outFile.write(reinterpret_cast<const char *>(&fileDesc[i].year), 2);
		outFile.write(reinterpret_cast<const char *>(&fileDesc[i].month), 2);
		outFile.write(reinterpret_cast<const char *>(&fileDesc[i].day), 2);
		outFile.write(reinterpret_cast<const char *>(&fileDesc[i].hour), 2);
		outFile.write(reinterpret_cast<const char *>(&fileDesc[i].minute), 2);
		outFile.write(reinterpret_cast<const char *>(&fileDesc[i].second), 2);
		outFile.write(reinterpret_cast<const char *>(&fileDesc[i].size), 4);
	}

	outFile.close();

	return !outFile.fail();
}

bool AFS_File::getFile(const uint32_t &index, char *&file) const
{
	std::fstream inFile;
	if (!openAFS(inFile, std::ios::in)) {
		return false; // unable to open afs
	}

	delPointer(file); // just to be shure
	file = new char[fileInfo[index].size];

	inFile.seekg(fileInfo[index].address);
	inFile.read(file, fileInfo[index].size);
	inFile.close();

	return !inFile.fail();
}

bool AFS_File::openAFS(std::fstream &afs, const std::ios::openmode &mode) const
{
	if (!fileExists(afsName)) {
		return false; // unable to find afs
	}

	afs.open(afsName, mode | std::ios::binary);
	return afs.is_open();
}

void AFS_File::loadFileInfo(std::fstream &inFile)
{
	inFile.seekg(8, std::ios::beg);

	uint32_t fileCount = this->fileCount + 1; // + 1 is required to store afl address and size
	for (uint32_t i = 0; i < fileCount && !inFile.eof(); ++i) {
		/* TODO -> skip 0 byte and set/implement 'voidZone' error */
		inFile.read(reinterpret_cast<char *>(&fileInfo[i].address), 4);
		inFile.read(reinterpret_cast<char *>(&fileInfo[i].size), 4);
	}

	if (inFile.eof()) {
		error.infoSize = true;
	}
}

void AFS_File::loadFileDesc(std::fstream &inFile)
{
	inFile.seekg(fileInfo[fileCount].address, std::ios::beg);

	std::string name;
	for (uint32_t i = 0; i < fileCount && !inFile.eof(); ++i) {
		inFile.read(reinterpret_cast<char *>(&fileDesc[i].name), 32);
		fileDesc[i].name[32] = nullbyte; // define end of std::string

		name = fileDesc[i].name;
		if (name.empty()) {
			error.blankNames = true;
			std::stringstream nameStream;
			nameStream << "blank_" << i;
			nameStream >> fileDesc[i].name;
			//nameStream.read(reinterpret_cast<char *>(&coherency[i].name), 32);
		}

		inFile.read(reinterpret_cast<char *>(&fileDesc[i].year), 2);
		inFile.read(reinterpret_cast<char *>(&fileDesc[i].month), 2);
		inFile.read(reinterpret_cast<char *>(&fileDesc[i].day), 2);
		inFile.read(reinterpret_cast<char *>(&fileDesc[i].hour), 2);
		inFile.read(reinterpret_cast<char *>(&fileDesc[i].minute), 2);
		inFile.read(reinterpret_cast<char *>(&fileDesc[i].second), 2);
		inFile.read(reinterpret_cast<char *>(&fileDesc[i].size), 4);
	}

	if (inFile.eof()) {
		error.descSize = true;
	}
}
