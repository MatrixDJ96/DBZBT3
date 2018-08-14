#include <cstring>
#include <sstream>

#include "Shared.h"

/* 1048576 -> 1 megabyte */
static constexpr uint64_t max_size = 1048576 * 512;

bool Shared::fileExists(const std::string &file)
{
	std::ifstream in(file, std::ios::in);
	if (in.is_open()) {
		in.close();
		return true;
	}
	else {
		return false;
	}
}

bool Shared::createFile(const std::string &file)
{
	std::ofstream out(file, std::ios::out);
	if (out.is_open()) {
		out.close();
		return true;
	}
	else {
		return false;
	}
}

std::string Shared::getLowercase(const std::string &name)
{
	std::string lowername = name;

	for (char &c: lowername) {
		if (c >= 'A' && c <= 'Z') {
			c -= ('A' - 'a');
		}
	}

	return lowername;
}

std::string Shared::getFilename(const std::string &file)
{
	std::string name;

	std::size_t found = file.find_last_of("/\\");
	if (found != std::string::npos) {
		name = file.substr(found + 1);
	}
	else {
		name = file;
	}

	return name;
}

std::string Shared::getFilePath(const std::string &file)
{
	std::string path;

	std::size_t found = file.find_last_of("/\\");
	if (found != std::string::npos) {
		path = file.substr(0, found);
	}
	else {
		path = file;
	}

	return path;
}

std::string Shared::getFileBaseName(const std::string &file, const char *ext)
{
	std::string basename = getFilename(file);

	if (ext != nullptr) {
		std::size_t found = basename.find_last_of(ext);
		if (found != std::string::npos) {
			basename = basename.substr(0, found);
		}
	}

	return basename;
}

std::string Shared::backSlashtoSlash(const std::string &str)
{
	std::string newstr = str;

	for (char &c: newstr) {
		if (c == '\\') {
			c = '/';
		}
	}

	return newstr;
}

void Shared::getLine(std::istream &in, std::string &line)
{
	line.clear();

	std::istream::sentry se(in, true);
	std::streambuf *buffer = in.rdbuf();

	while (true) {
		int c = buffer->sbumpc();
		switch (c) {
			case '\n':
				return;
			case '\r':
				if (buffer->sgetc() == '\n') {
					buffer->sbumpc();
				}
				return;
			case EOF:
				if (line.empty()) {
					in.setstate(std::ios::eofbit);
				}
				return;
			default:
				line += (char)c;
		}
	}
}

bool Shared::eraseContent(std::ofstream &file, const uint64_t &size)
{
	char *content = nullptr;

	if (size > max_size) {
		content = (char *)calloc((size_t)max_size, sizeof(char));
	}
	else {
		content = (char *)calloc((size_t)size, sizeof(char));
	}

	if (content != nullptr) {
		uint64_t times = size / max_size;
		uint64_t rest = size % max_size;

		for (uint64_t i = 0; i < times && !file.fail(); ++i) {
			file.write(content, (size_t)max_size);
		}
		if (rest > 0) {
			file.write(content, (size_t)rest);
		}

		free(content);

		return !file.fail();
	}
	else {
		return false;
	}
}

bool Shared::writeContent(std::ifstream &inFile, std::ofstream &outFile, const uint64_t &size)
{
	char *content = nullptr;

	if (size > max_size) {
		content = (char *)calloc((size_t)max_size, sizeof(char));
	}
	else {
		content = (char *)calloc((size_t)size, sizeof(char));
	}

	if (content != nullptr) {
		uint64_t times = size / max_size;
		uint64_t rest = size % max_size;

		for (uint64_t i = 0; i < times && !outFile.fail(); ++i) {
			inFile.read(content, (size_t)max_size);
			outFile.write(content, (size_t)max_size);
		}
		if (rest > 0) {
			inFile.read(content, (size_t)rest);
			outFile.write(content, (size_t)rest);
		}

		free(content);

		return !outFile.fail();
	}
	else {
		return false;
	}
}

std::string Shared::getStringSize(const uint64_t &size)
{
	double new_size = (double)size;
	uint8_t unit = 0U;
	while (new_size / 1024.0 >= 1.0 && unit < 3U) {
		new_size /= 1024.0;
		++unit;
	}
	std::ostringstream sizeStream;
	sizeStream.precision(2);
	sizeStream << std::fixed << new_size;
	return sizeStream.str() + (unit == 0U ? " B" : (unit == 1U ? " KB" : (unit == 2U ? " MB" : " GB")));
}

#ifdef _WIN32

#include <windows.h>
#include <psapi.h>

size_t Shared::getUsedRam()
{
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PPROCESS_MEMORY_COUNTERS)&pmc, sizeof(pmc));
	return pmc.PrivateUsage;
}

#elif __linux

size_t Shared::getUsedRam()
{
	std::string usedRam;
	std::string line("vmrss:");
	size_t pos = 0; // first char position of line (should be always at 0 but it's best to be sure)

	std::ifstream in("/proc/self/status");
	while (!in.eof()) {
		Shared::getLine(in, usedRam);
		pos = Shared::getLowercaseStr(usedRam).find(line);
		if (pos != std::string::npos) {
			if (Shared::getLowercaseStr(usedRam).find("kb") != std::string::npos)
				return strtoul(usedRam.substr(pos + line.size()).c_str(), nullptr, 10) * 1024; // return size in bytes

			break; // if not kb is unknown and an error
		}
	}

	return 0; // error
}

#else

size_t Shared::getUsedRam()
{
	return 0; // unknown
}

#endif
