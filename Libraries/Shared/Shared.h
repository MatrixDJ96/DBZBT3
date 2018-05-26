#ifndef SHARED_H
#define SHARED_H

#include <iostream>
#include <fstream>
#include <string>

/* This is a byte with value at 0 */
static const char nullbyte = '\0';

/* Function to safe-delete pointer */
template<typename T>
void delPointer(T *&t)
{
	if (t != nullptr) {
		delete t;
		t = nullptr;
	}
}

/* Function to safe-delete pointer array */
template<typename T>
void delPointerArray(T *&t)
{
	if (t != nullptr) {
		delete[] t;
		t = nullptr;
	}
}

namespace Shared
{
	/* Check if file exists */
	bool fileExists(const std::string &file);

	/* Create file and return result (bool) */
	bool createFile(const std::string &file);

	/* Function to tranform uppercase chars into lowercase chars */
	std::string getLowercase(const std::string &name);

	/* Function to get filename without path */
	std::string getFilename(const std::string &file);

	/* Function to get path from filename (if std::string(file) doesn't contain path it will return path = "") */
	std::string getFilePath(const std::string &file);

	/* Function to get filename without (custom) extension */
	std::string getFileBaseName(const std::string &file, const char *ext = nullptr);

	/* This function will change '\\' to '/' in a std::string */
	std::string backSlashtoSlash(const std::string &str);

	/* A better why to get a line (thanks stackoverflow.com) */
	void getLine(std::istream &in, std::string &line);

	/* Function to write nullbytes in a file */
	bool eraseContent(std::ofstream &file, const uint64_t &size);

	/* Function to copy content from inFile to outFile */
	bool writeContent(std::ifstream &inFile, std::ofstream &outFile, const uint64_t &size);

	/* Function to get string (in KB, MB or GB) from size */
	std::string getStringSize(const uint64_t &size);

	/* Return used RAM from current process (if return 0 is an error) */
	size_t getUsedRam();
}

#endif // SHARED_H
