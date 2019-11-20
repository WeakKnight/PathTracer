#pragma once
#include <memory>
#include <string>
#include "cyVector.h"
#include "scene.h"
#include "config.h"
#include <mutex>
#include <atomic>

Ray GenCameraRay(int x, int y, float xOffset = 0.5f, float yOffset = 0.5f, bool normalize = true);
bool GenerateRayForAnyIntersection(Ray& ray, float t_max = BIGFLOAT);
bool GenerateRayForNearestIntersection(RayContext& ray, HitInfoContext& hitinfoContext, int side, float& t);
bool TraceNode(HitInfoContext& hitInfoContext, RayContext& rayContext, Node* node, int side = HIT_FRONT);
RayContext GenCameraRayContext(int x, int y, float offsetX, float offsetY);

class Node;
class Filter;
class GaussianFilter;
class ColorShiftFilter;
class Texture2D;

class RayTracer
{
    public:

    void Init();
    void Run();
    void UpdateRenderResult();
    void WriteToFile();
	void Pause();
    
    std::shared_ptr<Texture2D> GetZBufferTexture(){return zbufferTexture;}
    std::shared_ptr<Texture2D> GetRenderTexture(){return renderTexture;}
    std::shared_ptr<Texture2D> GetNormalTexture(){return normalTexture;}
    std::shared_ptr<Texture2D> GetSampleTexture(){return sampleTexture;}
    std::shared_ptr<Texture2D> GetFilterTexture(){return filterTexture;}
	std::shared_ptr<Texture2D> GetIrradianceTexture() { return irradianceTexture; }
    
    char* scene_path = ScenePath;
    
private:
    GaussianFilter* gaussianFilter;
   // ColorShiftFilter* colorShiftFilter;
    std::shared_ptr<Texture2D> zbufferTexture;
    std::shared_ptr<Texture2D> renderTexture;
    std::shared_ptr<Texture2D> normalTexture;
    std::shared_ptr<Texture2D> sampleTexture;
	std::shared_ptr<Texture2D> filterTexture;
    std::shared_ptr<Texture2D> irradianceTexture;

	std::mutex mtx;
};

