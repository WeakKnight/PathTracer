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

Node rootNode;
Camera camera;
RenderImage renderImage;
Sphere theSphere;
MaterialList materials;
LightList lights;

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

bool TraceNode(HitInfo& hitInfo, Ray ray, Node* node, int side = HIT_FRONT)
{
    bool result = false;
    
    Ray rayInNodeSpace = node->ToNodeCoords(ray);
    
    HitInfo currentHitInfo;
    
    auto obj = node->GetNodeObj();
    
    if(obj != nullptr && obj->IntersectRay(rayInNodeSpace, currentHitInfo, side))
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
            
            node->FromNodeCoords(hitInfo);
        }
    }
    
    for(int index = 0; index < node->GetNumChild(); index++)
    {
        HitInfo currentHitInfo;
        Node* child = node->GetChild(index);
        if(TraceNode(currentHitInfo, rayInNodeSpace, child, side))
        {
            result = true;
            
            if(currentHitInfo.z < hitInfo.z)
            {
                hitInfo.z = currentHitInfo.z;
                hitInfo.p = currentHitInfo.p;
                hitInfo.N = currentHitInfo.N;
                hitInfo.node = currentHitInfo.node;
                hitInfo.front = currentHitInfo.front;
                
                node->FromNodeCoords(hitInfo);
            }
        }
    }
    
    return result;
}

bool GenerateRayForNearestIntersection(Ray ray, HitInfo& hitinfo, int side, float& t)
{
    bool result = TraceNode(hitinfo, ray, &rootNode, side);
    
    if(result)
    {
        t += hitinfo.z;
    }
    
    return result;
}

Ray GenCameraRay(int x, int y)
{
    Ray result;
    
    result.p = camera.pos;
    result.dir =
    cameraRight * (-1.0f * imgPlaneWidth * 0.5f + x * texelWdith + 0.5f * texelWdith) +
    cameraUp * (imgPlaneHeight * 0.5f - y * texelHeight - 0.5f * texelHeight) +
    cameraFront;
    
    result.Normalize();
    
    return result;
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

void RayTracer::Run()
{
    if(normalPixels != nullptr)
    {
        delete [] normalPixels;
    }
    
    normalPixels = new Color24[renderImage.GetWidth() * renderImage.GetHeight()];
    
    float now = glfwGetTime();
    
    std::size_t cores =
//    1;
    std::thread::hardware_concurrency() - 1;
    std::vector<std::future<void>> threads;
    
    std::size_t size = renderImage.GetWidth() * renderImage.GetHeight();
    
    renderImage.ResetNumRenderedPixels();
    
    for(std::size_t i = 0; i < cores; i++)
    {
        threads.push_back(std::async([=](){
            for(std::size_t index = i; index < size; index+= cores)
            {
                int y = index / renderImage.GetWidth();
                int x = index - y * renderImage.GetWidth();
                
                if(x == 278 && y == 357)
                {
                    int a = 1;
                }
                
                HitInfo hitInfo;
                Ray cameraRay = GenCameraRay(x, y);
                bool sthTraced = TraceNode(hitInfo, cameraRay, &rootNode);
                
                if(sthTraced)
                {
//                    hitInfo.node->FromNodeCoords(hitInfo);
                    Color shadingResult = hitInfo.node->GetMaterial()->Shade(cameraRay, hitInfo, lights, 3);
                    RenderImageHelper::SetPixel(renderImage, x, y, Color24(shadingResult.r * 255.0f, shadingResult.g * 255.0f, shadingResult.b * 255.0f));
//                    RenderImageHelper::SetPixel(renderImage, x, y, cyColor24::White());
                    RenderImageHelper::SetDepth(renderImage, x, y, hitInfo.z);
                    RenderImageHelper::SetNormal(normalPixels, renderImage, x, y, hitInfo.N);
                }
                else
                {
                    RenderImageHelper::SetPixel(renderImage, x, y, cyColor24::Black());
                    RenderImageHelper::SetDepth(renderImage, x, y, hitInfo.z);
                    RenderImageHelper::SetNormal(normalPixels, renderImage, x, y, Vec3f());
                }
                
                renderImage.IncrementNumRenderPixel(1);
                // done
                if(renderImage.IsRenderDone())
                {
                    float finish = glfwGetTime();
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
