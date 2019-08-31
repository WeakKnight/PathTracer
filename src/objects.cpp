#include "objects.h"
#include <vector>

extern Camera camera;

bool Sphere::IntersectRay(Ray const &ray, HitInfo &hInfo, int hitSide) const
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
            Vec3f intersectPoint = ray.p + ray.dir * dist;
            
            // if hit in front of the eye, it should be transparent, just like lights go through
            if(dist < 0.0f)
            {
                return false;
            }
            else
            {
                if(hitSide == HIT_FRONT || hitSide == HIT_FRONT_AND_BACK)
                {
                    if(dist < hInfo.z)
                    {
                        hInfo.z = dist;
                        hInfo.N = intersectPoint; // obj space
//                        hInfo.N.Normalize();
                        hInfo.p = intersectPoint; // obj space
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
            
            Vec3f intersectPoint1 = ray.p + ray.dir * dist1;
            Vec3f intersectPoint2 = ray.p + ray.dir * dist2;
            
            if(dist1 < 0.0f)
            {
                if(dist2 < 0.0f)
                {
                    return false;
                }
                else
                {
                    if(hitSide == HIT_BACK || hitSide == HIT_FRONT_AND_BACK)
                    {
                        if(dist2 < hInfo.z)
                        {
                            hInfo.z = dist2;
                            hInfo.N = intersectPoint2;
//                            hInfo.N.Normalize();
                            hInfo.p = intersectPoint2;
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
                    if(dist1 < hInfo.z)
                    {
                        hInfo.z = dist1;
                        hInfo.N = intersectPoint1;
//                        hInfo.N.Normalize();
                        hInfo.p = intersectPoint1;
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

void Sphere::ViewportDisplay(const Material *mtl) const
{
}
    
