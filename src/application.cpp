#include "application.h"
#include "window.h"
#include "raytracer.h"

#ifndef IMGUI_DEBUG
#include "GLFW/glfw3.h"
#endif


Application::Application()
{
    Init();
}

Application::~Application()
{

}

#ifdef IMGUI_DEBUG
void Application::Init()
{
    MWindow = std::make_shared<Window>(WindowProperties());
}

void Application::Run()
{
    MWindow->StartUpdate();
}
#else
void Application::Init()
{
    // for glfw time
    if (!glfwInit())
    {
        return;
    }
    auto rayTracer = new RayTracer();
    rayTracer->Init();
    rayTracer->Run();
}
void Application::Run()
{
}
#endif


