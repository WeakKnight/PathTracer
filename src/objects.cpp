#include "objects.h"
#include <vector>

namespace RayTracing {
extern Camera camera;


bool Sphere::IntersectRay(Ray const &ray, HitInfo &hInfo, TraceContext* context, int hitSide) const
{ 
    // |ray.p + ray.dir * l| = 1
    // ray.dir.LengthSquared() * l * l + 2 * ray.p.Dot(ray.dir) * l + ray.p.LengthSquared() - 1 = 0
    float a = ray.dir.LengthSquared();
    float b = 2 * ray.p.Dot(ray.dir);
    float c = ray.p.LengthSquared() - 1;
    
    float delta = b*b - 4 * a * c;
    
    if(delta < 0)
    {
        return false;
    }
    else
    {
        if(delta == 0)
        {
            float dist = -1.0f * b / (2.0f * a);
            
            cyVec3f intersectPoint = ray.p + ray.dir * dist;
            cyVec3f intersectPointWorld = RayTracing::localToWorld(intersectPoint, context);
            
            cyVec3f rayPositionWorld = RayTracing::localToWorld(ray.p, context);
            
            float zDepth = camera.dir.Dot(intersectPointWorld - rayPositionWorld);

            // float zDepth = camera.dir.Dot(dist * ray.dir);

            // if hit in front of the eye, it should be transparent, just like lights go through
            if(zDepth < 0.0f)
            {
                return false;
            }
            else
            {
                if(hitSide == HIT_FRONT || hitSide == HIT_FRONT_AND_BACK)
                {
                    if(zDepth < hInfo.z)
                    {
                        hInfo.z = zDepth;
                        // behind the image plane and only one root, it is in the tangent plane, must be front
                        hInfo.front = true;
                    }
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
        // Delta > 0, double roots
        else
        {
            float dist1 = (-1.0f * b - sqrtf(delta))/(2.0f * a);
            float dist2 = (-1.0f * b + sqrtf(delta))/(2.0f * a);
         
            cyVec3f intersectPoint1 = ray.p + ray.dir * dist1;
            cyVec3f intersectPointWorld1 = RayTracing::localToWorld(intersectPoint1, context);
            
            cyVec3f rayPositionWorld1 = RayTracing::localToWorld(ray.p, context);
            
            float zDepth1 = camera.dir.Dot(intersectPointWorld1 - rayPositionWorld1);
            
            cyVec3f intersectPoint2 = ray.p + ray.dir * dist2;
            cyVec3f intersectPointWorld2 = RayTracing::localToWorld(intersectPoint2, context);
            
            cyVec3f rayPositionWorld2 = RayTracing::localToWorld(ray.p, context);
            
            float zDepth2 = camera.dir.Dot(intersectPointWorld2 - rayPositionWorld2);
            
//            float zDepth1 = camera.dir.Dot(dist1 * ray.dir);
//            float zDepth2 = camera.dir.Dot(dist2 * ray.dir);

            if(zDepth1 < 0.0f)
            {
                if(zDepth2 < 0.0f)
                {
                    return false;
                }
                else
                {
                    if(hitSide == HIT_BACK || hitSide == HIT_FRONT_AND_BACK)
                    {
                        if(zDepth2 < hInfo.z)
                        {
                            hInfo.z = zDepth2;
                            hInfo.front = false;
                        }

                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
            else
            {
                if(hitSide == HIT_FRONT || hitSide == HIT_FRONT_AND_BACK)
                {
                    if(zDepth1 < hInfo.z)
                    {
                        hInfo.z = zDepth1;
                        hInfo.front = true;
                    }

                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        return true;
    }
}

void Sphere::ViewportDisplay() const
{

}
    
}
