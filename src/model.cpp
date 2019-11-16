#include "model.h"
#include "bvh.h"

bool Model::TraceBVHNode(Ray const& ray, HitInfo& hInfo, int hitSide, Mesh& mesh, BVHNode* node) const
{
	if (node->IsLeaf())
	{
		bool result = false;
		for (unsigned i = 0; i < node->faceList.size(); i++)
		{
			unsigned faceId = node->faceList[i];
			HitInfo currentHitInfo;
			
			if (IntersectRayWithFace(ray, currentHitInfo, hitSide, mesh, mesh.faces[faceId]))
			{
				if (currentHitInfo.z < hInfo.z)
				{
					result = true;

					hInfo.Copy(currentHitInfo);
				}
			}
		}
		return result;
	}
	else
	{
		if (node->bound.IntersectRay(ray))
		{
			HitInfo leftHitInfo;
			HitInfo rightHitInfo;
			bool hitLeft = TraceBVHNode(ray, leftHitInfo, hitSide, mesh, node->left);
			bool hitRight = TraceBVHNode(ray, rightHitInfo, hitSide, mesh, node->right);

			if (hitLeft && hitRight) {
				if (leftHitInfo.z < rightHitInfo.z)
				{
					hInfo.Copy(leftHitInfo);
				}
				else
				{
					hInfo.Copy(rightHitInfo);
				}
				return true;
			}
			else if (hitLeft)
			{
				hInfo.Copy(leftHitInfo);
				return true;
			}
			else if (hitRight)
			{
				hInfo.Copy(rightHitInfo);
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
}

bool Model::TraceBVHNode(RayContext& rayContext, HitInfoContext& hInfoContext, int hitSide, Mesh& mesh, BVHNode* node) const
{
	HitInfo& hInfo = hInfoContext.mainHitInfo;
	HitInfo& hInfoRight = hInfoContext.rightHitInfo;
	HitInfo& hInfoTop = hInfoContext.topHitInfo;

	if (node->IsLeaf())
	{
		bool result = false;
		for (unsigned i = 0; i < node->faceList.size(); i++)
		{
			unsigned faceId = node->faceList[i];

			HitInfoContext currentHitInfoContext;
			HitInfo& currentHitInfo = currentHitInfoContext.mainHitInfo;

			if (IntersectRayWithFace(rayContext, currentHitInfoContext, hitSide, mesh, mesh.faces[faceId]))
			{
				if (currentHitInfo.z < hInfo.z)
				{
					result = true;

					hInfo.Copy(currentHitInfo);

					hInfoRight.CopyForDiffRay(currentHitInfoContext.rightHitInfo);
					hInfoTop.CopyForDiffRay(currentHitInfoContext.topHitInfo);
				}
			}
		}
		return result;
	}
	else
	{
		const auto ray = rayContext.cameraRay;

		if (node->bound.IntersectRay(ray))
		{
			HitInfoContext leftHitInfoContext;
			HitInfo& leftHitInfo = leftHitInfoContext.mainHitInfo;
			HitInfo& left_RightHitInfo = leftHitInfoContext.rightHitInfo;
			HitInfo& left_TopHitInfo = leftHitInfoContext.topHitInfo;

			HitInfoContext rightHitInfoContext;
			HitInfo& rightHitInfo = rightHitInfoContext.mainHitInfo;
			HitInfo& right_RightHitInfo = rightHitInfoContext.rightHitInfo;
			HitInfo& right_TopHitInfo = rightHitInfoContext.topHitInfo;

			bool hitLeft = TraceBVHNode(rayContext, leftHitInfoContext, hitSide, mesh, node->left);
			bool hitRight = TraceBVHNode(rayContext, rightHitInfoContext, hitSide, mesh, node->right);

			if (hitLeft && hitRight) {
				if (leftHitInfo.z < rightHitInfo.z)
				{
					hInfo.Copy(leftHitInfo);

					hInfoRight.CopyForDiffRay(left_RightHitInfo);
					hInfoTop.CopyForDiffRay(left_TopHitInfo);
				}
				else
				{
					hInfo.Copy(rightHitInfo);

					hInfoRight.CopyForDiffRay(right_RightHitInfo);
					hInfoTop.CopyForDiffRay(right_TopHitInfo);
				}
				return true;
			}
			else if (hitLeft)
			{
				hInfo.Copy(leftHitInfo);

				hInfoRight.CopyForDiffRay(left_RightHitInfo);
				hInfoTop.CopyForDiffRay(left_TopHitInfo);

				return true;
			}
			else if (hitRight)
			{
				hInfo.Copy(rightHitInfo);

				hInfoRight.CopyForDiffRay(right_RightHitInfo);
				hInfoTop.CopyForDiffRay(right_TopHitInfo);

				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
}

bool Model::IntersectRay(Ray const& ray, HitInfo& hInfo, int hitSide) const
{
	if (!GetBoundBox().IntersectRay(ray, BIGFLOAT))
	{
		return false;
	}

	bool result = false;
	for (int i = 0; i < meshesNum; i++)
	{
		auto& mesh = meshes[i];

		if (!mesh.aabb.IntersectRay(ray, BIGFLOAT))
		{
			continue;
		}

		HitInfo currentHitInfo;
		if (TraceBVHNode(ray, currentHitInfo, hitSide, mesh, mesh.bvh->GetRoot()))
		{
			if (currentHitInfo.z < hInfo.z)
			{
				result = true;
				hInfo.Copy(currentHitInfo);
			}
		}

		/*	for (int j = 0; j < mesh.faces.size(); j++)
			{
				auto face = mesh.faces[j];
				HitInfo currentHitInfo;
				if (IntersectRayWithFace(ray, currentHitInfo, hitSide, mesh, face))
				{
					if (currentHitInfo.z < hInfo.z)
					{
						result = true;
						hInfo.Copy(currentHitInfo);
					}
				}
			}*/
	}

	return result;
}

bool Model::IntersectRay(RayContext& rayContext, HitInfoContext& hInfoContext, int hitSide) const
{
	if (!GetBoundBox().IntersectRay(rayContext.cameraRay, BIGFLOAT))
	{
		return false;
	}

	bool result = false;

	HitInfo& hInfo = hInfoContext.mainHitInfo;
	HitInfo& hInfoRight = hInfoContext.rightHitInfo;
	HitInfo& hInfoTop = hInfoContext.topHitInfo;

	for (int i = 0; i < meshesNum; i++)
	{
		auto& mesh = meshes[i];

		if (!mesh.aabb.IntersectRay(rayContext.cameraRay, BIGFLOAT))
		{
			continue;
		}

		HitInfoContext currentHitInfoContext;
		HitInfo& currentHitInfo = currentHitInfoContext.mainHitInfo;

		if (TraceBVHNode(rayContext, currentHitInfoContext, hitSide, mesh, mesh.bvh->GetRoot()))
		{
			if (currentHitInfo.z < hInfo.z)
			{
				result = true;
				hInfo.Copy(currentHitInfo);
				hInfoRight.CopyForDiffRay(currentHitInfoContext.rightHitInfo);
				hInfoTop.CopyForDiffRay(currentHitInfoContext.topHitInfo);
			}
		}
	}
	return result;
}