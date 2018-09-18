#include <AFLCore.h>

#include <cstring>

using namespace Shared;

bool isAFL(const std::string &file)
{
	std::ifstream inFile(file, std::ios::in | std::ios::binary);

	uint32_t buffer = 0;

	/* Check header */
	for (auto i : AFL_File::getHeader()) {
		inFile.read(reinterpret_cast<char *>(&buffer), 4);
		if (inFile.eof() || buffer != i) {
			inFile.close();
			return false;
		}
	}

	/* Count files */
	inFile.read(reinterpret_cast<char *>(&buffer), 4);
	if (inFile.eof() || buffer == 0) { // if there is no file list exits
		inFile.close();
		return false;
	}

	/* Check file integrity */
	inFile.seekg(0, std::ios::end);
	uint32_t dim = (uint32_t)inFile.tellg() - 16; // check file dimension without header
	if (dim % FILENAME_SIZE != 0 || dim / FILENAME_SIZE != buffer) {
		inFile.close();
		return false; // in this case I guess that AFL is corrupted
	}

	inFile.close();
	return true;
}

AFL_File::AFL_File(const std::string &inName) : inName(inName), outName(getDirname(inName) + getFilename(inName)), isText(!isAFL(inName))
{
	std::ifstream inFile(this->inName, std::ios::in | std::ios::binary);

	if (this->isText) {
		/* Assign correct out file name */
		this->outName += ".afl";

		/* Copy in memory all lines from file (if possible) */
		while (!inFile.eof()) {
			std::string filename;
			getLine(inFile, filename, FILENAME_SIZE);
			if (!filename.empty()) {
				list.push_back(filename);
			}
		}
	}
	else {
		/* Assign correct out file name */
		this->outName += ".txt";

		/* Check line/s number */
		inFile.seekg(12, std::ios::beg);
		uint32_t size = 0;
		inFile.read(reinterpret_cast<char *>(&size), 4);

		/* Copy in memory all lines from file */
		list = std::vector<std::string>(size);

		for (uint32_t i = 0; i < size; i++) {
			char filename[FILENAME_SIZE];
			inFile.read(reinterpret_cast<char *>(&filename), FILENAME_SIZE);

			bool isValid = false;
			for (auto c : filename) {
				if (c == nullbyte) {
					isValid = true;
					break;
				}
			}

			if (isValid) {
				list[i] = filename;
			}
			else {
				list[i] = std::string(filename, FILENAME_SIZE);
			}
		}
	}

	inFile.close(); // FILE is now FREE!
}

AFL_File::~AFL_File() = default;

std::array<uint32_t, 3> AFL_File::getHeader()
{
	return {0x004C4641U, 0x00000001U, 0xFFFFFFFFU};
}

const std::string &AFL_File::getOutName() const
{
	return outName;
}

uint32_t AFL_File::getFileCount() const
{
	return (uint32_t)list.size();
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
	auto size = getFileCount();

	std::ofstream outFile;
	if (isText) {
		outFile.open(outName, std::ios::out | std::ios::binary);
		if (outFile.is_open()) {
			for (auto i : AFL_File::getHeader()) {
				outFile.write(reinterpret_cast<const char *>(&i), 4); // write AFL header
			}

			outFile.write(reinterpret_cast<const char *>(&size), 4); // write file count

			for (uint32_t i = 0; i < size; ++i) {
				char filename[FILENAME_SIZE];
				strncpy(filename, list[i].c_str(), FILENAME_SIZE);
				outFile.write(reinterpret_cast<const char *>(&filename), FILENAME_SIZE);
			}
		}
		else {
			outFile.setstate(std::ios::failbit);
		}
	}
	else {
		outFile.open(outName, std::ios::out);
		if (outFile.is_open()) {
			for (uint32_t i = 0; i < size; ++i) {
				outFile << list[i].c_str() << std::endl; // write line per line
			}
		}
		else {
			outFile.setstate(std::ios::failbit);
		}
	}

	outFile.close();

	return !outFile.fail();
}
