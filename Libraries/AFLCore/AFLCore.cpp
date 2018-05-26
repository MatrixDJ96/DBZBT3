#include "AFLCore.h"

using namespace Shared;

bool isAFL(const std::string &file)
{
	std::ifstream inFile(file, std::ios::in | std::ios::binary);
	bool isAFL = true;

	/* Check header */
	uint32_t buffer = 0;
	for (const uint32_t &i : aflHeader) {
		inFile.read(reinterpret_cast<char *>(&buffer), 4);
		if (inFile.eof() || buffer != i) {
			isAFL = false;
			break;
		}
	}

	if (isAFL) {
		/* Count files */
		inFile.read(reinterpret_cast<char *>(&buffer), 4);
		if (inFile.eof() || buffer == 0) { // if there is no file list exits
			isAFL = false;
		}
	}

	if (isAFL) {
		/* Check file integrity */
		inFile.seekg(0, std::ios::end);
		uint32_t dim = uint32_t(inFile.tellg()) - 16U; // check file dimension without header
		if (dim % 32U != 0 || dim / 32U != buffer) {
			isAFL = false;
		} // in this case I guess that AFL is corrupted
	}

	inFile.close();

	return isAFL;
}

AFL_File::AFL_File(const std::string &inName) : inName(inName)
{
	std::ifstream inFile;

	this->isText = isAFL(this->inName);
	if (this->isText) {
		inFile.open(this->inName, std::ios::in);

		/* Assign correct out file name */
		this->outName = getFileBaseName(this->inName, ".afl");
		this->outName += ".txt";

		/* Check line/s number */
		inFile.seekg(sizeof(aflHeader));
		uint32_t size = 0;
		inFile.read(reinterpret_cast<char *>(&size), 4);

		/* Copy in memory all lines from file */
		for (uint32_t i = 0U; i < size; i++) {
			char *filename = new char[33];
			inFile.read(filename, 32);
			filename[32] = '\0';
			list.push_back(filename);
			delPointerArray(filename);
		}
	}
	else {
		inFile.open(this->inName, std::ios::in | std::ios::binary);

		/* Assign correct out file name */
		this->outName = getFileBaseName(this->inName, ".txt");
		this->outName += ".afl";

		/* Copy in memory all lines from file (if possible) */
		inFile.seekg(0, std::ios::beg);
		std::string filename;

		while (!inFile.eof()) {
			getLine(inFile, filename);
			if (!filename.empty()) {
				list.push_back(filename);
			}
		}
	}

	inFile.close(); // FILE is now FREE!
}

AFL_File::~AFL_File() = default;

const std::string &AFL_File::getOutName() const
{
	return outName;
}

size_t AFL_File::getFileCount() const
{
	return list.size();
}

const std::vector<std::string> &AFL_File::getFileList() const
{
	return list;
}

void AFL_File::setOutName(const std::string &outName)
{
	this->outName = outName;
}

bool AFL_File::Convert() const
{
	if (!createFile(outName)) {
		return false;
	} // unable to write file

	size_t size = getFileCount();

	std::ofstream outFile;
	if (isText) {
		outFile.open(outName, std::ios::out);

		for (size_t i = 0; i < size; i++) {
			outFile << list[i].c_str() << std::endl; // write line per line
		}
	}
	else {
		outFile.open(outName, std::ios::out | std::ios::binary);

		for (const uint32_t &i : aflHeader) {
			outFile.write(reinterpret_cast<const char *>(&i), 4); // write AFL header
		}

		outFile.write(reinterpret_cast<const char *>(&size), 4); // write file count

		uint8_t byte;
		for (size_t i = 0; i < size; ++i) {
			byte = (uint8_t)this->list[i].size();
			outFile.write(this->list[i].c_str(), byte > 32 ? 32 : byte);
			if (byte < 33) {
				for (uint8_t b = byte; b < 32; ++b) {
					outFile.write(&nullbyte, 1);
				}
			}
		}
	}

	outFile.close();

	return !outFile.fail();
}
