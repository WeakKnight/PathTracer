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
	std::vector<unsigned int> indices;
};

class Mesh 
{
public:
	Mesh() 
	{

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

class MeshBuilder 
{
	Model BuildUnitPlane() 
	{
		Mesh* result = new Mesh();
		glm::vec3 v0(-1.0f, -1.0f, 0.0f);
		glm::vec3 v1(1.0f, -1.0f, 0.0f);
		glm::vec3 v2(-1.0f, 1.0f, 0.0f);
		glm::vec3 v3(1.0f, 1.0f, 0.0f);

		glm::vec3 normal(0.0f, 0.0f, 1.0f);

		result->vertices.push_back(v0);
		result->normals.push_back(normal);
		result->textureCoords.push_back(glm::vec3(0.0f, 0.0f, 0.0f));

		result->vertices.push_back(v1);
		result->normals.push_back(normal);
		result->textureCoords.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
		
		result->vertices.push_back(v2);
		result->normals.push_back(normal);
		result->textureCoords.push_back(glm::vec3(0.0f, 1.0f, 0.0f));

		result->vertices.push_back(v3);
		result->normals.push_back(normal);
		result->textureCoords.push_back(glm::vec3(1.0f, 1.0f, 0.0f));
		
		// face 0
		result->tangents.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
		result->bitangents.push_back(glm::vec3(0.0f, 1.0f, 0.0f));

		// face 1
		result->tangents.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
		result->bitangents.push_back(glm::vec3(0.0f, 1.0f, 0.0f));

		// counter-clockwise
		Face face0;
		face0.indices.push_back(0);
		face0.indices.push_back(3);
		face0.indices.push_back(1);

		Face face1;
		face1.indices.push_back(1);
		face1.indices.push_back(3);
		face1.indices.push_back(2);

		result->faces.push_back(face0);
		result->faces.push_back(face1);

		result->aabb = Box(-1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f + FLT_EPSILON);

		Model modelResult = Model(result, 1);
		return modelResult;
	}
};

class Model: public Object
{
public:
	Model(Mesh* meshes, unsigned int meshesNum)
		:meshes(meshes),
		meshesNum(meshesNum)
	{
		float minx = INFINITY;
		float miny = INFINITY;
		float minz = INFINITY;
		float maxx = -INFINITY;
		float maxy = -INFINITY;
		float maxz = -INFINITY;

		for (int i = 0; i < meshesNum; i++)
		{
			auto mesh = meshes[i];

			if (mesh.aabb.pmin.x < minx)
			{
				minx = mesh.aabb.pmin.x;
			}
			if (mesh.aabb.pmin.y < miny)
			{
				miny = mesh.aabb.pmin.y;
			}
			if (mesh.aabb.pmin.z < minz)
			{
				minz = mesh.aabb.pmin.z;
			}

			if (mesh.aabb.pmax.x > maxx)
			{
				maxx = mesh.aabb.pmax.x;
			}
			if (mesh.aabb.pmax.y > maxy)
			{
				maxy = mesh.aabb.pmax.y;
			}
			if (mesh.aabb.pmax.z > maxz)
			{
				maxz = mesh.aabb.pmax.z;
			}
		}

		aabb = Box(minx, miny, minz, maxx, maxy, maxz);
	}

	~Model()
	{
		if (meshes)
		{
			delete meshes;
		}
	}

	virtual bool IntersectRay(Ray const& ray, HitInfo& hInfo, int hitSide = HIT_FRONT) const
	{
		if (!GetBoundBox().IntersectRay(ray, BIGFLOAT))
		{
			return false;
		}

		bool result = false;
		for (int i = 0; i < meshesNum; i++)
		{
			auto mesh = meshes[i];
			for (int j = 0; j < mesh.faces.size(); j++)
			{
				auto face = mesh.faces[j];
				HitInfo currentHitInfo;
				if (IntersectRayWithFace(ray, currentHitInfo, hitSide, mesh, face))
				{
					if (currentHitInfo.z < hInfo.z)
					{
						result = true;
						hInfo.Copy(currentHitInfo);
					}
				}
			}
		}

		return result;
	}

	virtual bool IntersectRay(RayContext& rayContext, HitInfoContext& hInfoContext, int hitSide = HIT_FRONT) const
	{
		if (!GetBoundBox().IntersectRay(rayContext.cameraRay, BIGFLOAT))
		{
			return false;
		}

		for (int i = 0; i < meshesNum; i++)
		{
			auto mesh = meshes[i];
			for (int j = 0; j < mesh.faces.size(); j++)
			{
				auto face = mesh.faces[j];

			}

		}
		return true;
	}

	virtual Box  GetBoundBox() const
	{
		
		return aabb;
	}

	bool IntersectRayWithFace(Ray const& ray, HitInfo& hInfo, int hitSide, Mesh mesh, Face face) const
	{
		const glm::vec3& v0 = mesh.vertices[face.indices[0]];
		const glm::vec3& v1 = mesh.vertices[face.indices[1]];
		const glm::vec3& v2 = mesh.vertices[face.indices[2]];

		glm::vec3 v01 = v1 - v0;
		glm::vec3 v02 = v2 - v0;

		// unnormalized
		glm::vec3 n = glm::cross(v01, v02);

		glm::vec3 rayDir = glm::vec3(ray.dir.x, ray.dir.y, ray.dir.z);
		glm::vec3 rayPos = glm::vec3(ray.p.x, ray.p.y, ray.p.z);

		// (ray.origin + ray.dir * t - v0).dot(n) = 0
		float dirDotN = glm::dot(rayDir, n);

		if (abs(dirDotN) <= FLT_EPSILON)
		{
			return false;
		}

		float originDotN = glm::dot(rayPos, n);
		float v0DotN = glm::dot(v0, n);
		float t = (v0DotN - originDotN) / dirDotN;

		if (t < 0.0f)
		{
			return false;
		}

		bool isFront = (dirDotN < 0.0f);

		if (hitSide == HIT_FRONT && !isFront)
		{
			return false;
		}

		if (hitSide == HIT_BACK && isFront)
		{
			return false;
		}

		glm::vec3 p = rayPos + rayDir * t;

		glm::mat3x3 mm(-rayDir, v01, v02);
		mm = glm::inverse(mm);

		glm::vec3 vv0 = mm * v0;
		glm::vec3 vv1 = mm * v1;
		glm::vec3 vv2 = mm * v2;
		glm::vec3 pp = mm * p;

		Vec2f v0_2d = Vec2f(vv0.y, vv0.z);
		Vec2f v1_2d = Vec2f(vv1.y, vv1.z);
		Vec2f v2_2d = Vec2f(vv2.y, vv2.z);
		Vec2f p_2d = Vec2f(pp.y, pp.z);

		Vec2f pv0 = v0_2d - p_2d;
		Vec2f pv1 = v1_2d - p_2d;
		Vec2f pv2 = v2_2d - p_2d;

		float two_a0 = pv1.Cross(pv2);
		if (two_a0 <= 0.0f)
		{
			return false;
		}
		float two_a1 = pv2.Cross(pv0);
		if (two_a1 <= 0.0f)
		{
			return false;
		}
		float two_a2 = pv0.Cross(pv1);
		if (two_a2 <= 0.0f)
		{
			return false;
		}

		float two_a = (v1_2d - v0_2d).Cross(v2_2d - v0_2d);

		float beta0 = two_a0 / two_a;
		float beta1 = two_a1 / two_a;
		float beta2 = two_a2 / two_a;
		
		const glm::vec3& n0 = mesh.normals[face.indices[0]];
		const glm::vec3& n1 = mesh.normals[face.indices[1]];
		const glm::vec3& n2 = mesh.normals[face.indices[2]];

		glm::vec3 normal = glm::normalize(beta0 * n0 + beta1 * n1 + beta2 * n2);

		const glm::vec3& t0 = mesh.textureCoords[face.indices[0]];
		const glm::vec3& t1 = mesh.textureCoords[face.indices[1]];
		const glm::vec3& t2 = mesh.textureCoords[face.indices[2]];

		glm::vec3 tex = beta0 * t0 + beta1 * t1 + beta2 * t2;

		hInfo.p = Vec3f(p.x, p.y, p.z);
		hInfo.z = t;
		hInfo.N = isFront ? Vec3f(normal.x, normal.y, normal.z) : -1.0f * Vec3f(normal.x, normal.y, normal.z);
		hInfo.front = isFront;
		hInfo.uvw = Vec3f(tex.x, tex.y, tex.z);

		return true;
	}

private:
	Box aabb;
	Mesh* meshes;
	unsigned int meshesNum;
};