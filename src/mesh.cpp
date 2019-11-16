#include "mesh.h"
#include "bvh.h"

extern BVHManager bvhManager;

void Mesh::BuildBVH()
{
	bvh = bvhManager.Get(path);
	if (bvh == nullptr)
	{
		bvh = new MeshBVHNew(this);
		bvhManager.Set(path, bvh);
	}
}