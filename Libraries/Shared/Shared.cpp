#include <cstring>
#include <sstream>

#include "Shared.h"

bool Shared::fileExists(const std::string &file) {
    std::ifstream in(file, std::ios::in);
    if (in.is_open()) {
        in.close();
        return true;
    } else
        return false;
}

bool Shared::createFile(const std::string &file) {
    std::ofstream out(file, std::ios::out);
    if (out.is_open()) {
        out.close();
        return true;
    } else
        return false;
}

void Shared::getLowercaseStr(const char *name, std::string &lowername) {
    if (name != nullptr) {
        std::string newname;

        int size = (int) strlen(name);

        for (int i = 0; i < size; ++i) {
            if (name[i] >= 'A' && name[i] <= 'Z')
                newname += name[i] - 'A' + 'a';
            else
                newname += name[i];
        }

        lowername.clear();
        lowername = newname;
    }
}

std::string Shared::getLowercaseStr(const std::string &name) {
    std::string lowername;
    getLowercaseStr(name.c_str(), lowername);
    return lowername;
}

void Shared::getFilename(const char *file, std::string &name) {
    if (file != nullptr) {
        std::string newname;

        int size = (int) strlen(file);
        int pos = size;

        for (int i = pos; i >= 0; --i) {
            if (file[i] == '\\' || file[i] == '/')
                break;
            pos = i;
        }

        for (int i = pos; i < size; ++i) {
            newname += file[i];
        }

        name.clear();
        name = newname;
    }
}

std::string Shared::getFilename(const std::string &file) {
    std::string name;
    getFilename(file.c_str(), name);
    return name;
}

void Shared::getFilePath(const char *file, std::string &path) {
    if (file != nullptr) {
        std::string newpath;

        int size = (int) strlen(file);
        int pos = size;

        for (int i = pos; i >= 0; --i) {
            if (file[i] == '\\' || file[i] == '/')
                break;
            pos = i;
        }

        for (int i = 0; i < pos; ++i) {
            newpath += file[i];
        }

        path.clear();
        path = newpath;
    }
}

std::string Shared::getFilePath(const std::string &file) {
    std::string path;
    getFilePath(file.c_str(), path);
    return path;
}

void Shared::getFileBaseName(const char *file, std::string &basename, const char *ext) {
    if (file != nullptr) {
        int extSize = ext != nullptr ? (int) strlen(ext) : 0;

        std::string name;
        getFilename(file, name);

        int nameSize = int(name.length()) - 1;

        int namePoints = 0;
        for (int i = 0; i <= nameSize; ++i) {
            if (name[i] == '.')
                namePoints++;
        }
        if (namePoints == 0 || (namePoints == 1 && name[0] == '.')) {
            basename = file;
            return;
        }

        int namePos = nameSize;
        if (extSize != 0) {
            int extPoints = 1;
            for (int j = 0; j < extSize; ++j) {
                if (ext[j] == '.' && j != 0)
                    extPoints++;
            }

            if (namePoints >= extPoints) {
                std::string lowername = name;
                getLowercaseStr(lowername.c_str(), lowername);

                for (int i = namePos; i > 0; --i) {
                    if (lowername[i] == '.') {
                        extPoints--;
                        if (extPoints == 0) {
                            std::string lowerext = ext;
                            getLowercaseStr(lowerext.c_str(), lowerext);
                            int pos = i + (extSize > 1 ? ext[0] == '.' ? 0 : 1 : 1);
                            bool found = true;

                            for (int j = 0; j < extSize; ++j)
                                if (lowername[pos + j] != lowerext[j]) {
                                    found = false;
                                    break;
                                }

                            if (found)
                                namePos = i;

                            break;
                        }
                    }
                }
            }
        } else {
            for (int i = namePos; i > 0; --i) {
                if (name[i] == '.') {
                    namePos = i;
                    break;
                }
            }
        }

        std::string newbasename;
        if (namePos != nameSize)
            for (int i = 0; i < namePos; ++i)
                newbasename += name[i];

        if (newbasename.empty())
            newbasename = file;
        else {
            std::string path;
            getFilePath(file, path);
            newbasename = path + newbasename;
        }

        basename.clear();
        basename = newbasename;
    }
}

std::string Shared::getFileBaseName(const std::string &file, const char *ext) {
    std::string basename;
    getFileBaseName(file.c_str(), basename, ext);
    return basename;
}

void Shared::backSlashtoSlash(const char *instr, std::string &outstr) {
    if (instr != nullptr) {
        outstr.clear();

        int size = int(strlen(instr));
        for (int i = 0; i < size; i++) {
            if (instr[i] == '\\')
                outstr += '/';
            else
                outstr += instr[i];
        }
    }
}

std::string Shared::backSlashtoSlash(const std::string &str) {
    std::string newstr;
    backSlashtoSlash(str.c_str(), newstr);
    return newstr;
}

void Shared::getLine(std::ifstream &in, std::string &line) {
    line.clear();

    std::istream::sentry se(in, true);
    std::streambuf *buffer = in.rdbuf();

    while (true) {
        int c = buffer->sbumpc();
        switch (c) {
            case '\n':
                return;
            case '\r':
                if (buffer->sgetc() == '\n')
                    buffer->sbumpc();
                return;
            case EOF:
                if (line.empty())
                    in.setstate(std::ios::eofbit);
                return;
            default:
                line += (char) c;
        }
    }
}

bool Shared::eraseContent(std::fstream &file, const uint32_t &size) {
    uint32_t max_size = 1048576 * 256; // allocate max 256mb in RAM (to avoid problem with bad_alloc)

    char *content = nullptr;
    if (size > max_size) {
        content = new char[max_size];
        for (uint64_t i = 0U; i < max_size; ++i) {
            content[i] = '\0';
        }
    } else {
        content = new char[size];
        for (uint64_t i = 0U; i < size; ++i) {
            content[i] = '\0';
        }
    }

    uint32_t times = size / max_size;
    uint32_t rest = size % max_size;

    for (uint32_t i = 0U; i < times; ++i) {
        file.write(content, max_size);
    }
    if (rest > 0U) {
        file.write(content, rest);
    }

    delPointerArray(content);

    return !file.bad();
}

std::string Shared::getStringSize(const uint32_t &size) {
    double new_size = (double) size;
    uint8_t unit = 0U;
    while (new_size / 1024.0 >= 1.0 && unit < 3) {
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

uint32_t Shared::getUsedRam() {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PPROCESS_MEMORY_COUNTERS) & pmc, sizeof(pmc));
    return pmc.PrivateUsage;
}

#elif __linux

uint32_t Shared::getUsedRam() {
    std::string usedRam;
    std::string line("vmrss:");
    size_t pos = 0; // first char position of line (should be always at 0 but it's best to be sure)

    std::ifstream in("/proc/self/status");
    while (!in.eof()) {
        Shared::getLine(in, usedRam);
        pos = Shared::getLowercaseStr(usedRam).find(line);
        if (pos != std::string::npos) {
            if (Shared::getLowercaseStr(usedRam).find("kb") != std::string::npos)
                return strtoul(usedRam.substr(pos + line.size()).c_str(), NULL, 10) * 1024; // return size in bytes

            break; // if not kb is unknown and an error
        }
    }

    return 0; // error
}

#else

uint32_t Shared::getUsedRam() {
    return 0; // unknown
}

#endif
