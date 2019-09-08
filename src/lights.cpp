#include "lights.h"

extern Node rootNode;

void GenLight::SetViewportParam(int lightID, ColorA ambient, ColorA intensity, Vec4f pos ) const
{
    
}

bool ShadowTraversal(Node* node, Ray& ray, float t_max)
{
    Ray objectRay = node->ToNodeCoords(ray);
    Object* obj = node->GetNodeObj();
    
    if(obj != nullptr)
    {
        HitInfo objHitInfo;
        if(obj->IntersectRay(objectRay, objHitInfo))
        {
            return true;
        }
    }
    
    for(int i = 0; i < node->GetNumChild(); i++)
    {
        Node* child = node->GetChild(i);
        if(ShadowTraversal(child, objectRay, t_max))
        {
            return true;
        }
    }
    
    return false;
}

float GenLight::Shadow(Ray ray, float t_max)
{
    if (ShadowTraversal(&rootNode, ray, t_max))
    {
        return 0.0f;
    }
    return 1.0f;
}


