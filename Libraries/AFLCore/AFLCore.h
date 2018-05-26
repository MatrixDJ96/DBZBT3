#ifndef AFLCORE_H
#define AFLCORE_H

#include <vector>

#include "../Shared/Shared.h"

/* Default AFL header */
const uint32_t aflHeader[] = {0x004C4641, 0x00000001, 0xFFFFFFFF};

class AFL_File
{
public:
	explicit AFL_File(const std::string &inName);

	~AFL_File();

	const std::string &getOutName() const;

	size_t getFileCount() const;

	const std::vector<std::string> &getFileList() const;

	void setOutName(const std::string &outName);

	bool Convert() const;

private:
	std::string inName;
	std::string outName;
	std::vector<std::string> list;
	bool isText; // false -> afl; true -> !afl
};

#endif // AFLCORE_H
