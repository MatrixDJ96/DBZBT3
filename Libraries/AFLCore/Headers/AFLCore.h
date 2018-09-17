#ifndef AFLCORE_H
#define AFLCORE_H

#include <array>
#include <vector>
#include <Shared.h>

#define FILENAME_SIZE 32

class AFL_File
{
public:
	AFL_File(const std::string &inName);

	~AFL_File();

	const std::string &getOutName() const;

	uint32_t getFileCount() const;

	const std::vector<std::string> &getFileList() const;

	void setOutName(const std::string &outName);

	bool Convert() const;

public:
	static std::array<uint32_t, 3> getHeader();

private:
	std::string inName;
	std::string outName;
	std::vector<std::string> list;
	bool isText; // false -> afl; true -> !afl
};


#endif // AFLCORE_H
