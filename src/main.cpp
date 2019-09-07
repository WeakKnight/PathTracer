#include <iostream>
#include <vector>
#include <memory>

#include "spdlog/spdlog.h"

#include "application.h"

int main(int, char**) 
{
    spdlog::set_pattern("[thread %t] %v");
    spdlog::set_level(spdlog::level::debug);

    auto app = std::make_shared<Application>();
    app->Run();
    
    return 0;
}
