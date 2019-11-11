#include "window.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "spdlog/spdlog.h"
//#include "scene.h"

#include "texture2d.h"
//#include "renderimagehelper.h"

//#include "xmlload.h"

//#include "objects.h"

//#include <stack>

#include "raytracer.h"
#include "utils.h"
#include "samplereditor.h"

#include <thread>

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void TestOrdinalSpeed()
{
	std::vector<Vec3f> testCase;

	for (int i = 0; i <= 10000; i++)
	{
		testCase.push_back(RandomInUnitSphere());
	}

	auto t1 = glfwGetTime();
	for (int i = 0; i <= 10000; i++)
	{
		Vec3f b1;
		Vec3f b2;
		CommonOrthonormalBasis(testCase[i], b1, b2);
	}
	auto t2 = glfwGetTime();
	for (int i = 0; i <= 10000; i++)
	{
		Vec3f b1;
		Vec3f b2;
		BranchlessONB(testCase[i], b1, b2);
	}
	auto t3 = glfwGetTime();

	spdlog::debug("common is {} branchless is {}", t2-t1, t3-t2);
}

Window::Window(const WindowProperties& InProperties)
{
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit())
    {
        return;
    }

	//TestOrdinalSpeed();

    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    MGLFWWindow = glfwCreateWindow(1280, 720, "Ray Tracer", NULL, NULL);
    if (MGLFWWindow == NULL)
    {
        spdlog::debug("window is null, error");
        return;
    }
    glfwMakeContextCurrent(MGLFWWindow);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
    bool err = gladLoadGL() == 0;

    if (err)
    {
        spdlog::debug("Failed to initialize OpenGL loader!");
        return;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(MGLFWWindow, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

Window::~Window()
{
    glfwDestroyWindow(MGLFWWindow);
    glfwTerminate();
}

int xInputMin;
int xInputMax;
int yInputMin;
int yInputMax;

float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

float updateTimer = 0.0f;

RayTracer rayTracer;

void RayTracing()
{
	rayTracer.Run();
}

void Window::StartUpdate()
{
    
    rayTracer.Init();

	std::thread tracingThread(RayTracing);

    auto renderTexture = rayTracer.GetRenderTexture();
    auto zbufferTexture = rayTracer.GetZBufferTexture();
    auto normalTexture = rayTracer.GetNormalTexture();
    auto sampleTexture = rayTracer.GetSampleTexture();
    auto filterTexture = rayTracer.GetFilterTexture();
    
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    
    while (!glfwWindowShouldClose(MGLFWWindow))
    {
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		updateTimer += deltaTime;

        glfwPollEvents();

		if(updateTimer > 0.3f)
        {
			updateTimer = 0.0f;
            rayTracer.UpdateRenderResult();
        }
        
//            spdlog::debug("current time is {}", glfwGetTime());
        
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

		OnSamplerEditor();
		{
			
		}

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open"))
				{
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

        {
            ImGui::Begin("Toolbox");
            ImGui::SetWindowSize(ImVec2(240.0f, 400.0f));

            if(ImGui::Button("output buffer file"))
            {
                rayTracer.WriteToFile();
            }
            
            if(ImGui::Button("Run Tracing"))
            {
                rayTracer.Run();
            }
            
            ImGui::InputText("SceneName", rayTracer.scene_path, 256);
            if(ImGui::Button("Reinit Scene"))
            {
                rayTracer.Init();
            }
            //rayTracer.scene_path = ImGui::inputt
            
            ImGui::End();
        }
        
        {
            ImGui::Begin("ViewPort");
            ImGui::SetWindowSize(ImVec2(renderTexture->Width + 20, renderTexture->Height + 60));
			if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
			{
				if (ImGui::BeginTabItem("Result"))
				{
					ImGui::Image
					(
						(ImTextureID)renderTexture->Id,
						ImVec2(renderTexture->Width, renderTexture->Height),
						ImVec2(0, 0),
						ImVec2(1, 1),
						ImVec4(1.0, 1.0, 1.0, 1.0),
						ImVec4(1.0, 1.0, 1.0, 1.0)
					);

					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Depth"))
				{
					ImGui::Image
					(
						(ImTextureID)zbufferTexture->Id,
						ImVec2(zbufferTexture->Width, zbufferTexture->Height),
						ImVec2(0, 0),
						ImVec2(1, 1),
						ImVec4(1.0, 1.0, 1.0, 1.0),
						ImVec4(1.0, 1.0, 1.0, 1.0)
					);
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Sample Count"))
				{
					ImGui::Image
					(
						(ImTextureID)sampleTexture->Id,
						ImVec2(sampleTexture->Width,
							sampleTexture->Height),
						ImVec2(0, 0),
						ImVec2(1, 1),
						ImVec4(1.0, 1.0, 1.0, 1.0),
						ImVec4(1.0, 1.0, 1.0, 1.0)
					);
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Normal"))
				{
					ImGui::Image
					(
						(ImTextureID)normalTexture->Id,
						ImVec2(normalTexture->Width, normalTexture->Height),
						ImVec2(0, 0),
						ImVec2(1, 1),
						ImVec4(1.0, 1.0, 1.0, 1.0),
						ImVec4(1.0, 1.0, 1.0, 1.0)
					);
					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}
           
            ImGui::End();
        }
        
        // Rendering
        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(MGLFWWindow, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(MGLFWWindow);
    }
}
