#pragma once

#include "model.h"

class MeshBuilder
{
public:
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
		return modelResult;
	}
};