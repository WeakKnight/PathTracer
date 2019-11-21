#include "objects.h"
#include <vector>
#include "bvh.h"
#include "GLFW/glfw3.h"
#include "float.h"
#include <math.h>

extern float buildTime;
extern Camera camera;

bool TriObj::TraceBVHNode( Ray const &ray, HitInfo &hInfo, int hitSide, BVHNode* node) const
{
    if(node->IsLeaf())
    {
        bool result = false;
        for(unsigned i = 0; i < node->faceList.size(); i++)
        {
            unsigned faceId = node->faceList[i];
            HitInfo currentHitInfo;
            if(IntersectTriangle(ray, currentHitInfo, hitSide, faceId))
            {
                if(currentHitInfo.z < hInfo.z)
                {
                    result = true;
                    
                    hInfo.Copy(currentHitInfo);
                }
            }
        }
        return result;
    }
    else
    {
        if(node->bound.IntersectRay(ray))
        {
            HitInfo leftHitInfo;
            HitInfo rightHitInfo;
            bool hitLeft = TraceBVHNode(ray, leftHitInfo, hitSide, node->left);
            bool hitRight = TraceBVHNode(ray, rightHitInfo, hitSide, node->right);
            
            if (hitLeft && hitRight) {
                if (leftHitInfo.z < rightHitInfo.z)
                {
                    hInfo.Copy(leftHitInfo);
                }
                else
                {
                    hInfo.Copy(rightHitInfo);
                }
                return true;
            }
            else if (hitLeft)
            {
                hInfo.Copy(leftHitInfo);
                return true;
            }
            else if (hitRight)
            {
                hInfo.Copy(rightHitInfo);
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
}

bool TriObj::TraceBVHNode( RayContext &rayContext, HitInfoContext& hInfoContext, int hitSide, BVHNode* node) const
{
    HitInfo& hInfo = hInfoContext.mainHitInfo;
    HitInfo& hInfoRight = hInfoContext.rightHitInfo;
    HitInfo& hInfoTop =hInfoContext.topHitInfo;
    
    if(node->IsLeaf())
    {
        bool result = false;
        for(unsigned i = 0; i < node->faceList.size(); i++)
        {
            unsigned faceId = node->faceList[i];
            
            HitInfoContext currentHitInfoContext;
            HitInfo& currentHitInfo = currentHitInfoContext.mainHitInfo;
            
            if(IntersectTriangle(rayContext, currentHitInfoContext, hitSide, faceId))
            {
                if(currentHitInfo.z < hInfo.z)
                {
                    result = true;
                    
                    hInfo.Copy(currentHitInfo);
                    
                    hInfoRight.CopyForDiffRay(currentHitInfoContext.rightHitInfo);
                    hInfoTop.CopyForDiffRay(currentHitInfoContext.topHitInfo);
                }
            }
        }
        return result;
    }
    else
    {
        const auto ray = rayContext.cameraRay;
        
        if(node->bound.IntersectRay(ray))
        {
            HitInfoContext leftHitInfoContext;
            HitInfo& leftHitInfo = leftHitInfoContext.mainHitInfo;
            HitInfo& left_RightHitInfo = leftHitInfoContext.rightHitInfo;
            HitInfo& left_TopHitInfo = leftHitInfoContext.topHitInfo;
            
            HitInfoContext rightHitInfoContext;
            HitInfo& rightHitInfo = rightHitInfoContext.mainHitInfo;
            HitInfo& right_RightHitInfo = rightHitInfoContext.rightHitInfo;
            HitInfo& right_TopHitInfo = rightHitInfoContext.topHitInfo;
            
            bool hitLeft = TraceBVHNode(rayContext, leftHitInfoContext, hitSide, node->left);
            bool hitRight = TraceBVHNode(rayContext, rightHitInfoContext, hitSide, node->right);
            
            if (hitLeft && hitRight) {
                if (leftHitInfo.z < rightHitInfo.z)
                {
                    hInfo.Copy(leftHitInfo);

                    hInfoRight.CopyForDiffRay(left_RightHitInfo);
                    hInfoTop.CopyForDiffRay(left_TopHitInfo);
                }
                else
                {
                    hInfo.Copy(rightHitInfo);

                    hInfoRight.CopyForDiffRay(right_RightHitInfo);
                    hInfoTop.CopyForDiffRay(right_TopHitInfo);
                }
                return true;
            }
            else if (hitLeft)
            {
                hInfo.Copy(leftHitInfo);

                hInfoRight.CopyForDiffRay(left_RightHitInfo);
                hInfoTop.CopyForDiffRay(left_TopHitInfo);
                
                return true;
            }
            else if (hitRight)
            {
                hInfo.Copy(rightHitInfo);

                hInfoRight.CopyForDiffRay(right_RightHitInfo);
                hInfoTop.CopyForDiffRay(right_TopHitInfo);
                
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
}


bool TriObj::IntersectRay(const Ray &ray, HitInfo &hInfo, int hitSide) const
{
    if(!GetBoundBox().IntersectRay(ray, BIGFLOAT))
    {
        return false;
    }

    return TraceBVHNode(ray, hInfo, hitSide, bvh->GetRoot());
}

bool TriObj::IntersectRay(RayContext &rayContext, HitInfoContext &hInfoContext, int hitSide) const
{
    if(!GetBoundBox().IntersectRay(rayContext.cameraRay, BIGFLOAT))
    {
        return false;
    }
    
    return TraceBVHNode(rayContext, hInfoContext, hitSide, bvh->GetRoot());
}

bool TriObj::Load(char const *filename, bool loadMtl)
{
    if(! LoadFromFileObj(filename, loadMtl))
    {
        if(! LoadFromFileObj( StringUtils::Format("assets/%s", filename).c_str() , loadMtl))
        {
            return false;
        }
    }
    if ( ! HasNormals() ) ComputeNormals();
    ComputeBoundingBox();
    
    if(bvh != nullptr)
    {
        delete bvh;
    }
    float now = glfwGetTime();
    bvh = new MeshBVH(this);
    float then = glfwGetTime();
    buildTime += (then - now);
   
    return true;
}

bool TriObj::IntersectTriangle( RayContext &rayContext, HitInfoContext &hInfoContext, int hitSide, unsigned int faceID) const
{
    HitInfo& hInfo = hInfoContext.mainHitInfo;
    
    if(IntersectTriangle(rayContext.cameraRay, hInfo, hitSide, faceID))
    {
        if(rayContext.hasDiff)
        {
            const TriFace& face = F(faceID);
            
            const Vec3f& v0 = V(face.v[0]);
            const Vec3f& v1 = V(face.v[1]);
            const Vec3f& v2 = V(face.v[2]);
            
            Vec3f v01 = v1 - v0;
            Vec3f v02 = v2 - v0;
            
            const Ray& rightRay = rayContext.rightRay;
            const Ray& topRay = rayContext.topRay;
            
            HitInfo& rightInfo = hInfoContext.rightHitInfo;
            HitInfo& topInfo = hInfoContext.topHitInfo;
            
            const auto N = hInfo.N.GetNormalized();
            
            const auto zRight = (rightRay.p - hInfo.p).Dot(N);
            const auto tRight = zRight / (rightRay.dir.Dot(N));
            const auto pRight = rightRay.p + tRight * rightRay.dir;
            const auto nRight = N;
            
            const auto zTop = (topRay.p - hInfo.p).Dot(N);
            const auto tTop = zTop / (topRay.dir.Dot(N));
            const auto pTop = topRay.p + tTop * topRay.dir;
            const auto nTop = N;
            
            assert(!isnan(nTop.x));
            assert(!isnan(nRight.x));
            
            rightInfo.N = nRight;
            rightInfo.z = tRight;
            rightInfo.p = pRight;
            
            topInfo.N = nTop;
            topInfo.z = tTop;
            topInfo.p = pTop;
            
            Vec3f texRight;
            {
                Matrix3<float> mmRight = Matrix3<float>(-rightRay.dir, v01, v02);
                mmRight.Invert();
                
                Vec3f vv0 = mmRight * v0;
                Vec3f vv1 = mmRight * v1;
                Vec3f vv2 = mmRight * v2;
                Vec3f pp = mmRight * pRight;
                
                Vec2f v0_2d = Vec2f(vv0.y, vv0.z);
                Vec2f v1_2d = Vec2f(vv1.y, vv1.z);
                Vec2f v2_2d = Vec2f(vv2.y, vv2.z);
                Vec2f p_2d = Vec2f(pp.y, pp.z);
                
                Vec2f pv0 = v0_2d - p_2d;
                Vec2f pv1 = v1_2d - p_2d;
                Vec2f pv2 = v2_2d - p_2d;
                
                float two_a0 = pv1.Cross(pv2);
                float two_a1 = pv2.Cross(pv0);
                float two_a2 = pv0.Cross(pv1);
                
                float two_a = (v1_2d - v0_2d).Cross(v2_2d - v0_2d);
                
                float beta0 = two_a0 / two_a;
                float beta1 = two_a1 / two_a;
                float beta2 = two_a2 / two_a;
                
                const TriFace& texFace = FT(faceID);
                
                const Vec3f& t0 = VT(texFace.v[0]);
                const Vec3f& t1 = VT(texFace.v[1]);
                const Vec3f& t2 = VT(texFace.v[2]);
                
                texRight = beta0 * t0 + beta1 * t1 + beta2 * t2;
            }
            
            Vec3f texTop;
            {
                Matrix3<float> mmTop = Matrix3<float>(-topRay.dir, v01, v02);
                mmTop.Invert();
                
                Vec3f vv0 = mmTop * v0;
                Vec3f vv1 = mmTop * v1;
                Vec3f vv2 = mmTop * v2;
                Vec3f pp = mmTop * pTop;
                
                Vec2f v0_2d = Vec2f(vv0.y, vv0.z);
                Vec2f v1_2d = Vec2f(vv1.y, vv1.z);
                Vec2f v2_2d = Vec2f(vv2.y, vv2.z);
                Vec2f p_2d = Vec2f(pp.y, pp.z);
                
                Vec2f pv0 = v0_2d - p_2d;
                Vec2f pv1 = v1_2d - p_2d;
                Vec2f pv2 = v2_2d - p_2d;
                
                float two_a0 = pv1.Cross(pv2);
                float two_a1 = pv2.Cross(pv0);
                float two_a2 = pv0.Cross(pv1);
                
                float two_a = (v1_2d - v0_2d).Cross(v2_2d - v0_2d);
                
                float beta0 = two_a0 / two_a;
                float beta1 = two_a1 / two_a;
                float beta2 = two_a2 / two_a;
                
                const TriFace& texFace = FT(faceID);
                
                const Vec3f& t0 = VT(texFace.v[0]);
                const Vec3f& t1 = VT(texFace.v[1]);
                const Vec3f& t2 = VT(texFace.v[2]);
                
                texTop = beta0 * t0 + beta1 * t1 + beta2 * t2;
            }
        
            hInfo.duvw[0] = (texRight - hInfo.uvw) / rayContext.delta;
            hInfo.duvw[1] = (texTop - hInfo.uvw) / rayContext.delta;
        }
        else
        {
            hInfo.duvw[0] = Vec3f(0.0f, 0.0f, 0.0f);
            hInfo.duvw[1] = Vec3f(0.0f, 0.0f, 0.0f);
        }
        
        return true;
    }
    else
    {
        return false;
    }
}

bool TriObj::IntersectTriangle( Ray const &ray, HitInfo &hInfo, int hitSide, unsigned int faceID) const
{
    const TriFace& face = F(faceID);
    
    const Vec3f& v0 = V(face.v[0]);
    const Vec3f& v1 = V(face.v[1]);
    const Vec3f& v2 = V(face.v[2]);
    
    Vec3f v01 = v1 - v0;
    Vec3f v02 = v2 - v0;
    
    // unnormalized
    Vec3f n = v01.Cross(v02);
    
    // (ray.origin + ray.dir * t - v0).dot(n) = 0
    float dirDotN = ray.dir.Dot(n);
    
    if(abs(dirDotN) <= FLT_EPSILON)
    {
        return false;
    }
    
    float originDotN = ray.p.Dot(n);
    float v0DotN = v0.Dot(n);
    float t = (v0DotN - originDotN) / dirDotN;
    
    if(t < 0.0f)
    {
        return false;
    }
    
    bool isFront = (dirDotN < 0.0f);
    
    if(hitSide == HIT_FRONT && !isFront)
    {
        return false;
    }
    
    if(hitSide == HIT_BACK && isFront)
    {
        return false;
    }
    
    Vec3f p = ray.p + ray.dir * t;
    
    Matrix3<float> mm = Matrix3<float>(-ray.dir, v01, v02);
    mm.Invert();
    
    Vec3f vv0 = mm * v0;
    Vec3f vv1 = mm * v1;
    Vec3f vv2 = mm * v2;
    Vec3f pp = mm * p;
    
    Vec2f v0_2d = Vec2f(vv0.y, vv0.z);
    Vec2f v1_2d = Vec2f(vv1.y, vv1.z);
    Vec2f v2_2d = Vec2f(vv2.y, vv2.z);
    Vec2f p_2d = Vec2f(pp.y, pp.z);
    
    Vec2f pv0 = v0_2d - p_2d;
    Vec2f pv1 = v1_2d - p_2d;
    Vec2f pv2 = v2_2d - p_2d;
    
    float two_a0 = pv1.Cross(pv2);
    if(two_a0 <= 0.0f)
    {
        return false;
    }
    float two_a1 = pv2.Cross(pv0);
    if(two_a1 <= 0.0f)
    {
        return false;
    }
    float two_a2 = pv0.Cross(pv1);
    if(two_a2 <= 0.0f)
    {
        return false;
    }
    
    float two_a = (v1_2d - v0_2d).Cross(v2_2d - v0_2d);
    
    float beta0 = two_a0 / two_a;
    float beta1 = two_a1 / two_a;
    float beta2 = two_a2 / two_a;
    
    const TriFace& normalFace = FN(faceID);
    
    const Vec3f& n0 = VN(normalFace.v[0]);
    const Vec3f& n1 = VN(normalFace.v[1]);
    const Vec3f& n2 = VN(normalFace.v[2]);
    
    Vec3f normal = (beta0 * n0 + beta1 * n1 + beta2 * n2).GetNormalized();
    
    const TriFace& texFace = FT(faceID);
    
    const Vec3f& t0 = VT(texFace.v[0]);
    const Vec3f& t1 = VT(texFace.v[1]);
    const Vec3f& t2 = VT(texFace.v[2]);
    
    Vec3f tex = beta0 * t0 + beta1 * t1 + beta2 * t2;
    
    hInfo.p = p;
    hInfo.z = t;
    hInfo.N = isFront? normal: -1.0f * normal;
    hInfo.front = isFront;
    hInfo.uvw = tex;
    
    return true;
}

void TriObj::ViewportDisplay(const Material *mtl) const
{
    
}

Vec3f PlaneCalculatePlaneTexCoord(const Vec3f &p)
{
    return p * 0.5f + Vec3f(0.5f, 0.5f, 0.0f);
};

Vec3f Plane::Sample() const
{
	float x = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))* 2.0f - 1.0f;
	float y = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 2.0f - 1.0f;
	
	return parent->TransformPointToWorld(Vec3f(x, y, 0.0f));
}

float Plane::Area() const
{
	Vec3f zero = parent->TransformPointToWorld(Vec3f(0.0f, 0.0f, 0.0f));
	Vec3f width = parent->TransformPointToWorld(Vec3f(2.0f, 0.0f, 0.0f));
	Vec3f height = parent->TransformPointToWorld(Vec3f(0.0f, 2.0f, 0.0f));
	
	return (width - zero).Length() * (height - zero).Length();
}

bool Plane::IntersectRay(RayContext &rayContext, HitInfoContext& hInfoContext, int hitSide) const
{
    HitInfo& hInfo = hInfoContext.mainHitInfo;

    if(IntersectRay(rayContext.cameraRay, hInfo, hitSide))
    {
        if(rayContext.hasDiff)
        {
            // RAY DIFF
            const Ray& rightRay = rayContext.rightRay;
            const Ray& topRay = rayContext.topRay;
            
            HitInfo& rightInfo = hInfoContext.rightHitInfo;
            HitInfo& topInfo = hInfoContext.topHitInfo;
            
            const auto N = hInfo.N.GetNormalized();
            
            const auto zRight = rightRay.p.Dot(N);
            const auto tRight = -1.0f * zRight / (rightRay.dir.Dot(N));
            const auto pRight = rightRay.p + tRight * rightRay.dir;
            const auto nRight = N;
            
            const auto zTop = topRay.p.Dot(N);
            const auto tTop = -1.0f * zTop / (topRay.dir.Dot(N));
            const auto pTop = topRay.p + tTop * topRay.dir;
            const auto nTop = N;
            
            assert(!isnan(nTop.x));
            assert(!isnan(nRight.x));
            
            hInfoContext.rightHitInfo.p = pRight;
            hInfoContext.rightHitInfo.N = nRight;
            hInfoContext.rightHitInfo.z = tRight;
            
            hInfoContext.topHitInfo.p = pTop;
            hInfoContext.topHitInfo.N = nTop;
            hInfoContext.topHitInfo.z = tTop;
        
            hInfo.duvw[0] = (PlaneCalculatePlaneTexCoord(hInfoContext.rightHitInfo.p) - hInfo.uvw) / rayContext.delta;
            hInfo.duvw[1] = (PlaneCalculatePlaneTexCoord(hInfoContext.topHitInfo.p) - hInfo.uvw) / rayContext.delta;
        }
        else
        {
            hInfo.duvw[0] = Vec3f(0.0f, 0.0f, 0.0f);
            hInfo.duvw[1] = Vec3f(0.0f, 0.0f, 0.0f);
        }

        return true;
    }
    else
    {
        return false;
    }
}

bool Plane::IntersectRay(Ray const &ray, HitInfo &hInfo, int hitSide) const
{
    Vec3f n = Vec3f(0.0f, 0.0f, 1.0f);
    float dirDotN = ray.dir.Dot(n);
    
    if(abs(dirDotN - 0.0f) < 0.000001f)
    {
        return false;
    }
    
    float originDotN = ray.p.Dot(n);
    float t = -1.0f * (originDotN / dirDotN);
    
    if(t <= 0.0f)
    {
        return false;
    }
    
    Vec3f p = ray.p + ray.dir * t;
    
    if(abs(p.x) >= 1.0f)
    {
        return false;
    }
    
    if(abs(p.y) >= 1.0f)
    {
        return false;
    }
    
    bool isFront = (dirDotN < 0.0f);
    
    if(hitSide == HIT_FRONT && !isFront)
    {
        return false;
    }
    
    if(hitSide == HIT_BACK && isFront)
    {
        return false;
    }
    
    hInfo.front = isFront;
    hInfo.N = isFront? n : -1.0f * n;
    hInfo.p = p;
    hInfo.z = t;

	// calculate tangent
	hInfo.Tangent = Vec3f(1.0f, 0.0f, 0.0f);
	hInfo.Bitangent = Vec3f(0.0f, 1.0f, 0.0f);
    
	hInfo.uvw = PlaneCalculatePlaneTexCoord(p);

    return true;
}

void Plane::ViewportDisplay(const Material *mtl) const
{
    
}

Vec3f SphereCalculateCoord(const Vec3f& p, float reverseLength)
{
    return Vec3f(
                 (0.5f - atan2(p.x, p.y) / (2.0f * Pi<float>())),
                 (0.5f + asin(p.z * reverseLength) / (Pi<float>())),
                 0.0f
                 );
};

bool Sphere::IntersectRay(RayContext &rayContext, HitInfoContext& hInfoContext, int hitSide) const
{
    HitInfo& hInfo = hInfoContext.mainHitInfo;
    
    if(IntersectRay(rayContext.cameraRay, hInfo, hitSide))
    {
        if(!rayContext.hasDiff)
        {
            hInfo.duvw[0] = Vec3f(0.0f, 0.0f, 0.0f);
            hInfo.duvw[1] = Vec3f(0.0f, 0.0f, 0.0f);
        }
        // RAY DIFF
        else
        {
            const Ray& rightRay = rayContext.rightRay;
            const Ray& topRay = rayContext.topRay;
            
            HitInfo& rightHitInfo = hInfoContext.rightHitInfo;
            HitInfo& topHitInfo = hInfoContext.topHitInfo;
            
            const auto N = hInfo.N.GetNormalized();
            
            const auto zRight = (rightRay.p - hInfo.p).Dot(N);
            const auto tRight = -1.0f * zRight / (rightRay.dir.Dot(N));
            const auto pRight = rightRay.p + tRight * rightRay.dir;
            const auto nRight = pRight.GetNormalized();
            
            const auto zTop = (topRay.p - hInfo.p).Dot(N);
            const auto tTop = -1.0f * zTop / (topRay.dir.Dot(N));
            const auto pTop = topRay.p + tTop * topRay.dir;
            const auto nTop = pTop.GetNormalized();
            
            assert(!isnan(nTop.x));
            assert(!isnan(nRight.x));
            
            rightHitInfo.p = pRight;
            rightHitInfo.N = nRight;
            rightHitInfo.z = tRight;
            
            topHitInfo.p = pTop;
            topHitInfo.N = nTop;
            topHitInfo.z = tTop;
        
            hInfo.duvw[0] = (SphereCalculateCoord(rightHitInfo.p, 1.0f/ rightHitInfo.p.Length()) - hInfo.uvw) / rayContext.delta;
            hInfo.duvw[1] = (SphereCalculateCoord(topHitInfo.p, 1.0f/ topHitInfo.p.Length()) - hInfo.uvw) / rayContext.delta;
        }
        
        return true;
    }
    else
    {
        return false;
    }
}

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
                        hInfo.uvw = SphereCalculateCoord(intersectPoint, 1.0f);
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
                            hInfo.N = -1.0f * intersectPoint2;
//                            hInfo.N.Normalize();
                            hInfo.p = intersectPoint2;
                            hInfo.front = false;
                            hInfo.uvw = SphereCalculateCoord(intersectPoint2, 1.0f);
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
                        hInfo.uvw = SphereCalculateCoord(intersectPoint1, 1.0f);
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
    
