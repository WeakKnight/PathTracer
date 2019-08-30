#pragma once

#include <memory>

class Window;

class Application
{
    public:
        Application();
        ~Application();
    
        void Run();
    private:
        void Init();
    private:
        std::shared_ptr<Window> MWindow;
};

