#include "application.h"
#include "window.h"

namespace RayTracing
{
    Application::Application()
    {
        Init();
    }

    Application::~Application()
    {

    }

    void Application::Init()
    {
        MWindow = std::make_shared<Window>(WindowProperties());
    }

    void Application::Run()
    {
        MWindow->StartUpdate();
    }
}