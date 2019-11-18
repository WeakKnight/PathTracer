
//-------------------------------------------------------------------------------
///
/// \file       lights.h 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    10.0
/// \date       August 21, 2019
///
/// \brief Example source for CS 6620 - University of Utah.
///
//-------------------------------------------------------------------------------
 
#ifndef _LIGHTS_H_INCLUDED_
#define _LIGHTS_H_INCLUDED_
 
#include "scene.h"
#include "sampler.h"
 
//-------------------------------------------------------------------------------
 
class GenLight : public Light
{
protected:
    void SetViewportParam(int lightID, ColorA ambient, ColorA intensity, Vec4f pos ) const;
    static float Shadow(Ray ray, float t_max=BIGFLOAT);
	static QuasyMonteCarloCircleSampler* CircleAreaLightSampler;
};
 
//-------------------------------------------------------------------------------
 
class DirectLight : public GenLight
{
public:
    DirectLight() : intensity(0,0,0), direction(0,0,1) {}
    virtual Color Illuminate(Vec3f const &p, Vec3f const &N) const { return Shadow(Ray(p,-direction)) * intensity; }
    virtual Vec3f Direction(Vec3f const &p) const { return direction; }
    virtual void SetViewportLight(int lightID) const { SetViewportParam(lightID,ColorA(0.0f),ColorA(intensity),Vec4f(-direction,0.0f)); }
 
    void SetIntensity(Color intens) { intensity=intens; }
    void SetDirection(Vec3f dir) { direction=dir.GetNormalized(); }

	virtual Color GetFallOffIntensity(Vec3f const& x) const
	{
		return intensity;
	}
private:
    Color intensity;
    Vec3f direction;
};
 
//-------------------------------------------------------------------------------
 
class PointLight : public GenLight
{
public:
    PointLight() : intensity(0,0,0), position(0,0,0), size(0) {}
    virtual Color Illuminate(Vec3f const &p, Vec3f const &N) const;
	virtual Vec3f Direction(Vec3f const& p) const;
    virtual void SetViewportLight(int lightID) const { SetViewportParam(lightID,ColorA(0.0f),ColorA(intensity),Vec4f(position,1.0f)); }
    void SetIntensity(Color intens) { intensity=intens; }
    void SetPosition(Vec3f pos) { position=pos; }
    void SetSize(float s) { size=s; }
	virtual Color GetFallOffIntensity(Vec3f const& x) const;
private:
    Color intensity;
    Vec3f position;
    float size;
};
 
//-------------------------------------------------------------------------------
 
#endif