#include <iostream>
#include "MiniFB.h"
#include <vector>
#include <array>
#include "ThreadPool.h"
#include "spdlog.h"
#include "sinks/stdout_color_sinks.h"

#define WIDTH 1024
#define HEIGHT 768
#define MAX WIDTH * HEIGHT

int main(int, char**) 
{
    spdlog::set_pattern("[thread %t] %v");
    spdlog::set_level(spdlog::level::off);

    unsigned int colorBuffer[WIDTH * HEIGHT];

    Window* window = mfb_open("RayTracer", WIDTH, HEIGHT);
    if (!window)
    {
        return 0;
    }

    int cores = std::thread::hardware_concurrency();
    ThreadPool pool(cores);

    for(int i = 0; i < cores; i++)
    {
        pool.enqueue([=](unsigned int buffer[]) 
        { 
            for(int j = i; j < MAX; j+= cores)
            {
                int y = j/WIDTH;
                int x = j - y * WIDTH;
                
                int r = 255.0f * (float)x/(float)WIDTH;
                int g = 255.0f * (float)y/(float)HEIGHT;
                int b = 0;
                spdlog::debug("Rendering {} and {}", x, y);
                buffer[j] = MFB_RGB(r,g,b);
            }
        }, colorBuffer);
    }

    while(true)
    {
        UpdateState state;
        
        state = mfb_update(window, colorBuffer);

        if (state != STATE_OK)
        {
            window = nullptr;
            break;
        }
    }
    return 0;
}
