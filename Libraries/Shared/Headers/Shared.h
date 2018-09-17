#ifndef SHARED_H
#define SHARED_H

#include <iostream>
#include <fstream>
#include <string>

#define OOF_STRING "Out of range!"

namespace Shared
{
	/* This is a byte with value at 0 */
	static const char nullbyte = '\0';

	enum class Type
	{
		Export, Import, Rebuild, Loading
	};

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

	/* Check if file exists */
	bool fileExists(const std::string &path);

	/* Create file and return result (bool) */
	bool createFile(const std::string &path);

	/* Function to tranform uppercase chars into lowercase chars */
	std::string getLowercase(const std::string &str);

	/* Function to get filename without dirname from path */
	std::string getFileBasename(const std::string &path);

	/* Function to get filename without dirname and extension from path */
	std::string getFilename(const std::string &path);

	/* Function to get dirname from path */
	std::string getDirname(const std::string &path);

	/* This function will change '\\' to '/' in a std::string */
	std::string backSlashtoSlash(const std::string &str);

	/* A better why to get a line (thanks stackoverflow.com) */
	void getLine(std::istream &in, std::string &line, int max_size = 0);

	/* Function to write nullbytes in a file */
	bool eraseContent(std::ofstream &outFile, uint64_t pos, uint64_t size);

	/* Function to copy content from inFile to outFile */
	bool writeContent(std::ifstream &inFile, uint64_t inPos, std::ofstream &outFile, uint64_t outPos, uint64_t size);

	/* Function to get string (in KB, MB or GB) from size */
	std::string getStringSize(const uint64_t &size);

	/* Function to truncate file size */
	bool truncateFile(const std::string &path, long size);

	/* Return used RAM from current process (if return 0 is an error) */
	size_t getUsedRam();
}

#endif // SHARED_H
