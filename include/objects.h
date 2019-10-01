
//-------------------------------------------------------------------------------
///
/// \file       objects.h
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    5.0
/// \date       August 21, 2019
///
/// \brief Example source for CS 6620 - University of Utah.
///
//-------------------------------------------------------------------------------

#ifndef _OBJECTS_H_INCLUDED_
#define _OBJECTS_H_INCLUDED_

#include "scene.h"
#include "cyTriMesh.h"
#include "string_utils.h"
//#include "bvh.h"

#define MY_BVH

#ifndef MY_BVH
#include "cyBVH.h"
#endif
//-------------------------------------------------------------------------------

class Sphere : public Object
{
public:
    virtual bool IntersectRay( Ray const &ray, HitInfo &hInfo, int hitSide=HIT_FRONT ) const;
    virtual Box GetBoundBox() const { return Box(-1,-1,-1,1,1,1); }
    virtual void ViewportDisplay(const Material *mtl) const;
};

extern Sphere theSphere;

//-------------------------------------------------------------------------------

class Plane : public Object
{
public:
    virtual bool IntersectRay( Ray const &ray, HitInfo &hInfo, int hitSide=HIT_FRONT ) const;
    virtual Box GetBoundBox() const { return Box(-1,-1,0,1,1,0); }
    virtual void ViewportDisplay(const Material *mtl) const;
};

extern Plane thePlane;

//-------------------------------------------------------------------------------
class MeshBVH;
class BVHNode;

class TriObj : public Object, public cyTriMesh
{
public:
    virtual bool IntersectRay( Ray const &ray, HitInfo &hInfo, int hitSide=HIT_FRONT ) const;
    virtual Box GetBoundBox() const { return Box(GetBoundMin(),GetBoundMax()); }
    virtual void ViewportDisplay(const Material *mtl) const;
 
    bool Load(char const *filename);
 
private:
    
    #ifndef MY_BVH
    cyBVHTriMesh bvh;
    #else
    MeshBVH* bvh = nullptr;
    #endif
    
    bool IntersectTriangle( Ray const &ray, HitInfo &hInfo, int hitSide, unsigned int faceID ) const;
    #ifndef MY_BVH
    bool TraceBVHNode( Ray const &ray, HitInfo &hInfo, int hitSide, unsigned int nodeID ) const;
    #else
    bool TraceBVHNode( Ray const &ray, HitInfo &hInfo, int hitSide, BVHNode* node) const;
    #endif
};

//-------------------------------------------------------------------------------

#endif
