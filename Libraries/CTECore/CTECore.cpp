#include "CTECore.h"

using namespace Shared;

Texture_File::Texture_File(const std::string &name) : inName(name), outName(name)
{
	std::ifstream inFile;

	inFile.open(this->inName, std::ios::in | std::ios::binary);

	/* Assign correct out file name */
	this->outName += ".ini";

	/* Copy in memory all information from file */
	bool offset = false; // false = graphics; true = palette
	uint32_t buffer;
	inFile.read(reinterpret_cast<char *>(&buffer), 4);
	Texture texture;
	int bytes = 0;
	while (!inFile.eof()) {
		/* Check byte correlation with QRS matrix */
		if (buffer == qrs_matrix[bytes] || (bytes == 2 || bytes == 4 || bytes == 5)) {
			if (!offset) {
				if (bytes == 4) {
					texture.width = buffer * 2;
				}
				else if (bytes == 5) {
					texture.height = buffer * 2;
				}
			}
			bytes++;
			if (bytes - 1 == 2) {
				if (buffer == 0) {
					(bytes)--;
				}
			}
		}
		else {
			if (bytes == 12) {
				if (offset) {
					texture.palette_offset = uint32_t(inFile.tellg()) + 12;
					list.push_back(texture);
				}
				else {
					texture.gfx_offset = uint32_t(inFile.tellg()) + 12;
				}
				offset = !offset;
			}
			bytes = 0;
		}
		inFile.read(reinterpret_cast<char *>(&buffer), 4);
	}

	inFile.close(); // FILE is now FREE!
}

Texture_File::~Texture_File() = default;

const std::string &Texture_File::getOutName() const
{
	return outName;
}

size_t Texture_File::getTexturesCount() const
{
	return list.size();
}

void Texture_File::setOutName(const std::string &outName)
{
	this->outName = outName;
}

bool Texture_File::Convert() const
{
	if (!createFile(outName)) {
		return false;
	} // unable to write file

	std::ofstream out(outName.c_str(), std::ios::out);

	size_t size = getTexturesCount();
	Texture texture;

	out << "[items_count]" << std::endl;
	out << "count=" << size << std::endl;
	for (size_t i = 0; i < size; ++i) {
		texture = list[i];
		out << "[item_" << i << "]" << std::endl;
		out << "name=texture_" << i + 1 << std::endl;
		out << "platform=PS2" << std::endl;
		out << "offset=" << texture.gfx_offset << std::endl;
		out << "width=" << texture.width << std::endl;
		out << "height=" << texture.height << std::endl;
		out << "BPP=8" << std::endl;
		out << "mipmaps=-1" << std::endl;
		out << "palette_offset=" << texture.palette_offset << std::endl;
		out << "swizzling=Enabled" << std::endl;
	}
	out.close();

	return true;
}
