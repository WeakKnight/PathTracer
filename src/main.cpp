#include <iostream>
#include <vector>
#include <memory>

#include "spdlog/spdlog.h"

#include "application.h"

#ifdef _WIN32
#include <direct.h>
#include "string_utils.h"
#endif
int main(int, char** args) 
{

#ifdef _WIN32
	auto path = args[0];
	auto tokens = StringUtils::Split(path, "\\");
	std::string environmentPath = "";

	for (int i = 0; i < tokens.size() - 1; i++)
	{
		environmentPath += (tokens[i] + "\\");
	}

	chdir(environmentPath.c_str());
#endif

    spdlog::set_pattern("[thread %t] %v");
    spdlog::set_level(spdlog::level::debug);

    auto app = std::make_shared<Application>();
    app->Run();
    
    return 0;
}
