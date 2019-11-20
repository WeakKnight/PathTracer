#pragma once

#include "cyMatrix.h"
#include "cyVector.h"
#include "constants.h"

using namespace cy;

class Node;
class Material;

struct HitInfo
{
	float       z;      // the distance from the ray center to the hit point
	Vec3f       p;      // position of the hit point
	Vec3f       N;      // surface normal at the hit point
	Vec3f	    Tangent;
	Vec3f       Bitangent;
	Vec3f       uvw;    // texture coordinate at the hit point
	Vec3f       duvw[2];// derivatives of the texture coordinate
	Node* node;   // the object node that was hit
	bool        front;  // true if the ray hits the front side, false if the ray hits the back side
	int         mtlID;  // sub-material index
	Material* mtl;

	HitInfo() { Init(); }
	void Init() { z = BIGFLOAT; node = nullptr; front = true; uvw.Set(0.5f, 0.5f, 0.5f); duvw[0].Zero(); duvw[1].Zero(); mtlID = 0; mtl = nullptr; }

	void Copy(const HitInfo& other)
	{
		z = other.z;
		p = other.p;
		N = other.N;
		Tangent = other.Tangent;
		Bitangent = other.Bitangent;
		uvw = other.uvw;
		duvw[0] = other.duvw[0];
		duvw[1] = other.duvw[1];
		node = other.node;
		front = other.front;
		mtlID = other.mtlID;
		mtl = other.mtl;
	}

	void CopyForDiffRay(const HitInfo& other)
	{
		z = other.z;
		p = other.p;
		N = other.N;
	}
};

struct HitInfoContext
{
	HitInfoContext()
	{
		Init();
	}

	void Init()
	{
		mainHitInfo.Init();
		rightHitInfo.Init();
		topHitInfo.Init();
	}

	HitInfo mainHitInfo;
	HitInfo rightHitInfo;
	HitInfo topHitInfo;

	bool screenRay = false;
	int screenX = -1;
	int screenY = -1;

	void SetAsScreenInfo(int x, int y)
	{
		screenRay = true;
		screenX = x;
		screenY = y;
	}
};