#include "scene.h"

bool Box::IntersectRay(Ray const &r, float t_max) const{
        assert(pmax.x - pmin.x >= 0.0f);
        assert(pmax.y - pmin.y >= 0.0f);
        assert(pmax.z - pmin.z >= 0.0f);
        
		float tmin, tmax, tymin, tymax, tzmin, tzmax;

        Vec3f invertDir =  Vec3f(1.0f, 1.0f, 1.0f) / r.dir;
		int sign0 = (invertDir.x < 0);
		int sign1 = (invertDir.y < 0);
		int sign2 = (invertDir.z < 0);

		auto starThis = *this;

		tmin = (starThis[sign0].x - r.p.x) * invertDir.x;

		tmax = (starThis[1 - sign0].x - r.p.x) * invertDir.x;
		tymin = (starThis[sign1].y - r.p.y) * invertDir.y;
		tymax = (starThis[1 - sign1].y - r.p.y) * invertDir.y;

		if ((tmin > tymax) || (tymin > tmax))
			return false;
		if (tymin > tmin)
			tmin = tymin;
		if (tymax < tmax)
			tmax = tymax;

		tzmin = (starThis[sign2].z - r.p.z) * invertDir.z;
		tzmax = (starThis[1 - sign2].z - r.p.z) * invertDir.z;

		if ((tmin > tzmax) || (tzmin > tmax))
			return false;
		if (tzmin > tmin)
			tmin = tzmin;
		if (tzmax < tmax)
			tmax = tzmax;

		return true;

     /*   float tx0;
        float tx1;
        
        if(invertDir.x >= 0.0f)
        {
            tx0 = (pmin.x - r.p.x) * invertDir.x;
            tx1 = (pmax.x - r.p.x) * invertDir.x;
        }
        else
        {
            tx1 = (pmin.x - r.p.x) * invertDir.x;
            tx0 = (pmax.x - r.p.x) * invertDir.x;
        }
        
        assert(tx0 <= tx1);
        
        float ty0;
        float ty1;
        
        if(invertDir.y >= 0.0f)
        {
            ty0 = (pmin.y - r.p.y) * invertDir.y;
            ty1 = (pmax.y - r.p.y) * invertDir.y;
        }
        else
        {
            ty1 = (pmin.y - r.p.y) * invertDir.y;
            ty0 = (pmax.y - r.p.y) * invertDir.y;
        }
        
        assert(ty0 <= ty1);
        
        if(ty1 < tx0)
        {
            return false;
        }
        
        if(tx1 < ty0)
        {
            return false;
        }
        
        float t0 = Max(tx0, ty0);
        float t1 = Min(tx1, ty1);
        
        float tz0;
        float tz1;
        if(invertDir.z >= 0.0f)
        {
            tz0 = (pmin.z - r.p.z) * invertDir.z;
            tz1 = (pmax.z - r.p.z) * invertDir.z;
        }
        else
        {
            tz1 = (pmin.z - r.p.z) * invertDir.z;
            tz0 = (pmax.z - r.p.z) * invertDir.z;
        }
        
        if(t1 < tz0)
        {
            return false;
        }
        
        if(tz1 < t0)
        {
            return false;
        }
        
        t0 = Max(t0, tz0);
        t1 = Min(t1, tz1);
        
        assert(t0 <= t1);
        
        if(t0 < 0.0f)
        {
            if(t1 < 0.0f)
            {
                return false;
            }
            else
            {
                if(t1 > t_max)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
        }
        else
        {
            if(t0 > t_max)
            {
                return false;
            }
            else
            {
                return true;
            }
        }
        
        return false;*/
    }
