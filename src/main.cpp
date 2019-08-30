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

//     glfwSetErrorCallback(glfw_error_callback);

//     if (!glfwInit())
//     {
//         return 1;
//     }

//     // Decide GL+GLSL versions
// #if __APPLE__
//     // GL 3.2 + GLSL 150
//     const char* glsl_version = "#version 150";
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
//     glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
//     glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
// #else
//     // GL 3.0 + GLSL 130
//     const char* glsl_version = "#version 130";
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
//     //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
//     //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
// #endif

//     // Create window with graphics context
//     GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
//     if (window == NULL)
//         return 1;
//     glfwMakeContextCurrent(window);
//     glfwSwapInterval(1); // Enable vsync

//     // Initialize OpenGL loader
//     bool err = gladLoadGL() == 0;

//     if (err)
//     {
//         fprintf(stderr, "Failed to initialize OpenGL loader!\n");
//         return 1;
//     }

//     // Setup Dear ImGui context
//     IMGUI_CHECKVERSION();
//     ImGui::CreateContext();
//     ImGuiIO& io = ImGui::GetIO(); (void)io;
//     //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
//     //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

//     // Setup Dear ImGui style
//     ImGui::StyleColorsDark();
//     //ImGui::StyleColorsClassic();

//     // Setup Platform/Renderer bindings
//     ImGui_ImplGlfw_InitForOpenGL(window, true);
//     ImGui_ImplOpenGL3_Init(glsl_version);

//     // Our state
//     bool show_demo_window = true;
//     bool show_another_window = false;
//     ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

//     // Main loop
//     while (!glfwWindowShouldClose(window))
//     {
//         glfwPollEvents();

//         // Start the Dear ImGui frame
//         ImGui_ImplOpenGL3_NewFrame();
//         ImGui_ImplGlfw_NewFrame();
//         ImGui::NewFrame();

//         {
//             static float f = 0.0f;
//             static int counter = 0;

//             ImGui::Begin("Hello, world!");               
//             ImGui::Text("This is some useful text.");
//             ImGui::End();
//         }

//         // Rendering
//         ImGui::Render();
//         int display_w, display_h;
//         glfwGetFramebufferSize(window, &display_w, &display_h);
//         glViewport(0, 0, display_w, display_h);
//         glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
//         glClear(GL_COLOR_BUFFER_BIT);
//         ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

//         glfwSwapBuffers(window);
//     }

//     // Cleanup
//     ImGui_ImplOpenGL3_Shutdown();
//     ImGui_ImplGlfw_Shutdown();
//     ImGui::DestroyContext();

//     glfwDestroyWindow(window);
//     glfwTerminate();
    
    return 0;
}
