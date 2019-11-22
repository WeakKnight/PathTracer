#pragma once

#include "ray.h"
#include "box.h"

#include "cyMatrix.h"
#include "cyVector.h"
#include "constants.h"

class Material;
class LightComponent;

struct HitInfo;
struct HitInfoContext;

class Node;

//-------------------------------------------------------------------------------
// Base class for all object types
class Object
{
public:
	virtual bool IntersectRay(Ray const& ray, HitInfo& hInfo, int hitSide = HIT_FRONT) const = 0;
	virtual bool IntersectRay(RayContext& rayContext, HitInfoContext& hInfoContext, int hitSide = HIT_FRONT) const = 0;
	virtual Box  GetBoundBox() const = 0;
	virtual void ViewportDisplay(const Material* mtl) const {}  // used for OpenGL display
	virtual Vec3f Normal(const Vec3f& p) const
	{
		return Vec3f(0.0f, 0.0f, 1.0f);
	}
	virtual Vec3f Sample() const;
	virtual float Pdf() const
	{
		return 1.0f;
	}

	virtual float Area() const {
		return 0.0f;
	};

	void SetParent(Node* node)
	{
		parent = node;
	}

	Node* parent = nullptr;
};