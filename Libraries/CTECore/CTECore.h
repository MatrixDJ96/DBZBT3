#ifndef CTECORE_H
#define CTECORE_H

#include <vector>

#include "../Shared/Shared.h"

/* Q(81) R(82) S(83) */
static const uint32_t qrs_matrix[] = {
	0, 0, 81, 0,
	0, 0, 82, 0,
	0, 0, 83, 0
};

struct Texture
{
	uint32_t width, height, gfx_offset, palette_offset;
};

/* Before using this class you must initialize a valid std::ifstream and check manually if it 'is_open()' */
class Texture_File
{
public:
	Texture_File() = delete;
	Texture_File(const std::string& name, std::ifstream& in);
	~Texture_File();

	const std::string& getOutName() const;
	uint32_t getTexturesCount() const;

	void setOutName(const std::string& outName);
	uint8_t Convert() const;

private:
	std::string inName;
	std::string outName;
	std::vector<Texture> list;
};

#endif // CTECORE_H