#pragma once

#include <vector>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective

#include "objects.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <spdlog/spdlog.h>

#include "utils.h"
#include "hitinfo.h"

class MeshBVHNew;

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
		area = 0.0f;
		cdf.Init();
	} 

	Interaction SampleFace(int faceId)
	{
		Vec2f b = UniformSampleTriangle();

		auto face = faces[faceId];
		auto p0 = vertices[face.indices[0]];
		auto p1 = vertices[face.indices[1]];
		auto p2 = vertices[face.indices[2]];

		Interaction it;
		auto p = b[0] * p0 + b[1] * p1 + (1 - b[0] - b[1]) * p2;
		it.p = Vec3f(p.x, p.y, p.z);
		auto n = glm::normalize(glm::cross(p1 - p0, p2 - p0));
		it.n = Vec3f(n.x, n.y, n.z);

		return it;
	}

	Interaction Sample()
	{
		int faceId = cdf.Sample();
		return SampleFace(faceId);
	}

	float FaceArea(int faceId)
	{
		auto face = faces[faceId];
		auto p0 = vertices[face.indices[0]];
		auto p1 = vertices[face.indices[1]];
		auto p2 = vertices[face.indices[2]];

		return 0.5f * glm::length(glm::cross(p1 - p0, p2 - p0));
	}

	CDF cdf;

	void CalculateAreaAndCDF()
	{
		for (int i = 0; i < faces.size(); i++)
		{
			float thisArea = FaceArea(i);
			cdf.Add(thisArea);
			area += thisArea;
		}
	}

	float area;

	void ProcessAssimpData(aiMesh* mesh)
	{
		// Face
		for (int i = 0; i < mesh->mNumFaces; i++)
		{
			Face face;
			for (int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
			{
				face.indices.push_back(mesh->mFaces[i].mIndices[j]);
			}

			// Push face id
			face.indices.push_back(i);

			faces.push_back(face);
		}
		// Vertices
		for (int i = 0; i < mesh->mNumVertices; i++)
		{
			auto vertex = mesh->mVertices[i];
			vertices.push_back(glm::vec3(vertex.x, vertex.y, vertex.z));
		}
		// Normals
		if (mesh->HasNormals())
		{
			for (int i = 0; i < mesh->mNumVertices; i++)
			{
				auto normal = mesh->mNormals[i];
				normals.push_back(glm::vec3(normal.x, normal.y, normal.z));
			}
		}
		
		//// Tangent And Bitangent
		//if (mesh->HasTangentsAndBitangents())
		//{
		//	for (int i = 0; i < mesh->mNumVertices; i++)
		//	{
		//		auto tangent = mesh->mTangents[i];
		//		auto bitangent = mesh->mBitangents[i];

		//		tangents.push_back(glm::vec3(tangent.x, tangent.y, tangent.z));
		//		bitangents.push_back(glm::vec3(bitangent.x, bitangent.y, bitangent.z));
		//	}
		//}

		// Texture Coords
		if (mesh->HasTextureCoords(0))
		{
			for (int i = 0; i < mesh->mNumVertices; i++)
			{
				auto texCoord = mesh->mTextureCoords[0][i];
				textureCoords.push_back(glm::vec3(texCoord.x, texCoord.y, texCoord.z));
			}
		}

		GenerateTangent();

		aabb = Box(mesh->mAABB.mMin.x, mesh->mAABB.mMin.y, mesh->mAABB.mMin.z, mesh->mAABB.mMax.x, mesh->mAABB.mMax.y, mesh->mAABB.mMax.z);

		BuildBVH();
	}

	void BuildBVH();
	
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

public:
	MeshBVHNew* bvh;

	std::string path;

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



