#pragma once
#include "utils.h"
#include "imgui.h"
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective

std::vector<ImVec2> circleSamplers;

std::vector<glm::vec3> verticalGrids;
std::vector<glm::vec3> horizontalGrids;

void RenderWiredFrameSphere()
{
	auto radius = 90.0f;
	auto drawList = ImGui::GetWindowDrawList();
	auto windowPos = ImGui::GetWindowPos();
	auto center = ImVec2(windowPos.x + 200.0f, windowPos.y + 200.0f);

	// sphere center world pos = 0.0, 0.0, 0.0, so it has a model matrix with zero translation
	static glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	static int verticalGrid = 10;
	static int horizontalGrid = 10;

	// x2 + y2 + z2 = r * r
	// z from center - 90 to center + 90, than decide x and y use a polor basis

}

void OnSamplerEditor()
{
	ImGui::SetNextWindowSize(ImVec2(300.0f, 320.0f));
	ImGui::Begin("Sampler Test");
	auto drawList = ImGui::GetWindowDrawList();
	auto windowPos = ImGui::GetWindowPos();
	auto center = ImVec2(windowPos.x + 200.0f, windowPos.y + 200.0f);
	auto radius = 90.0f;

	if (ImGui::BeginTabBar("##Sampler Tabs", ImGuiTabBarFlags_None))
	{
		if (ImGui::BeginTabItem("Circle"))
		{
			static int addingSamplerCount = 1;
			static bool uniform = true;

			ImGui::InputInt("Count", &addingSamplerCount);
			ImGui::LabelText("Total Count", "%d", circleSamplers.size());
			ImGui::Checkbox("Uniform Random", &uniform);

			if (ImGui::Button("Add Sampler"))
			{
				for (int i = 0; i < addingSamplerCount; i++)
				{
					auto randomPoint = RandomPointInCircle(radius);
					if (!uniform)
					{
						randomPoint = NonUniformRandomPointInCircle(radius);
					}
					circleSamplers.push_back(ImVec2(randomPoint.x, randomPoint.y));
				}
			}

			if (ImGui::Button("Clear"))
			{
				circleSamplers.clear();
			}
			drawList->AddCircleFilled(center, radius, IM_COL32(70, 255, 255, 255), 100);

			for (int i = 0; i < circleSamplers.size(); i++)
			{
				drawList->AddCircleFilled(ImVec2(center.x + circleSamplers[i].x, center.y + circleSamplers[i].y), 3.0f, IM_COL32(10, 10, 10, 255), 12);
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("HemiSphere"))
		{

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Sphere"))
		{

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}