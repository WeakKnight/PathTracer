#pragma once

#include "scene.h"

class ObjMaterial : public Material
{
public:
	virtual Color Shade(RayContext const& rayContext, const HitInfoContext& hInfoContext, const LightList& lights, int bounceCount) const;

	virtual void SetViewportMaterial(int subMtlID = 0) const 
	{
	}  
};