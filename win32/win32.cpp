#include "../src/utf8.h"

#include <string>
#include <vector>
#include <algorithm>
#include <exception>
#include <iostream>

int Main(std::vector<std::string> args);

int wmain(int argc, wchar_t *argv[])
{
	try
	{
		std::vector<std::string> args;
		args.reserve(argc);
		std::for_each(argv, argv + argc,
			[&args](const wchar_t* src) -> void
			{
				size_t len = wcslen(src);
				const wchar_t* end = src + len;
				std::string str;
				utf8::utf16to8(src, end, std::back_inserter(str));
				args.emplace_back(std::move(str));
			});
		return Main(std::move(args));
	}
	catch (std::exception& e)
	{
		std::cerr << "Fatal error: " << e.what() << std::endl;
		return 1;
	}
}