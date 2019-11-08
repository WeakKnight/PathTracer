#pragma once

#include <vector>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective

#include "objects.h"


class Face
{
public:
	// v0. v1, v2
	std::vector<unsigned int> indices;
};

class Mesh 
{
public:
	Mesh() 
	{

	} 
	
	void GenerateTangent()
	{
		for (int i = 0; i < faces.size(); i++)
		{
			auto& face = faces[i];
			auto& pos1 = vertices[face.indices[0]];
			auto& pos2 = vertices[face.indices[1]];
			auto& pos3 = vertices[face.indices[2]];

			auto& uv1 = textureCoords[face.indices[0]];
			auto& uv2 = textureCoords[face.indices[1]];
			auto& uv3 = textureCoords[face.indices[2]];

			glm::vec3 edge1 = pos2 - pos1;
			glm::vec3 edge2 = pos3 - pos1;
			glm::vec2 deltaUV1 = uv2 - uv1;
			glm::vec2 deltaUV2 = uv3 - uv1;

			float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

			glm::vec3 tangent1;
			glm::vec3 bitangent1;

			tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
			tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
			tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
			tangent1 = glm::normalize(tangent1);

			bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
			bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
			bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
			bitangent1 = glm::normalize(bitangent1);

			tangents.push_back(tangent1);
			bitangents.push_back(bitangent1);
		}
	}

private:
	friend class MeshBuilder;
	friend class Model;

	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> tangents;
	std::vector<glm::vec3> bitangents;

	std::vector<glm::vec3> textureCoords;

	std::vector<glm::vec3> vertices;

	std::vector<Face> faces;

	Box aabb;
};



