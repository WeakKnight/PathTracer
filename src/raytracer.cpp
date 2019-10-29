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
#include "filter.h"

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
Color24 *mySampleImg = nullptr;
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

Vec2f RandomPointInCircle(float radius)
{
//    float x = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * radius * 2 - radius;
//    float y = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * radius * 2 - radius;
//
//    while((x * x + y * y) > (radius * radius))
//    {
//        x = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * radius * 2 - radius;
//        y = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * radius * 2 - radius;
//    }
//
//    return Vec2f(x, y);
    // generate a random value between 0 to Radius as the value of Cumulative Distribution Function
    float S = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
    // S = r2 / R2, choose r based on F
    float r = sqrtf(S) * radius;
    float theta = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * Pi<float>() * 2;
    
    float x = r * cos(theta);
    float y = r * sin(theta);
    
    return Vec2f(x, y);
}

Ray GenCameraRay(int x, int y, float xOffset, float yOffset, bool normalize)
{
    // random in a circle
    Vec2f randomCirclePoint = RandomPointInCircle(camera.dof);
    
    Ray cameraRay;
    cameraRay.p = camera.pos + cameraRight * randomCirclePoint.x + cameraUp * randomCirclePoint.y;
    cameraRay.dir =
    (cameraRight * (-1.0f * imgPlaneWidth * 0.5f + x * texelWdith + xOffset * texelWdith) +
    cameraUp * (imgPlaneHeight * 0.5f - y * texelHeight - yOffset * texelHeight) +
    cameraFront * camera.focaldist) - (cameraRight * randomCirclePoint.x + cameraUp * randomCirclePoint.y);
    
    if(normalize)
    {
        cameraRay.Normalize();
    }
    
    return cameraRay;
}


RayContext GenCameraRayContext(int x, int y, float offsetX, float offsetY)
{
    float delta = RAY_DIFF_DELTA;
    
    auto ray = GenCameraRay(x, y, offsetX, offsetY, false);
    
    RayContext result;
    result.cameraRay = ray;
    
    result.rightRay.p = ray.p;
    result.topRay.p = ray.p;
    
    auto rightOffset = ray.dir + cameraRight * texelWdith * delta;
    result.rightRay.dir = rightOffset.GetNormalized();
    
    auto topOffset = ray.dir + cameraUp * texelHeight * delta;
    result.topRay.dir = (topOffset).GetNormalized();
    
    result.cameraRay.Normalize();
    
    result.delta = delta;
    
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
    if(!sampleTexture)
    {
        sampleTexture = std::make_shared<Texture2D>();
    }
    if(!filterTexture)
    {
        filterTexture = std::make_shared<Texture2D>();
    }
    
#endif
    // scene load, ini global variables
    LoadScene(scene_path);
    
    // make sure camera dir normalized
    camera.dir.Normalize();
    
    cameraUp = camera.up;
    cameraFront = camera.dir;
    cameraRight = cameraFront.Cross(cameraUp);
    
    imgPlaneHeight = camera.focaldist * tanf(camera.fov * 0.5f /180.0f * static_cast<float>(M_PI)) * 2.0f;
    imgPlaneWidth = imgPlaneHeight * static_cast<float>(camera.imgWidth) / static_cast<float>(camera.imgHeight);
    
    // pixel's world space size
    texelWdith = imgPlaneWidth / static_cast<float>(camera.imgWidth);
    texelHeight = imgPlaneHeight / static_cast<float>(camera.imgHeight);
    
//    gaussianFilter = new GaussianFilter(renderImage.GetPixels(),
//                                        (unsigned int)renderImage.GetWidth(),
//                                        (unsigned int)renderImage.GetHeight(),
//                                        0.3f,
//                                        Vec2f(3.0f, 3.0f));
//    
//    colorShiftFilter = new ColorShiftFilter(gaussianFilter->GetOutput(),
//                                (unsigned int)renderImage.GetWidth(),
//                                (unsigned int)renderImage.GetHeight());
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
//    1;
    std::thread::hardware_concurrency() - 1;
    std::vector<std::future<void>> threads;
    
    std::size_t size = renderImage.GetWidth() * renderImage.GetHeight();
    
    renderImage.ResetNumRenderedPixels();
    
    static HaltonSampler* haltonSampler = new HaltonSampler();
    // for test
    haltonSampler->SetMinimumSampleCount(4);
    haltonSampler->SetSampleCount(16);
    
    for(std::size_t i = 0; i < cores; i++)
    {
        threads.push_back(std::async([=](){
            for(std::size_t index = i; index < size; index+= cores)
            {
                int y = index / renderImage.GetWidth();
                int x = index - y * renderImage.GetWidth();
                
//                if(x == 399 && y == 150)
//                {
//                    int a = 1;
//                }
                SampleResult sampleResult = haltonSampler->SamplePixel(x, y);
                
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
                    renderImage.ComputeSampleCountImage();
                    
//                    gaussianFilter->Compute();
//                    colorShiftFilter->Compute();
                    
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
    
    if(mySampleImg != nullptr)
    {
        delete [] mySampleImg;
    }
    
    mySampleImg = new Color24[renderImage.GetWidth() * renderImage.GetHeight()];
}

void RayTracer::UpdateRenderResult()
{
    RenderImageHelper::CalculateMyDepthImg(myZImg, renderImage);
    RenderImageHelper::CalculateMySampleImg(mySampleImg, renderImage);
    
    sampleTexture->SetData((unsigned char *)mySampleImg, renderImage.GetWidth(), renderImage.GetHeight(), GL_RGB);
    zbufferTexture->SetData((unsigned char *)myZImg, renderImage.GetWidth(), renderImage.GetHeight(), GL_RGB);
    renderTexture->SetData((unsigned char *)renderImage.GetPixels(), renderImage.GetWidth(), renderImage.GetHeight());
    normalTexture->SetData((unsigned char *)normalPixels, renderImage.GetWidth(), renderImage.GetHeight());
    
//    filterTexture->SetData((unsigned char *)colorShiftFilter->GetOutput(), renderImage.GetWidth(), renderImage.GetHeight());
}

void RayTracer::WriteToFile()
{
    renderImage.SaveImage("colorbuffer.png");
    renderImage.SaveZImage("zbuffer.png");
    renderImage.SaveSampleCountImage("samplecount.png");
//    renderImage.SaveImage("colorShiftFilter.png", colorShiftFilter->GetOutput());
//    renderImage.SaveImage("gaussianFilter.png", gaussianFilter->GetOutput());
}
