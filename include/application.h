#pragma once

#include <memory>

namespace RayTracing
{
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
}