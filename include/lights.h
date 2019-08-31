
//-------------------------------------------------------------------------------
///
/// \file       lights.h 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    2.0
/// \date       August 21, 2019
///
/// \brief Example source for CS 6620 - University of Utah.
///
//-------------------------------------------------------------------------------
 
#ifndef _LIGHTS_H_INCLUDED_
#define _LIGHTS_H_INCLUDED_
 
#include "scene.h"
 
//-------------------------------------------------------------------------------
 
class GenLight : public Light
{
protected:
    void SetViewportParam(int lightID, ColorA ambient, ColorA intensity, Vec4f pos ) const;
};
 
//-------------------------------------------------------------------------------
 
class AmbientLight : public GenLight
{
public:
    AmbientLight() : intensity(0,0,0) {}
    virtual Color Illuminate(Vec3f const &p, Vec3f const &N) const {
        return intensity;
    }
    virtual Vec3f Direction(Vec3f const &p) const { return Vec3f(0,0,0); }
    virtual bool IsAmbient() const { return true; }
    virtual void SetViewportLight(int lightID) const { SetViewportParam(lightID,ColorA(intensity),ColorA(0.0f),Vec4f(0,0,0,1)); }
 
    void SetIntensity(Color intens) { intensity=intens; }
private:
    Color intensity;
};
 
//-------------------------------------------------------------------------------
 
class DirectLight : public GenLight
{
public:
    DirectLight() : intensity(0,0,0), direction(0,0,1) {}
    virtual Color Illuminate(Vec3f const &p, Vec3f const &N) const
    {
        return intensity;
    }
    virtual Vec3f Direction(Vec3f const &p) const { return direction; }
    virtual void SetViewportLight(int lightID) const { SetViewportParam(lightID,ColorA(0.0f),ColorA(intensity),Vec4f(-direction,0.0f)); }
 
    void SetIntensity(Color intens) { intensity=intens; }
    void SetDirection(Vec3f dir) { direction=dir.GetNormalized(); }
private:
    Color intensity;
    Vec3f direction;
};
 
//-------------------------------------------------------------------------------
 
class PointLight : public GenLight
{
public:
    PointLight() : intensity(0,0,0), position(0,0,0) {}
    virtual Color Illuminate(Vec3f const &p, Vec3f const &N) const
    {
        return intensity;
    }
    virtual Vec3f Direction(Vec3f const &p) const { return (p-position).GetNormalized(); }
    virtual void SetViewportLight(int lightID) const { SetViewportParam(lightID,ColorA(0.0f),ColorA(intensity),Vec4f(position,1.0f)); }
    void SetIntensity(Color intens) { intensity=intens; }
    void SetPosition(Vec3f pos) { position=pos; }
 
private:
    Color intensity;
    Vec3f position;
};
 
//-------------------------------------------------------------------------------
 
#endif
