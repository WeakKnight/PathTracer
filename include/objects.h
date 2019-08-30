//-------------------------------------------------------------------------------
///
/// \file       objects.h 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    1.0
/// \date       August 21, 2019
///
/// \brief Example source for CS 6620 - University of Utah.
///
//-------------------------------------------------------------------------------
 
#ifndef _OBJECTS_H_INCLUDED_
#define _OBJECTS_H_INCLUDED_
 
#include "scene.h"
#include "raytracer.h"

namespace RayTracing {
    //-------------------------------------------------------------------------------
    
    class Sphere : public Object
    {
    public:
        virtual bool IntersectRay( Ray const &ray, HitInfo &hInfo, int hitSide=HIT_FRONT) const;
        
        virtual void ViewportDisplay() const;
    };
    
    extern Sphere theSphere;
    
    //-------------------------------------------------------------------------------
}

 
#endif
