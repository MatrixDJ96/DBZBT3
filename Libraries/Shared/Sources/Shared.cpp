#include <cstring>
#include <sstream>

#include <Shared.h>

#ifdef _WIN32

#include <windows.h>
#include <direct.h>
#include <psapi.h>

#elif __linux

#include <unistd.h>

#endif

static constexpr uint64_t max_size = 1048576 * 64; // 1 * x MB

uint64_t Shared::getFileSize(const std::string &path)
{
	std::ifstream inFile(path, std::ios::in | std::ios::binary);
	if (inFile.is_open()) {
		inFile.seekg(0, std::ios::end);
		auto size = inFile.tellg();
		inFile.close();
		return size;
	}
	else {
		return 0;
	}
}

std::string Shared::getLowercase(const std::string &str)
{
	std::string lowername = str;

	for (char &c: lowername) {
		if (c >= 'A' && c <= 'Z') {
			c -= ('A' - 'a');
		}
	}

	return lowername;
}

std::string Shared::getFullPath(const std::string &path)
{
	char str[512];
	getcwd(str, 512);

	std::string file_basename = Shared::getFileBasename(path);
	std::string curr_path = Shared::backSlashtoSlash(str) + '/';
	std::string new_path = Shared::backSlashtoSlash(Shared::getDirname(path));

	auto found = new_path.find("/./");
	while (found != std::string::npos) {
		new_path.erase(found, 2);
		found = new_path.find("/./");
	}

	if (new_path.substr(0, 2) == "./") {
		new_path.erase(0, 2);
	}

#ifdef _WIN32
	if (new_path[1] != ':') {
#else
	if (new_path[0] != '/') {
#endif
		new_path = curr_path + new_path;

		found = new_path.find("/../");
		while (found != std::string::npos) {
			auto new_found = new_path.substr(0, found).rfind('/');
			new_path = new_path.substr(0, new_found) + new_path.substr(found + 3);
			found = new_path.find("/../");
		}
	}

	return new_path + file_basename;
}

std::string Shared::getFileBasename(const std::string &path)
{
	std::string basename;

	std::size_t found = path.find_last_of("/\\");
	if (found != std::string::npos) {
		basename = path.substr(found + 1);
	}
	else {
		basename = path;
	}

	return basename;
}

std::string Shared::getFilename(const std::string &path)
{
	std::string filename = getFileBasename(path);

	std::size_t found = filename.find_last_of('.');
	if (found != std::string::npos) {
		filename = filename.substr(0, found);
	}

	return filename;
}

std::string Shared::getDirname(const std::string &path)
{
	std::string dirname;

	std::size_t found = path.find_last_of("/\\");
	if (found != std::string::npos) {
		dirname = path.substr(0, found + 1);
	}

	return dirname;
}

std::string Shared::backSlashtoSlash(const std::string &path)
{
	std::string newpath = path;

	for (char &c: newpath) {
		if (c == '\\') {
			c = '/';
		}
	}

	return newpath;
}

void Shared::getLine(std::istream &in, std::string &line, int max_size)
{
	line.clear();

	std::istream::sentry se(in, true);
	std::streambuf *buffer = in.rdbuf();

	while (true) {
		int c = buffer->sbumpc();
		switch (c) {
			case '\r':
				if (buffer->sgetc() == '\n') {
					buffer->sbumpc();
				}
			case '\n':
				if (buffer->sgetc() == EOF) {
					in.setstate(std::ios::eofbit);
				}
				return;
			case EOF:
				in.setstate(std::ios::eofbit);
				return;
			default:
				if (max_size == 0 || (max_size != 0 && line.size() != max_size)) {
					line += (char)c;
				}
		}
	}
}

bool Shared::eraseContent(std::ofstream &outFile, uint64_t pos, uint64_t size)
{
	char *content = nullptr;
	return eraseContent(outFile, pos, size, content);
}

bool Shared::eraseContent(std::ofstream &outFile, uint64_t pos, uint64_t size, char *&content)
{
	if (content != nullptr) {
		free(content);
	}

	if (size > max_size) {
		content = (char *)calloc((size_t)max_size, sizeof(char));
	}
	else {
		content = (char *)calloc((size_t)size, sizeof(char));
	}

	if (content != nullptr) {
		outFile.seekp(pos, std::ios::beg);

		uint64_t times = size / max_size;
		uint64_t rest = size % max_size;

		for (uint64_t i = 0; i < times && !outFile.fail(); ++i) {
			outFile.write(content, (size_t)max_size);
		}
		if (rest > 0 && !outFile.fail()) {
			outFile.write(content, (size_t)rest);
		}

		free(content);
		content = nullptr;

		return !outFile.fail();
	}
	else {
		return false;
	}
}

bool Shared::writeContent(std::ifstream &inFile, uint64_t inPos, std::ofstream &outFile, uint64_t outPos, uint64_t size)
{
	char *content = nullptr;
	return writeContent(inFile, inPos, outFile, outPos, size, content);
}

bool Shared::writeContent(std::ifstream &inFile, uint64_t inPos, std::ofstream &outFile, uint64_t outPos, uint64_t size, char *&content)
{
	if (content != nullptr) {
		free(content);
	}

	if (size > max_size) {
		content = (char *)calloc((size_t)max_size, sizeof(char));
	}
	else {
		content = (char *)calloc((size_t)size, sizeof(char));
	}

	if (content != nullptr) {
		uint64_t times = size / max_size;
		uint64_t rest = size % max_size;

		for (uint64_t i = 0; i < times && !inFile.fail() && !outFile.fail() && !inFile.eof(); ++i) {
			inFile.seekg(inPos + i * max_size, std::ios::beg);
			inFile.read(content, (size_t)max_size);

			outFile.seekp(outPos + i * max_size, std::ios::beg);
			outFile.write(content, (size_t)max_size);
		}
		if (rest > 0 && !inFile.fail() && !outFile.fail() && !inFile.eof()) {
			inFile.seekg(inPos + times * max_size, std::ios::beg);
			inFile.read(content, (size_t)rest);

			outFile.seekp(outPos + times * max_size, std::ios::beg);
			outFile.write(content, (size_t)rest);
		}

		free(content);
		content = nullptr;

		return !outFile.fail();
	}
	else {
		return false;
	}
}

std::string Shared::getSize(double dsize)
{
	int unit = 0;
	for (; dsize / 1024 > 1 && unit < 3; ++unit) {
		dsize /= 1024;
	}

	std::ostringstream ossize;
	ossize.precision(2);
	ossize << std::fixed << dsize;

	return (ossize.str() + (unit == 0 ? " B" : (unit == 1 ? " KB" : (unit == 2 ? " MB" : " GB"))));
}

double Shared::getSize(const std::string &ssize)
{
	std::string::size_type pos;
	double size = std::stod(ssize, &pos);

	if (pos + 1 < ssize.size()) {
		auto substr = ssize.substr(pos + 1);
		switch (substr[0]) {
			case 'G':
				size *= 1024;
			case 'M':
				size *= 1024;
			case 'K':
				size *= 1024;
			default:
				break;
		}
	}

	return size;
}

bool Shared::truncateFile(const std::string &path, long size)
{
#ifdef _WIN32

	auto outFile = CreateFile(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (outFile != INVALID_HANDLE_VALUE) {
		if (SetFilePointer(outFile, size, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER) {
			return SetEndOfFile(outFile) && CloseHandle(outFile);
		}
		else {
			CloseHandle(outFile);
		}
	}

#elif __linux

	return truncate(path.c_str(), size) == 0;

#endif

	return false;
}

size_t Shared::getUsedRam()
{
	size_t total = 0;

#ifdef _WIN32

	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PPROCESS_MEMORY_COUNTERS)&pmc, sizeof(pmc));
	total = pmc.PrivateUsage;

#elif __linux

	std::ifstream in("/proc/self/status");

	if (in.is_open()) {
		size_t pos = 0;
		std::string usedRam;
		std::string line("vmrss:");

		while (!in.eof()) {
			Shared::getLine(in, usedRam);
			pos = Shared::getLowercase(usedRam).find(line);
			if (pos != std::string::npos) {
				if (Shared::getLowercase(usedRam).find("kb") != std::string::npos) {
					total = strtoul(usedRam.substr(pos + line.size()).c_str(), nullptr, 10) * 1024; // return size in bytes
				}
				break; // if not kb is unknown and an error
			}
		}
	}

#endif

	return total;
}
