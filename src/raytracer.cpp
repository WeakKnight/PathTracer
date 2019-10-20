#include "raytracer.h"
#include "scene.h"
#include "materials.h"
#include "lights.h"
#include "texture2d.h"
#include "renderimagehelper.h"

#include "xmlload.h"

#include "objects.h"
#include <thread>
#include <vector>
#include <future>

#include "spdlog/spdlog.h"
#include "GLFW/glfw3.h"

#include "application.h"

#include "sampler.h"

Node rootNode;
Camera camera;
RenderImage renderImage;
Sphere theSphere;
Plane thePlane;
MaterialList materials;
LightList lights;
ObjFileList objList;
TexturedColor background;
TexturedColor environment;
TextureList textureList;

float imgPlaneHeight;
float imgPlaneWidth;

// pixel's world space size
float texelWdith;
float texelHeight;

cyVec3f cameraUp;
cyVec3f cameraFront;
cyVec3f cameraRight;

Color24 *myZImg = nullptr;

Color24 *normalPixels = nullptr;

float buildTime = 0.0f;

bool InternalGenerateRayForAnyIntersection(Node* node, Ray& ray, float t_max)
{
    Ray objectRay = node->ToNodeCoords(ray);
    Object* obj = node->GetNodeObj();
    
    if(obj != nullptr)
    {
        HitInfo objHitInfo;
        if(obj->IntersectRay(objectRay, objHitInfo))
        {
            if(objHitInfo.z < t_max)
            {
                return true;
            }
        }
    }
    
    for(int i = 0; i < node->GetNumChild(); i++)
    {
        Node* child = node->GetChild(i);
        
        if(InternalGenerateRayForAnyIntersection(child, objectRay, t_max))
        {
            return true;
        }
    }
    
    return false;
}

bool GenerateRayForAnyIntersection(Ray& ray, float t_max)
{
    return InternalGenerateRayForAnyIntersection(&rootNode, ray, t_max);
}

bool TraceNode(HitInfoContext& hitInfoContext, RayContext& rayContext, Node* node, int side = HIT_FRONT)
{
    bool result = false;
    
    RayContext rayContextInNodeSpace = node->ToNodeCoords(rayContext);
    
    HitInfo& hitInfo = hitInfoContext.mainHitInfo;
    HitInfo& rightInfo = hitInfoContext.rightHitInfo;
    HitInfo& topInfo = hitInfoContext.topHitInfo;
    
    HitInfoContext currentHitInfoContext;
    HitInfo& currentHitInfo = currentHitInfoContext.mainHitInfo;
    
    auto obj = node->GetNodeObj();
    
    if(obj != nullptr && obj->IntersectRay(rayContextInNodeSpace, currentHitInfoContext, side))
    {
        result = true;
        
        if(currentHitInfo.z < hitInfo.z)
        {
            // world space
            hitInfo.z = currentHitInfo.z;
            // node space
            hitInfo.p = currentHitInfo.p;
            // node space
            hitInfo.N = currentHitInfo.N;
            hitInfo.node = node;
            hitInfo.front = currentHitInfo.front;
            
            hitInfo.uvw = currentHitInfo.uvw;
            hitInfo.mtlID = currentHitInfo.mtlID;
            hitInfo.duvw[0] = currentHitInfo.duvw[0];
            hitInfo.duvw[1] = currentHitInfo.duvw[1];
            
            rightInfo.N = currentHitInfoContext.rightHitInfo.N;
            rightInfo.z = currentHitInfoContext.rightHitInfo.z;
            rightInfo.p = currentHitInfoContext.rightHitInfo.p;
            
            topInfo.N = currentHitInfoContext.topHitInfo.N;
            topInfo.z = currentHitInfoContext.topHitInfo.z;
            topInfo.p = currentHitInfoContext.topHitInfo.p;
            
            assert(!isnan(rightInfo.N.Sum()));
            assert(!isnan(topInfo.N.Sum()));
            
            node->FromNodeCoords(hitInfoContext);
            
            assert(hitInfo.node != nullptr);
        }
    }
    
    for(int index = 0; index < node->GetNumChild(); index++)
    {
        HitInfoContext currentHitInfoContext;
        HitInfo& currentHitInfo = currentHitInfoContext.mainHitInfo;
        
        Node* child = node->GetChild(index);
        if(TraceNode(currentHitInfoContext, rayContextInNodeSpace, child, side))
        {
            result = true;
            
            if(currentHitInfo.z < hitInfo.z)
            {
                hitInfo.z = currentHitInfo.z;
                hitInfo.p = currentHitInfo.p;
                hitInfo.N = currentHitInfo.N;
                hitInfo.node = currentHitInfo.node;
                hitInfo.front = currentHitInfo.front;
                hitInfo.uvw = currentHitInfo.uvw;
                hitInfo.mtlID = currentHitInfo.mtlID;
                hitInfo.duvw[0] = currentHitInfo.duvw[0];
                hitInfo.duvw[1] = currentHitInfo.duvw[1];
                
                rightInfo.N = currentHitInfoContext.rightHitInfo.N;
                rightInfo.z = currentHitInfoContext.rightHitInfo.z;
                rightInfo.p = currentHitInfoContext.rightHitInfo.p;
                
                topInfo.N = currentHitInfoContext.topHitInfo.N;
                topInfo.z = currentHitInfoContext.topHitInfo.z;
                topInfo.p = currentHitInfoContext.topHitInfo.p;
                
                assert(!isnan(rightInfo.N.Sum()));
                assert(!isnan(topInfo.N.Sum()));
                
                node->FromNodeCoords(hitInfoContext);
                
                assert(hitInfo.node != nullptr);
            }
        }
    }
    
    return result;
}

bool GenerateRayForNearestIntersection(RayContext& rayContext, HitInfoContext& hitinfoContext, int side, float& t)
{
    bool result = TraceNode(hitinfoContext, rayContext, &rootNode, side);
    
    if(result)
    {
        t += hitinfoContext.mainHitInfo.z;
    }
    
    return result;
}

Ray GenCameraRay(int x, int y, float xOffset, float yOffset)
{
    Ray cameraRay;
    
    cameraRay.p = camera.pos;
    cameraRay.dir =
    cameraRight * (-1.0f * imgPlaneWidth * 0.5f + x * texelWdith + xOffset * texelWdith) +
    cameraUp * (imgPlaneHeight * 0.5f - y * texelHeight - yOffset * texelHeight) +
    cameraFront;
    
    cameraRay.Normalize();
    
    return cameraRay;
}

RayContext GenRayContext(Ray ray, float delta)
{
    RayContext result;
    result.cameraRay = ray;
    
    result.rightRay.p = ray.p;
    result.topRay.p = ray.p;
    
    result.rightRay.dir = (result.cameraRay.dir + cameraRight * texelWdith * delta).GetNormalized();
    result.topRay.dir = (result.cameraRay.dir + cameraUp * texelHeight * delta).GetNormalized();
    
    result.delta = delta;
    
    return result;
}

RayContext GenCameraRayContext(int x, int y)
{
    return GenRayContext(GenCameraRay(x, y));
}

void RayTracer::Init()
{
#ifdef IMGUI_DEBUG
    if(!renderTexture)
    {
        renderTexture = std::make_shared<Texture2D>();
    }
    if(!zbufferTexture)
    {
        zbufferTexture = std::make_shared<Texture2D>();
    }
    if(!normalTexture)
    {
        normalTexture = std::make_shared<Texture2D>();
    }
    
#endif
    
    // scene load, ini global variables
    LoadScene(scene_path);
    
    // make sure camera dir normalized
    camera.dir.Normalize();
    
    cameraUp = camera.up;
    cameraFront = camera.dir;
    cameraRight = cameraFront.Cross(cameraUp);
    
    imgPlaneHeight = 1.0f * tanf(camera.fov * 0.5f /180.0f * static_cast<float>(M_PI)) * 2.0f;
    imgPlaneWidth = imgPlaneHeight * static_cast<float>(camera.imgWidth) / static_cast<float>(camera.imgHeight);
    
    // pixel's world space size
    texelWdith = imgPlaneWidth / static_cast<float>(camera.imgWidth);
    texelHeight = imgPlaneHeight / static_cast<float>(camera.imgHeight);
}

Color RootTrace(RayContext& rayContext, HitInfoContext& hitInfoContext, int x, int y)
{
    HitInfo& hitInfo = hitInfoContext.mainHitInfo;
    
    bool sthTraced = TraceNode(hitInfoContext, rayContext, &rootNode);
    
    if(sthTraced)
    {
        Color shadingResult = hitInfo.node->GetMaterial()->Shade(rayContext, hitInfoContext, lights, 4);
        return shadingResult;
    }
    else
    {
        return background.Sample(Vec3f(x/(float)renderImage.GetWidth(), y/(float)renderImage.GetHeight(), 0.0f));
    }
}

void RayTracer::Run()
{
    if(normalPixels != nullptr)
    {
        delete [] normalPixels;
    }
    
    normalPixels = new Color24[renderImage.GetWidth() * renderImage.GetHeight()];
    
    float now = glfwGetTime();
    
    std::size_t cores =
    1;
//    std::thread::hardware_concurrency() - 1;
    std::vector<std::future<void>> threads;
    
    std::size_t size = renderImage.GetWidth() * renderImage.GetHeight();
    
    renderImage.ResetNumRenderedPixels();
    
    HaltonSampler haltonSampler;
    // for test
    haltonSampler.SetSampleCount(4);
    
    for(std::size_t i = 0; i < cores; i++)
    {
        threads.push_back(std::async([=, &haltonSampler](){
            for(std::size_t index = i; index < size; index+= cores)
            {
                int y = index / renderImage.GetWidth();
                int x = index - y * renderImage.GetWidth();
                
//                if(x == 399 && y == 150)
//                {
//                    int a = 1;
//                }
                SampleResult sampleResult = haltonSampler.SamplePixel(x, y);
                
                RenderImageHelper::SetPixel(renderImage, x, y, Color24(sampleResult.avgColor.r * 255.0f, sampleResult.avgColor.g * 255.0f, sampleResult.avgColor.b * 255.0f));
                RenderImageHelper::SetDepth(renderImage, x, y, sampleResult.avgZ);
                RenderImageHelper::SetNormal(normalPixels, renderImage, x, y, sampleResult.avgN);
                
//                RayContext rayContext = GenCameraRayContext(x, y);
//                HitInfoContext hitInfoContext;
//                HitInfo& hitInfo = hitInfoContext.mainHitInfo;
//
//                auto color = RootTrace(rayContext, hitInfoContext, x, y);
                
//                RenderImageHelper::SetPixel(renderImage, x, y, Color24(color.r * 255.0f, color.g * 255.0f, color.b * 255.0f));
//                RenderImageHelper::SetDepth(renderImage, x, y, hitInfo.z);
//                RenderImageHelper::SetNormal(normalPixels, renderImage, x, y, hitInfo.N);
                
                renderImage.IncrementNumRenderPixel(1);
                // done
                if(renderImage.IsRenderDone())
                {
                    float finish = glfwGetTime();
                    
                    spdlog::debug("bvh build time is {}", buildTime);
                    spdlog::debug("time is {}", finish - now);
                    renderImage.ComputeZBufferImage();
#ifndef IMGUI_DEBUG
                    WriteToFile();
#endif
                }
            }
        }));
    }
    
    if(myZImg != nullptr)
    {
        delete [] myZImg;
    }
    
    myZImg = new Color24[renderImage.GetWidth() * renderImage.GetHeight()];
}

void RayTracer::UpdateRenderResult()
{
    RenderImageHelper::CalculateMyDepthImg(myZImg, renderImage);

    zbufferTexture->SetData((unsigned char *)myZImg, renderImage.GetWidth(), renderImage.GetHeight(), GL_RGB);
    renderTexture->SetData((unsigned char *)renderImage.GetPixels(), renderImage.GetWidth(), renderImage.GetHeight());
    normalTexture->SetData((unsigned char *)normalPixels, renderImage.GetWidth(), renderImage.GetHeight());
}

void RayTracer::WriteToFile()
{
    renderImage.SaveImage("colorbuffer.png");
    renderImage.SaveZImage("zbuffer.png");
}
