#ifndef CTECORE_H
#define CTECORE_H

#include <vector>

#include "../Shared/Shared.h"

/* Q(81) R(82) S(83) */
const uint32_t qrs_matrix[] = {
        0, 0, 81, 0,
        0, 0, 82, 0,
        0, 0, 83, 0
};

struct Texture {
    Texture() : width(0), height(0), gfx_offset(0), palette_offset(0) {}

    uint32_t width, height, gfx_offset, palette_offset;
};

class Texture_File {
public:
    Texture_File(const std::string &name);

    ~Texture_File();

    const std::string &getOutName() const;

    uint32_t getTexturesCount() const;

    void setOutName(const std::string &outName);

    bool Convert() const;

private:
    std::string inName;
    std::string outName;
    std::vector<Texture> list;
};

#endif // CTECORE_H