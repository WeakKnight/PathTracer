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

#include "utils.h"

#include "irradiancemap.h"
#include "config.h"

#include "bvh.h"

#include "pathtracer.h"
#include "constants.h"

Node rootNode;
Camera camera;
RenderImage renderImage;
MaterialList materials;
LightList lights;
ObjFileList objList;
TexturedColor background;
TexturedColor environment;
TextureList textureList;
BVHManager bvhManager;
LightComList lightList;

std::atomic<bool> outputing;

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
Color24 *irradianceCachePixels = nullptr;

float buildTime = 0.0f;

IrradianceCacheMap irradianceCacheMap;

bool InternalLightTest(Node* node, HitInfo& hitinfo, Ray& ray, Node* light)
{
	Ray objectRay = node->ToNodeCoords(ray);
	Object* obj = node->GetNodeObj();

	if (obj != nullptr && node == light)
	{
		HitInfo objHitInfo;
		if (obj->IntersectRay(objectRay, objHitInfo, HIT_FRONT_AND_BACK))
		{
			hitinfo.Copy(objHitInfo);
			node->FromNodeCoords(hitinfo);
			return true;
		}
	}

	for (int i = 0; i < node->GetNumChild(); i++)
	{
		Node* child = node->GetChild(i);

		HitInfo objHitInfo;
		if (InternalLightTest(child, objHitInfo, objectRay, light))
		{
			hitinfo.Copy(objHitInfo);
			node->FromNodeCoords(hitinfo);
			return true;
		}
	}

	return false;
}

bool InternalNonLightTest(Node* node, Ray& ray, float t_max, Node* light, float lightZ)
{
	Ray objectRay = node->ToNodeCoords(ray);
	Object* obj = node->GetNodeObj();

	bool result = false;
	// dont test light itself
	if (obj != nullptr && node != light)
	{
		HitInfo objHitInfo;
		if (obj->IntersectRay(objectRay, objHitInfo, HIT_FRONT))
		{
			// closer than light
			if (objHitInfo.z < t_max && objHitInfo.z < lightZ)
			{
				return true;
			}
		}
	}

	for (int i = 0; i < node->GetNumChild(); i++)
	{
		Node* child = node->GetChild(i);

		if (InternalNonLightTest(child, objectRay, t_max, light, lightZ))
		{
			return true;
		}
	}

	return false;
}

bool LightVisTest(Ray& ray, HitInfo& hitInfo,float t_max, Node* light)
{
	// if hit on light back side, must no light , pdf zero
	HitInfo lightHitInfo;
	if (InternalLightTest(&rootNode, lightHitInfo, ray, light))
	{
		if (!lightHitInfo.front)
		{
			return true;
		}
	}
	// don't hit light, pdf zero
	else
	{
		return true;
	}

	// if hit obj closer than light, pdf zero
	return InternalNonLightTest(&rootNode, ray, t_max, light, lightHitInfo.z);
}

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

bool TraceNode(HitInfoContext& hitInfoContext, RayContext& rayContext, Node* node, int side)
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
			hitInfo.Copy(currentHitInfo);
            hitInfo.node = node;
			hitInfo.mtl = node->GetMaterial();
            
			rightInfo.CopyForDiffRay(currentHitInfoContext.rightHitInfo);
			topInfo.CopyForDiffRay(currentHitInfoContext.topHitInfo);
            
            if(rayContextInNodeSpace.hasDiff)
            {
                assert(!isnan(rightInfo.N.Sum()));
                assert(!isnan(topInfo.N.Sum()));
            }
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
				hitInfo.Copy(currentHitInfo);

				rightInfo.CopyForDiffRay(currentHitInfoContext.rightHitInfo);
				topInfo.CopyForDiffRay(currentHitInfoContext.topHitInfo);
                
                if(rayContextInNodeSpace.hasDiff)
                {
                    assert(!isnan(rightInfo.N.Sum()));
                    assert(!isnan(topInfo.N.Sum()));
                }
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

Ray GenCameraRay(int x, int y, float xOffset, float yOffset, bool normalize)
{
    // random in a circle
    Vec2f randomCirclePoint = RandomPointInCircle(camera.dof);
    
    Ray cameraRay;
    cameraRay.p = camera.pos + cameraRight * randomCirclePoint.x + cameraUp * randomCirclePoint.y;
    cameraRay.dir =
    (cameraRight * (-1.0f * imgPlaneWidth * 0.5f + (float)x * texelWdith + 0.5f * texelWdith + xOffset * texelWdith) +
    cameraUp * (imgPlaneHeight * 0.5f - (float)y * texelHeight - 0.5f * texelHeight - yOffset * texelHeight) +
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
    result.topRay.dir = topOffset.GetNormalized();
    
    result.cameraRay.Normalize();
    
    result.delta = delta;
    
    return result;
}

void InitCamera()
{
	// make sure camera dir normalized
	camera.dir.Normalize();

	cameraUp = camera.up;
	cameraFront = camera.dir;
	cameraRight = cameraFront.Cross(cameraUp);

	imgPlaneHeight = camera.focaldist * tanf(camera.fov * 0.5f / 180.0f * static_cast<float>(M_PI)) * 2.0f;
	imgPlaneWidth = imgPlaneHeight * static_cast<float>(camera.imgWidth) / static_cast<float>(camera.imgHeight);

	// pixel's world space size
	texelWdith = imgPlaneWidth / static_cast<float>(camera.imgWidth);
	texelHeight = imgPlaneHeight / static_cast<float>(camera.imgHeight);
}

void RayTracer::Init()
{
	outputing = false;
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
	if (!irradianceTexture)
	{
		irradianceTexture = std::make_shared<Texture2D>();
	}
    
#endif
    // scene load, ini global variables
    LoadScene(scene_path);
	InitCamera();
}

void ComputeIrradianceCacheMap()
{
	while (irradianceCacheMap.ComputeNextPoint())
	{
	}
}

void RayTracer::Restart()
{
	outputing = true;
	Sleep(2000);
	Init();
}

void RayTracer::Run()
{
    if(normalPixels != nullptr)
    {
        delete [] normalPixels;
    }
    
    normalPixels = new Color24[renderImage.GetWidth() * renderImage.GetHeight()];

	if (irradianceCachePixels != nullptr)
	{
		delete[] irradianceCachePixels;
	}

	irradianceCachePixels = new Color24[renderImage.GetWidth() * renderImage.GetHeight()];

	if (myZImg != nullptr)
	{
		delete[] myZImg;
	}

	myZImg = new Color24[renderImage.GetWidth() * renderImage.GetHeight()];

	if (mySampleImg != nullptr)
	{
		delete[] mySampleImg;
	}

	mySampleImg = new Color24[renderImage.GetWidth() * renderImage.GetHeight()];
    
    float now = glfwGetTime();
    
    std::size_t cores = std::thread::hardware_concurrency() - 1;
    std::vector<std::future<void>> threads;
    
    std::size_t size = renderImage.GetWidth() * renderImage.GetHeight();
    
    renderImage.ResetNumRenderedPixels();
	if (IrradianceCache)
	{
		irradianceCacheMap.Initialize(renderImage.GetWidth(), renderImage.GetHeight());

		std::thread irradianceThread1(ComputeIrradianceCacheMap);
		std::thread irradianceThread2(ComputeIrradianceCacheMap);
		std::thread irradianceThread3(ComputeIrradianceCacheMap);
		std::thread irradianceThread4(ComputeIrradianceCacheMap);
		irradianceThread1.join();
		irradianceThread2.join();
		irradianceThread3.join();
		irradianceThread4.join();
	}

	PathTracer pathTracer;
	pathTracer.Init(renderImage.GetWidth(), renderImage.GetHeight());
	pathTracer.Run();
	pathTracer.Join();
}

void RayTracer::UpdateRenderResult()
{
    RenderImageHelper::CalculateMyDepthImg(myZImg, renderImage);
    RenderImageHelper::CalculateMySampleImg(mySampleImg, renderImage);
    
    sampleTexture->SetData((unsigned char *)mySampleImg, renderImage.GetWidth(), renderImage.GetHeight(), GL_RGB);
    zbufferTexture->SetData((unsigned char *)myZImg, renderImage.GetWidth(), renderImage.GetHeight(), GL_RGB);
    renderTexture->SetData((unsigned char *)renderImage.GetPixels(), renderImage.GetWidth(), renderImage.GetHeight());
    normalTexture->SetData((unsigned char *)normalPixels, renderImage.GetWidth(), renderImage.GetHeight());
	irradianceTexture->SetData((unsigned char*)irradianceCachePixels, renderImage.GetWidth(), renderImage.GetHeight());
//    filterTexture->SetData((unsigned char *)colorShiftFilter->GetOutput(), renderImage.GetWidth(), renderImage.GetHeight());
}

void RayTracer::Pause()
{
	outputing = true;
	spdlog::info("Outputing Set To True");
}

void RayTracer::WriteToFile()
{
	
	
	// unsigned char* tempData = new unsigned char[renderImage.GetWidth() * renderImage.GetHeight()];
	// memcpy(tempData, renderImage.GetPixels(), renderImage.GetWidth() * renderImage.GetHeight() * 3 * sizeof(unsigned char));
	// renderImage.SaveImage("color.png", tempData);

	// outputing = false;
     renderImage.SaveImage("colorbuffer.png");
    // renderImage.SaveZImage("zbuffer.png");
    // renderImage.SaveSampleCountImage("samplecount.png");
	
}
