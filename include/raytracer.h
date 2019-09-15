#pragma once
#include <memory>
#include <string>
#include "cyVector.h"
#include "scene.h"

bool GenerateRayForNearestIntersection(Ray& ray, HitInfo& hitinfo);

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
    
    char scene_path[256] = "assets/project4.xml";
    
private:
    std::shared_ptr<Texture2D> zbufferTexture;
    std::shared_ptr<Texture2D> renderTexture;
    std::shared_ptr<Texture2D> normalTexture;
};

