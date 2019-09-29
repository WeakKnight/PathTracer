#ifndef _BVH_H_INCLUDED_
#define _BVH_H_INCLUDED_

#include "scene.h"
#include "cyVector.h"
#include <vector>
#include "cyTriMesh.h"
#include "objects.h"

class TriObj;

class BVHNode
{
public:
    BVHNode* left = nullptr;
    BVHNode* right = nullptr;
    Box bound;
    std::vector<unsigned int> faceList;
    
    bool IsLeaf()
    {
        return (left == nullptr && right == nullptr);
    }
};

class MeshBVH
{
public:
    MeshBVH(TriObj* triObj)
    {
        mesh = triObj;
        BuildRoot();
    }
    
    BVHNode* GetRoot()
    {
        return root;
    }
    
private:
    
    void BuildRoot()
    {
        // divide by middle
        // choose widest side from x y z
        root = new BVHNode();
        root->bound = mesh->GetBoundBox();
        
        // add all face to root node
        for(unsigned int i = 0; i < mesh->NF(); i++)
        {
            root->faceList.push_back(i);
        }
        
        BuildNode(root);
    }
    
    void BuildNode(BVHNode* parent)
    {
        if(parent->faceList.size() == 0)
        {
            return;
        }
        
        Vec3f lengthVec = (parent->bound.pmax - parent->bound.pmin);
        int maxIndex = lengthVec.MaxIndex();
        float middle = (parent->bound.pmin[maxIndex] + parent->bound.pmax[maxIndex]) * 0.5f;
        
        std::vector<unsigned int> leftFaceList;
        std::vector<unsigned int> rightFaceList;
        
        float left_xmin = BIGFLOAT;
        float left_xmax = -BIGFLOAT;
        
        float left_ymin = BIGFLOAT;
        float left_ymax = -BIGFLOAT;
        
        float left_zmin = BIGFLOAT;
        float left_zmax = -BIGFLOAT;
        
        float right_xmin = BIGFLOAT;
        float right_xmax = -BIGFLOAT;
        
        float right_ymin = BIGFLOAT;
        float right_ymax = -BIGFLOAT;
        
        float right_zmin = BIGFLOAT;
        float right_zmax = -BIGFLOAT;
        
        for(size_t i = 0; i < parent->faceList.size(); i++)
        {
            unsigned int faceId = parent->faceList[i];
            auto& face = mesh->F(faceId);
            
            // should this face add into left node
            bool left = false;
            
            Vec3f sum = Vec3f(0.0f, 0.0f, 0.0f);
            
            for(int index = 0; index < 3; index++)
            {
                int verticeIndex = face.v[index];
                Vec3f pos = mesh->V(verticeIndex);
                sum += pos;
            }
            
            Vec3f avg = sum/3.0f;
            
            if(avg[maxIndex] <= middle)
            {
                left = true;
            }
            
            for(int index = 0; index < 3; index++)
            {
                int verticeIndex = face.v[index];
                Vec3f pos = mesh->V(verticeIndex);
                
                if(left)
                {
                    if(pos.x > left_xmax)
                    {
                        left_xmax = pos.x;
                    }
                    if(pos.x < left_xmin)
                    {
                        left_xmin = pos.x;
                    }
                    if(pos.y > left_ymax)
                    {
                        left_ymax = pos.y;
                    }
                    if(pos.y < left_ymin)
                    {
                        left_ymin = pos.y;
                    }
                    if(pos.z > left_zmax)
                    {
                        left_zmax = pos.z;
                    }
                    if(pos.z < left_zmin)
                    {
                        left_zmin = pos.z;
                    }
                }
                else
                {
                    if(pos.x > right_xmax)
                    {
                        right_xmax = pos.x;
                    }
                    if(pos.x < right_xmin)
                    {
                        right_xmin = pos.x;
                    }
                    if(pos.y > right_ymax)
                    {
                        right_ymax = pos.y;
                    }
                    if(pos.y < right_ymin)
                    {
                        right_ymin = pos.y;
                    }
                    if(pos.z > right_zmax)
                    {
                        right_zmax = pos.z;
                    }
                    if(pos.z < right_zmin)
                    {
                        right_zmin = pos.z;
                    }
                }
            }
            
            if(left)
            {
                leftFaceList.push_back(faceId);
            }
            else
            {
                rightFaceList.push_back(faceId);
            }
        }
        
        
        Box leftBound = Box(left_xmin, left_ymin, left_zmin, left_xmax, left_ymax, left_zmax);
        Box rightBound = Box(right_xmin, right_ymin, right_zmin, right_xmax, right_ymax, right_zmax);
        
        float totalSurfaceArea = parent->bound.SurfaceArea();
        // possibility to hit based on surface area size
        float leftSuraceArea = leftBound.SurfaceArea();
        float rightSurfaceArea = rightBound.SurfaceArea();
        float pLeft = leftSuraceArea / totalSurfaceArea;
        float pRight = rightSurfaceArea / totalSurfaceArea;
        
        if(leftFaceList.size() == 0)
        {
            pLeft = 0.0f;
        }

        if(rightFaceList.size() == 0)
        {
            pRight = 0.0f;
        }
        
        float parentAsLeafTime = parent->faceList.size() * 1.0f;
        float parentAsBranchTime = 1.0f + pLeft * leftFaceList.size() + pRight * rightFaceList.size();
        
        // continue, divided into two leaves still could get better performance
        if(parentAsLeafTime > parentAsBranchTime)
        {
            parent->left = new BVHNode();
            parent->left->bound = leftBound;
            parent->left->faceList = leftFaceList;
            BuildNode(parent->left);
            
            parent->right = new BVHNode();
            parent->right->bound = rightBound;
            parent->right->faceList = rightFaceList;
            BuildNode(parent->right);
            
        }
        // stop
        else
        {
            parent->left = nullptr;
            parent->right = nullptr;
        }
    }
    
    TriObj* mesh;
    BVHNode* root;
    
};

#endif
