#pragma once
#include <memory>
#include <string>
#include "cyVector.h"
#include "scene.h"

Ray GenCameraRay(int x, int y, float xOffset = 0.5f, float yOffset = 0.5f, bool normalize = true);
bool GenerateRayForAnyIntersection(Ray& ray, float t_max = BIGFLOAT);
bool GenerateRayForNearestIntersection(RayContext& ray, HitInfoContext& hitinfoContext, int side, float& t);
bool TraceNode(HitInfoContext& hitInfoContext, RayContext& rayContext, Node* node, int side = HIT_FRONT);
Color RootTrace(RayContext& rayContext, HitInfoContext& hitInfoContext, int x, int y);
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
    
    std::shared_ptr<Texture2D> GetZBufferTexture(){return zbufferTexture;}
    std::shared_ptr<Texture2D> GetRenderTexture(){return renderTexture;}
    std::shared_ptr<Texture2D> GetNormalTexture(){return normalTexture;}
    std::shared_ptr<Texture2D> GetSampleTexture(){return sampleTexture;}
    std::shared_ptr<Texture2D> GetFilterTexture(){return filterTexture;}
    
    char scene_path[256] = "assets/project11b.xml";
    
private:
    GaussianFilter* gaussianFilter;
   // ColorShiftFilter* colorShiftFilter;
    std::shared_ptr<Texture2D> zbufferTexture;
    std::shared_ptr<Texture2D> renderTexture;
    std::shared_ptr<Texture2D> normalTexture;
    std::shared_ptr<Texture2D> sampleTexture;
    std::shared_ptr<Texture2D> filterTexture;
};

