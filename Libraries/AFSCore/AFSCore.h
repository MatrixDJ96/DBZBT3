#ifndef AFSCORE_H
#define AFSCORE_H

#include "../AFLCore/AFLCore.h"

struct FileInfo {
    FileInfo() : address(0), size(0) {}

    uint32_t address, size;
};

struct FileDesc {
    FileDesc() : name(""), year(0), month(0), day(0), hour(0), minute(0), second(0), size(0) {}

    char name[33]; // name[32] equals to '\0'
    uint16_t year, month, day, hour, minute, second;
    uint32_t size;
};

/* Default AFS header */
const uint32_t afsHeader = 0x00534641;

struct AFS_Error {
    AFS_Error() : unableToOpen(false), notAFS(false), infoSize(false), descSize(false), afsSize(false), fileDesc(false),
                  blankNames(false)/*, voidZone(false)*/ // TODO -> to implement
    {}

    bool unableToOpen, notAFS, infoSize, descSize, afsSize, fileDesc, blankNames/*, voidZone*/; // TODO -> to implement
};

class AFS_File {
public:
    AFS_File(const std::string &afsName);

    ~AFS_File();

    const AFS_Error &getError() const;

    const uint32_t &getFileCount() const;

    const std::vector<FileInfo> &getFileInfo() const;

    const std::vector<FileDesc> &getFileDesc() const;

    const uint32_t &getAFSSize() const;

    void changeFileName(const int &index, const std::string &name);

    bool exportFile(const int &index, const std::string &path) const; // TODO -> check if file already exists
    bool exportCommon(const std::string &path) const; // TODO -> check if file already exists
    bool exportRAW(const std::string &path) const; // TODO -> remove, only for testing
    uint8_t importFile(const int &index, const std::string &path);

    bool importCommon(const std::string &path);

    bool importRAW(const std::string &path); // TODO -> remove, only for testing

    bool commitFileInfo() const;

    bool commitFileDesc() const;

private:
    bool getFile(const int &index, char *&file) const;

    bool openAFS(std::fstream &afs, const std::ios::openmode &mode) const;

    void loadFileInfo(std::fstream &inFile);

    void loadFileDesc(std::fstream &inFile);

public:
    const std::string afsName;

private:
    uint32_t afsSize;
    AFS_Error error;
    uint32_t fileCount;
    std::vector<FileInfo> fileInfo;
    std::vector<FileDesc> fileDesc;
};

#endif // AFSCORE_H
