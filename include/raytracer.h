#pragma once
#include <memory>
#include <string>
#include "cyVector.h"
#include "scene.h"

Ray GenCameraRay(int x, int y, float xOffset = 0.5f, float yOffset = 0.5f);
bool GenerateRayForAnyIntersection(Ray& ray, float t_max = BIGFLOAT);
bool GenerateRayForNearestIntersection(RayContext& ray, HitInfoContext& hitinfoContext, int side, float& t);
RayContext GenRayContext(Ray ray, float delta = RAY_DIFF_DELTA);
Color RootTrace(RayContext& rayContext, HitInfoContext& hitInfoContext, int x, int y);

class Node;

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
    
    char scene_path[256] = "assets/project7.xml";
    
private:
    std::shared_ptr<Texture2D> zbufferTexture;
    std::shared_ptr<Texture2D> renderTexture;
    std::shared_ptr<Texture2D> normalTexture;
};

