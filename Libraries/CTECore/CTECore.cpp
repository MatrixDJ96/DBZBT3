#include "CTECore.h"

using namespace Shared;

Texture_File::Texture_File(const std::string& name, std::ifstream& in) : inName(name), outName(name)
{
	/* Assign correct out file name */
	this->outName += ".ini";

	/* Copy in memory all information from file */
	bool offset = false; // false = graphics; true = palette
	uint32_t buffer;
	in.read(reinterpret_cast<char*>(&buffer), 4);
	Texture texture;
	int bytes = 0;
	while (!in.eof())
	{
		/* Check byte correlation with QRS matrix */
		if (buffer == qrs_matrix[bytes] || (bytes == 2 || bytes == 4 || bytes == 5))
		{
			if (offset == false)
				if (bytes == 4)
					texture.width = buffer * 2;
				else if (bytes == 5)
					texture.height = buffer * 2;
			bytes++;
			if (bytes - 1 == 2)
				if (buffer == 0)
					(bytes)--;
		}
		else
		{
			if (bytes == 12)
			{
				if (offset == true)
				{
					texture.palette_offset = uint32_t(in.tellg()) + 12;
					list.push_back(texture);
				}
				else
					texture.gfx_offset = uint32_t(in.tellg()) + 12;
				offset = !offset;
			}
			bytes = 0;
		}
		in.read(reinterpret_cast<char*>(&buffer), 4);
	}

	in.close(); // FILE is now FREE!
}

Texture_File::~Texture_File()
{}

const std::string& Texture_File::getOutName() const
{
	return outName;
}

uint32_t Texture_File::getTexturesCount() const
{
	return uint32_t(list.size());
}

void Texture_File::setOutName(const std::string & outName)
{
	this->outName = outName;
}

uint8_t Texture_File::Convert() const
{
	if (fileExists(outName.c_str()))
	{
		return 1; // File already exists
	}

	std::ofstream out(outName.c_str(), std::ios::out);
	if (!fileExists(outName.c_str()))
	{
		return 2; // Unable to write file
	}

	int size = getTexturesCount();
	Texture texture;

	out << "[items_count]" << std::endl;
	out << "count=" << size << std::endl;
	for (int i = 0; i < size; ++i)
	{
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

	return 0;
}
