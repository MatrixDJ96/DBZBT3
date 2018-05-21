#ifndef SHARED_H
#define SHARED_H

#include <iostream>
#include <fstream>
#include <string>

/* This is a byte with value at 0 */
static const char nullbyte = char(0x00);

/* Function to safe-delete pointer */
template<typename T>
void delPointer(T *&t) {
    if (t != NULL) {
        delete t;
        t = NULL;
    }
}

/* Function to safe-delete pointer array */
template<typename T>
void delPointerArray(T *&t) {
    if (t != NULL) {
        delete[] t;
        t = NULL;
    }
}

namespace Shared {
    /* Check if file exists */
    bool fileExists(const std::string &file);

    /* Create file and return result (bool) */
    bool createFile(const std::string &file);

    /* Function to tranform uppercase chars into lowercase chars */
    void getLowercaseStr(const char *name, std::string &lowername);

    /* Function to tranform uppercase chars into lowercase chars */
    std::string getLowercaseStr(const std::string &name);

    /* Function to get filename without path */
    void getFilename(const char *file, std::string &name);

    /* Function to get filename without path */
    std::string getFilename(const std::string &file);

    /* Function to get path from filename (if std::string(file) doesn't contain path it will return path = "") */
    void getFilePath(const char *file, std::string &path);

    /* Function to get path from filename (if std::string(file) doesn't contain path it will return path = "") */
    std::string getFilePath(const std::string &file);

    /* Function to get filename without (custom) extension */
    void getFileBaseName(const char *file, std::string &basename, const char *ext = 0);

    /* Function to get filename without (custom) extension */
    std::string getFileBaseName(const std::string &file, const char *ext = 0);

    /* This function will change '\\' to '/' in a std::string */
    void backSlashtoSlash(const char *instr, std::string &outstr);

    /* This function will change '\\' to '/' in a std::string */
    std::string backSlashtoSlash(const std::string &str);

    /* A better why to get a line (thanks stackoverflow.com) */
    void getLine(std::ifstream &in, std::string &line);

    /* Function to write zeroes in a file */
    bool eraseContent(std::fstream &file, const uint32_t &size);

    /* Function to get string (in KB,MB or GB) from size */
    std::string getStringSize(const uint32_t &size);

    /* Return used RAM from current process (if return 0 is an error) */
    uint32_t getUsedRam();
}

#endif // SHARED_H
