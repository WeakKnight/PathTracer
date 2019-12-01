#pragma once

#include <string>

struct GLFWwindow;

struct WindowProperties
{
    std::string Title;
    unsigned int Width;
    unsigned int Height;

    WindowProperties(const std::string &InTitle = "Ray Tracing",
                    unsigned int InWidth = 1900,
                    unsigned int InHeight = 1000) : Title(InTitle), Width(InWidth), Height(InHeight)
    {
    }
};

class Window
{
    public:
        Window(const WindowProperties& InProperties);
        ~Window();

        void StartUpdate();
        inline unsigned int GetWidth() const { return MWidth; }
        inline unsigned int GetHeight() const { return MHeight; }

    private:
        GLFWwindow* MGLFWWindow;
        std::string MTitle;
        unsigned int MWidth, MHeight;
        // bool MVSync;
        // unsigned int MFrameRate = 60;
};
