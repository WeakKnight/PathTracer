#pragma once

#include "cyColor.h"

using namespace cy;

class LightComponent 
{
public:
	Color Le()
	{
		return Color(intensity, intensity, intensity);
	}

	float intensity = 0.0f;
};