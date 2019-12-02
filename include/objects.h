
//-------------------------------------------------------------------------------
///
/// \file       objects.h
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    7.0
/// \date       October 6, 2015
///
/// \brief Example source for CS 6620 - University of Utah.
///
//-------------------------------------------------------------------------------

#pragma once

#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include <vector>
#include <atomic>
#include <mutex>

#include "scene.h"
#include "cyTriMesh.h"
#include "string_utils.h"
#include "constants.h"
#include "ray.h"
#include "box.h"
#include "objbase.h"

using namespace cy;

struct HitInfo;
struct HitInfoContext;
class Material;

class Sphere : public Object
{
public:
    virtual bool IntersectRay( const Ray &ray, HitInfo &hInfo, int hitSide=HIT_FRONT ) const;
    virtual Box GetBoundBox() const { return Box(-1,-1,-1,1,1,1); }
    virtual void ViewportDisplay(const Material *mtl) const;
    virtual bool IntersectRay(RayContext &rayContext, HitInfoContext& hInfoContext, int hitSide = HIT_FRONT) const;
};

//-------------------------------------------------------------------------------

class Plane : public Object
{
public:
    virtual bool IntersectRay( const Ray &ray, HitInfo &hInfo, int hitSide=HIT_FRONT ) const;
    virtual Box GetBoundBox() const { return Box(-1,-1,0,1,1,0); }
    virtual void ViewportDisplay(const Material *mtl) const;
    virtual bool IntersectRay(RayContext &rayContext, HitInfoContext& hInfoContext, int hitSide = HIT_FRONT) const;
	virtual Interaction Sample() const;
	virtual float Area() const;
	virtual Vec3f Normal(const Vec3f& p) const;
};
//-------------------------------------------------------------------------------
class MeshBVH;
class BVHNode;

class TriObj : public Object, public cyTriMesh
{
public:
    virtual bool IntersectRay( const Ray &ray, HitInfo &hInfo, int hitSide=HIT_FRONT ) const;
    virtual Box GetBoundBox() const { return Box(GetBoundMin(),GetBoundMax()); }
    virtual void ViewportDisplay(const Material *mtl) const;
    virtual bool IntersectRay( RayContext &rayContext, HitInfoContext& hInfoContext, int hitSide = HIT_FRONT) const;
    
    bool Load(const char *filename, bool loadMtl);
    
private:
    MeshBVH* bvh = nullptr;
    bool IntersectTriangle( const Ray &ray, HitInfo &hInfo, int hitSide, unsigned int faceID ) const;
    bool TraceBVHNode( Ray const &ray, HitInfo &hInfo, int hitSide, BVHNode* node) const;
    
    bool IntersectTriangle( RayContext &rayContext, HitInfoContext& hInfoContext, int hitSide, unsigned int faceID ) const;
    bool TraceBVHNode( RayContext &rayContext, HitInfoContext& hInfoContext, int hitSide, BVHNode* node) const;
};

