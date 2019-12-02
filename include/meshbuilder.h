#pragma once

#include "model.h"
#include <string>
#include "string_utils.h"
#include <vector>
#include "spdlog/spdlog.h"

class MeshBuilder
{
public:

	static Model* BuildTextModel(const std::string& path)
	{
		auto content = StringUtils::ReadFile(path);
		if (content.size() == 0)
		{
			content = StringUtils::ReadFile("assets/" + path);
		}

		Mesh* result = new Mesh();

		std::istringstream f(content);
		std::string currentLine;

		result->aabb.Init();

		int type = 0;

		// 0 indices
		// 1 points
		// 2 normal
		// 3 uv

		while (std::getline(f, currentLine))
		{
			spdlog::debug("current line is {}", currentLine);
			StringUtils::trim(currentLine);

			if (currentLine.rfind("#", 0) == 0)
			{
				spdlog::debug("Comments Line");
			}
			else if (currentLine.size() == 0)
			{
				spdlog::debug("Empty Line");
			}
			else if (currentLine[0] == '\r')
			{
				spdlog::debug("Empty Line");
			}
			else
			{
				std::vector<std::string> tokens;
				std::string res = "";

				for (size_t i = 0; i < currentLine.size(); i++)
				{
					if (currentLine[i] != ' ')
					{
						res += currentLine[i];
					}
					else
					{
						if (res.size() != 0)
						{
							tokens.push_back(res);
						}
						res = "";
					}
				}

				if (res.size() != 0)
				{
					tokens.push_back(res);
				}

				// indices
				if (type == 0)
				{
					std::vector<int> indiceContainer;

					for (int i = 0; i < tokens.size(); i++)
					{
						if (tokens[i].size() > 0
							&&
							tokens[i][tokens[i].size() - 1] != '\r')
						{
							int index = std::stoi(tokens[i]);

							indiceContainer.push_back(index);
							if (indiceContainer.size() == 3)
							{
								Face face;
								face.indices.push_back(indiceContainer[0]);
								face.indices.push_back(indiceContainer[1]);
								face.indices.push_back(indiceContainer[2]);
								// face id
								face.indices.push_back(result->faces.size());

								result->faces.push_back(face);

								indiceContainer.clear();
							}
						}
					}

					type = 1;
				}
				// vertices
				else if (type == 1)
				{
					std::vector<float> comContainer;

					for (int i = 0; i < tokens.size(); i++)
					{
						if (tokens[i].size() > 0
							&&
							tokens[i][tokens[i].size() - 1] != '\r')
						{
							float com = std::stof(tokens[i]);

							comContainer.push_back(com);
							if (comContainer.size() == 3)
							{
								result->vertices.push_back(glm::vec3(comContainer[0], comContainer[1], comContainer[2]));
								result->aabb += Vec3f(comContainer[0], comContainer[1], comContainer[2]);

								comContainer.clear();
							}
						}
					}

					type = 2;
				}
				// normal
				else if (type == 2)
				{
					std::vector<float> comContainer;

					for (int i = 0; i < tokens.size(); i++)
					{
						if (tokens[i].size() > 0
							&&
							tokens[i][tokens[i].size() - 1] != '\r')
						{
							float com = std::stof(tokens[i]);

							comContainer.push_back(com);
							if (comContainer.size() == 3)
							{
								result->normals.push_back(glm::vec3(comContainer[0], comContainer[1], comContainer[2]));
								comContainer.clear();
							}
						}
					}

					type = 3;
				}
				// uv
				else if (type == 3)
				{
					std::vector<float> comContainer;

					for (int i = 0; i < tokens.size(); i++)
					{
						if (tokens[i].size() > 0
							&&
							tokens[i][tokens[i].size() - 1] != '\r')
						{
							float com = std::stof(tokens[i]);

							comContainer.push_back(com);
							if (comContainer.size() == 2)
							{
								result->textureCoords.push_back(glm::vec3(comContainer[0], comContainer[1], 0.0f));
								comContainer.clear();
							}
						}
					}
				}
			}
		}

		result->GenerateTangent();
		result->path = path;
		result->BuildBVH();

		Model* modelResult = new Model(result, 1);
		modelResult->BuildCDFAndArea();

		return modelResult;
		
	}

	static Model* BuildUnitPlane()
	{
		Mesh* result = new Mesh();
		glm::vec3 v0(-1.0f, -1.0f, 0.0f);
		glm::vec3 v1(1.0f, -1.0f, 0.0f);
		glm::vec3 v2(1.0f, 1.0f, 0.0f);
		glm::vec3 v3(-1.0f, 1.0f, 0.0f);

		glm::vec3 normal(0.0f, 0.0f, 1.0f);

		result->vertices.push_back(v0);
		result->normals.push_back(normal);
		result->textureCoords.push_back(glm::vec3(0.0f, 0.0f, 0.0f));

		result->vertices.push_back(v1);
		result->normals.push_back(normal);
		result->textureCoords.push_back(glm::vec3(1.0f, 0.0f, 0.0f));

		result->vertices.push_back(v2);
		result->normals.push_back(normal);
		result->textureCoords.push_back(glm::vec3(1.0f, 1.0f, 0.0f));

		result->vertices.push_back(v3);
		result->normals.push_back(normal);
		result->textureCoords.push_back(glm::vec3(0.0f, 1.0f, 0.0f));

		// counter-clockwise
		Face face0;
		face0.indices.push_back(1);
		face0.indices.push_back(3);
		face0.indices.push_back(0);

		// face id
		face0.indices.push_back(0);

		Face face1;
		face1.indices.push_back(2);
		face1.indices.push_back(3);
		face1.indices.push_back(1);

		// face id
		face1.indices.push_back(1);

		result->faces.push_back(face0);
		result->faces.push_back(face1);

		result->GenerateTangent();

		result->aabb = Box(-1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f + FLT_EPSILON);

		Model* modelResult = new Model(result, 1);
		modelResult->BuildCDFAndArea();

		return modelResult;
	}
};