#pragma once
#include "cyMatrix.h"
#include "cyVector.h"
#include "constants.h"
#include "ray.h"

//-------------------------------------------------------------------------------
using namespace cy;

class Box
{
public:
	Vec3f pmin, pmax;

	Vec3f& operator[](int index)
	{
		if (index == 0)
		{
			return pmin;
		}
		else
		{
			return pmax;
		}
	}

	// Constructors
	Box() { Init(); }
	Box(Vec3f const& _pmin, Vec3f const& _pmax) : pmin(_pmin), pmax(_pmax) {}
	Box(float xmin, float ymin, float zmin, float xmax, float ymax, float zmax) : pmin(xmin, ymin, zmin), pmax(xmax, ymax, zmax) {}
	Box(float const* dim) : pmin(dim[0], dim[1], dim[2]), pmax(dim[3], dim[4], dim[5]) {}

	// Initializes the box, such that there exists no point inside the box (i.e. it is empty).
	void Init() { pmin.Set(BIGFLOAT, BIGFLOAT, BIGFLOAT); pmax.Set(-BIGFLOAT, -BIGFLOAT, -BIGFLOAT); }

	// Returns true if the box is empty; otherwise, returns false.
	bool IsEmpty() const { return pmin.x > pmax.x || pmin.y > pmax.y || pmin.z > pmax.z; }

	// Returns one of the 8 corner point of the box in the following order:
	// 0:(x_min,y_min,z_min), 1:(x_max,y_min,z_min)
	// 2:(x_min,y_max,z_min), 3:(x_max,y_max,z_min)
	// 4:(x_min,y_min,z_max), 5:(x_max,y_min,z_max)
	// 6:(x_min,y_max,z_max), 7:(x_max,y_max,z_max)
	Vec3f Corner(int i) const // 8 corners of the box
	{
		Vec3f p;
		p.x = (i & 1) ? pmax.x : pmin.x;
		p.y = (i & 2) ? pmax.y : pmin.y;
		p.z = (i & 4) ? pmax.z : pmin.z;
		return p;
	}

	// Enlarges the box such that it includes the given point p.
	void operator += (Vec3f const& p)
	{
		for (int i = 0; i < 3; i++) {
			if (pmin[i] > p[i]) pmin[i] = p[i];
			if (pmax[i] < p[i]) pmax[i] = p[i];
		}
	}

	// Enlarges the box such that it includes the given box b.
	void operator += (const Box& b)
	{
		for (int i = 0; i < 3; i++) {
			if (pmin[i] > b.pmin[i]) pmin[i] = b.pmin[i];
			if (pmax[i] < b.pmax[i]) pmax[i] = b.pmax[i];
		}
	}

	// Returns true if the point is inside the box; otherwise, returns false.
	bool IsInside(Vec3f const& p) const { for (int i = 0; i < 3; i++) if (pmin[i] > p[i] || pmax[i] < p[i]) return false; return true; }

	// Returns true if the ray intersects with the box for any parameter that is smaller than t_max; otherwise, returns false.
	bool IntersectRay(Ray const& r, float t_max) const;
};