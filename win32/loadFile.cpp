#include "../src/utf8.h"

#include <string>
#include <fstream>

std::string loadFile(const std::string& filename)
{
	std::ifstream ifs;

	{
		std::wstring wfilename;
		wfilename.reserve(filename.size());
		utf8::utf8to16(filename.begin(), filename.end(), std::back_inserter(wfilename));
		ifs.open(wfilename, std::ios_base::in);
	}

	return std::string(
		std::istreambuf_iterator<char>(ifs),
		std::istreambuf_iterator<char>()
		);
}