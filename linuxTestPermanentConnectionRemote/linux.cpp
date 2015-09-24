#include <string>
#include <vector>
#include <algorithm>
#include <exception>
#include <iostream>

int Main(std::vector<std::string> args);

int main(int argc, char *argv[])
{
	try
	{
		std::vector<std::string> args;
		args.reserve(argc);
		std::for_each(argv, argv + argc,
			[&args](const char* src) -> void
			{
				std::string str;				
				args.emplace_back(src);
			});
		return Main(std::move(args));
	}
	catch (std::exception& e)
	{
		std::cerr << "Fatal error: " << e.what() << std::endl;
		return 1;
	}
}