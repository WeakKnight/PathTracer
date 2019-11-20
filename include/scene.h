
//-------------------------------------------------------------------------------
///
/// \file       scene.h
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    7.0
/// \date       August 21, 2019
///
/// \brief Example source for CS 6620 - University of Utah.
///
//-------------------------------------------------------------------------------

#ifndef _SCENE_H_INCLUDED_
#define _SCENE_H_INCLUDED_

//-------------------------------------------------------------------------------

#define TEXTURE_SAMPLE_COUNT 32

//-------------------------------------------------------------------------------

#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include <vector>
#include <atomic>
#include <mutex>

#include "lodepng.h"

#include "cyVector.h"
#include "cyMatrix.h"
#include "cyColor.h"

#include "objects.h"

#include "node.h"
#include "itembase.h"
#include "transformation.h"
#include "box.h"
#include "ray.h"
#include "hitinfo.h"

using namespace cy;

//-------------------------------------------------------------------------------

#ifndef Min
# define Min(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef max
# define max(a,b) ((a)>(b)?(a):(b))
#endif

#define BIGFLOAT 1.0e30f

#define RAY_DIFF_DELTA 1.0f

inline float Halton(int index, int base)
{
    float r = 0;
    float f = 1.0f / (float)base;
    for ( int i=index; i>0; i/=base ) {
        r += f * (i%base);
        f /= (float) base;
    }
    return r;
}

//-------------------------------------------------------------------------------

class Node;

#define HIT_NONE            0
#define HIT_FRONT           1
#define HIT_BACK            2
#define HIT_FRONT_AND_BACK  (HIT_FRONT|HIT_BACK)
class Material;



//-------------------------------------------------------------------------------


template <class T> class ItemList : public std::vector<T*>
{
public:
    virtual ~ItemList() { DeleteAll(); }
    void DeleteAll() { int n=(int)this->size(); for ( int i=0; i<n; i++ ) if ( this->at(i) ) delete this->at(i); }
};


template <class T> class ItemFileList
{
public:
    void Clear() { list.DeleteAll(); }
    void Append( T* item, char const *name ) { list.push_back( new FileInfo(item,name) ); }
    T* Find( char const *name ) const { int n=list.size(); for ( int i=0; i<n; i++ ) if ( list[i] && strcmp(name,list[i]->GetName())==0 ) return list[i]->GetObj(); return nullptr; }
    
private:
    class FileInfo : public ItemBase
    {
    private:
        T *item;
    public:
        FileInfo() : item(nullptr) {}
        FileInfo(T *_item, char const *name) : item(_item) { SetName(name); }
        ~FileInfo() { Delete(); }
        void Delete() { if (item) delete item; item=nullptr; }
        void SetObj(T *_item) { Delete(); item=_item; }
        T* GetObj() { return item; }
    };
    
    ItemList<FileInfo> list;
};

//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------

class Material;



typedef ItemFileList<Object> ObjFileList;

//-------------------------------------------------------------------------------

class Light : public ItemBase
{
public:
    virtual Color Illuminate(Vec3f const &p, Vec3f const &N) const=0;
    virtual Vec3f Direction (Vec3f const &p) const=0;
    virtual void  SetViewportLight(int lightID) const {}    // used for OpenGL display
	virtual Color GetFallOffIntensity(Vec3f const& x) const= 0;
};

class LightList : public ItemList<Light> {};

//-------------------------------------------------------------------------------

class Material : public ItemBase
{
public:
    // The main method that handles the shading by calling all the lights in the list.
    // ray: incoming ray,
    // hInfo: hit information for the point that is being shaded, lights: the light list,
    // bounceCount: permitted number of additional bounces for reflection and refraction.
    virtual Color Shade(RayContext const &rayContext, const HitInfoContext &hInfoContext, const LightList &lights, int bounceCount, int indirectLightBounce) const=0;
	virtual Color IndirectLightShade(const Vec3f& N, RayContext const& rayContext, const HitInfoContext& hInfoContext, const LightList& lights, int bounceCount, int indirectLightBounce) const =0;
    virtual void SetViewportMaterial(int subMtlID=0) const {}   // used for OpenGL display
	bool emissive = false;
};

class MaterialList : public ItemList<Material>
{
public:
    Material* Find( char const *name ) { int n=size(); for ( int i=0; i<n; i++ ) if ( at(i) && strcmp(name,at(i)->GetName())==0 ) return at(i); return nullptr; }
};

//-------------------------------------------------------------------------------

class Texture : public ItemBase
{
public:
    // Evaluates the color at the given uvw location.
    virtual Color Sample(Vec3f const &uvw) const=0;
    
    // Evaluates the color around the given uvw location using the derivatives duvw
    // by calling the Sample function multiple times.
    virtual Color Sample(Vec3f const &uvw, Vec3f const duvw[2], bool elliptic=true) const
    {
        Color c = Sample(uvw);
        if ( duvw[0].LengthSquared() + duvw[1].LengthSquared() == 0 ) return c;
        for ( int i=1; i<TEXTURE_SAMPLE_COUNT; i++ ) {
            float x = Halton(i,2);
            float y = Halton(i,3);
            if ( elliptic ) {
                float r = sqrtf(x)*0.5f;
                x = r*sinf(y*(float)M_PI*2);
                y = r*cosf(y*(float)M_PI*2);
            } else {
                if ( x > 0.5f ) x-=1;
                if ( y > 0.5f ) y-=1;
            }
            c += Sample( uvw + x*duvw[0] + y*duvw[1] );
        }
        return c / float(TEXTURE_SAMPLE_COUNT);
    }
    
    virtual bool SetViewportTexture() const { return false; }   // used for OpenGL display
    
protected:
    
    // Clamps the uvw values for tiling textures, such that all values fall between 0 and 1.
    static Vec3f TileClamp(Vec3f const &uvw)
    {
        Vec3f u;
        u.x = uvw.x - (int) uvw.x;
        u.y = uvw.y - (int) uvw.y;
        u.z = uvw.z - (int) uvw.z;
        if ( u.x < 0 ) u.x += 1;
        if ( u.y < 0 ) u.y += 1;
        if ( u.z < 0 ) u.z += 1;
        return u;
    }
};

typedef ItemFileList<Texture> TextureList;

//-------------------------------------------------------------------------------

// This class handles textures with texture transformations.
// The uvw values passed to the Sample methods are transformed
// using the texture transformation.
class TextureMap : public Transformation
{
public:
    TextureMap() : texture(nullptr) {}
    TextureMap(Texture *tex) : texture(tex) {}
    void SetTexture(Texture *tex) { texture = tex; }
    
    virtual Color Sample(Vec3f const &uvw) const { return texture ? texture->Sample(TransformTo(uvw)) : Color(0,0,0); }
    virtual Color Sample(Vec3f const &uvw, Vec3f const duvw[2], bool elliptic=true) const
    {
        if ( texture == nullptr ) return Color(0,0,0);
        Vec3f u = TransformTo(uvw);
        Vec3f d[2];
        d[0] = TransformTo(duvw[0]+uvw)-u;
        d[1] = TransformTo(duvw[1]+uvw)-u;
        return texture->Sample(u,d,elliptic);
    }

	virtual Vec3f SampleVector(Vec3f const& uvw) const
	{
		Color color = Sample(uvw);
		// 0.5 * normal + 0.5 = color
		// (color - 0.5) * 2
		return Vec3f(2.0f * color.r - 1.0f, 2.0f * color.g - 1.0f, 2.0f * color.b - 1.0f);
	}

	virtual Vec3f SampleVector(Vec3f const& uvw, Vec3f const duvw[2]) const
	{
		Color color = Sample(uvw, duvw);
		// 0.5 * normal + 0.5 = color
		// (color - 0.5) * 2
		return Vec3f(2.0f * color.r - 1.0f, 2.0f * color.g - 1.0f, 2.0f * color.b - 1.0f);
	}
    
    bool SetViewportTexture() const { if ( texture ) return texture->SetViewportTexture(); return false; }   // used for OpenGL display
    
private:
    Texture *texture;
};

//-------------------------------------------------------------------------------

// This class keeps a TextureMap and a color. This is useful for keeping material
// color parameters that can also be textures. If no texture is specified, it
// automatically uses the color value. Otherwise, the texture value is multiplied
// by the color value.
class TexturedColor
{
private:
    Color color;
    TextureMap *map;
public:
    TexturedColor() : color(0,0,0), map(nullptr) {}
    TexturedColor(float r, float g, float b) : color(r,g,b), map(nullptr) {}
    virtual ~TexturedColor() { if ( map ) delete map; }
    
    void SetColor(const Color &c) { color=c; }
    void SetTexture(TextureMap *m) { if ( map ) delete map; map=m; }
    
    Color GetColor() const { return color; }
    const TextureMap* GetTexture() const { return map; }
    
    Color Sample(Vec3f const &uvw) const { return ( map ) ? color*map->Sample(uvw) : color; }
    Color Sample(Vec3f const &uvw, Vec3f const duvw[2], bool elliptic=true) const { return ( map ) ? color*map->Sample(uvw,duvw,elliptic) : color; }
	
	Vec3f SphereCalculateCoord(const Vec3f& p, float reverseLength) const
	{
		return Vec3f(
			(0.5f - atan2(p.x, p.y) / (2.0f * Pi<float>())),
			(0.5f + asin(p.z * reverseLength) / (Pi<float>())),
			0.0f
		);
	};
    // Returns the color value at the given direction for environment mapping.
    Color SampleEnvironment(Vec3f const &dir) const
    {
		return Sample(SphereCalculateCoord(dir, 1.0f));
      /*  float z = asinf(-dir.z)/float(M_PI)+0.5f;
        float x = dir.x / (fabs(dir.x)+fabs(dir.y));
        float y = dir.y / (fabs(dir.x)+fabs(dir.y));
        return Sample( Vec3f(0.5f,0.5f,0.0f) + z*(x*Vec3f(0.5f,0.5f,0) + y*Vec3f(-0.5f,0.5f,0)) );*/
    }
};

//-------------------------------------------------------------------------------

class Camera
{
public:
    Vec3f pos, dir, up;
    float fov, focaldist, dof;
    int imgWidth, imgHeight;
    
    void Init()
    {
        pos.Set(0,0,0);
        dir.Set(0,0,-1);
        up.Set(0,1,0);
        fov = 40;
        focaldist = 1;
        dof = 0;
        imgWidth = 200;
        imgHeight = 150;
    }
};

//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------

class RenderImage
{
private:
	Color24* img;
	float* zbuffer;
	uint8_t* zbufferImg;
	uint8_t* sampleCount;
	uint8_t* sampleCountImg;
	uint8_t* irradComp;
	int      width, height;
	std::atomic<int> numRenderedPixels;
public:
	RenderImage() : img(nullptr), zbuffer(nullptr), zbufferImg(nullptr), sampleCount(nullptr), sampleCountImg(nullptr), irradComp(nullptr), width(0), height(0), numRenderedPixels(0) {}
	void Init(int w, int h)
	{
		width = w;
		height = h;
		if (img) delete[] img;
		{
			img = new Color24[width * height];
			for (int i = 0; i < width * height; i++)
			{
				img[i] = Color24::Black();
			}
		}
		if (zbuffer) delete[] zbuffer;
		zbuffer = new float[width * height];
		if (zbufferImg) delete[] zbufferImg;
		zbufferImg = nullptr;
		if (sampleCount) delete[] sampleCount;
		sampleCount = new uint8_t[width * height];
		if (sampleCountImg) delete[] sampleCountImg;
		sampleCountImg = nullptr;
		if (irradComp) delete[] irradComp;
		irradComp = nullptr;
		ResetNumRenderedPixels();
	}
	void AllocateIrradianceComputationImage()
	{
		if (!irradComp) irradComp = new uint8_t[width * height];
		for (int i = 0; i < width * height; i++) irradComp[i] = 0;
	}

	int      GetWidth() const { return width; }
	int      GetHeight() const { return height; }
	Color24* GetPixels() { return img; }
	float* GetZBuffer() { return zbuffer; }
	uint8_t* GetZBufferImage() { return zbufferImg; }
	uint8_t* GetSampleCount() { return sampleCount; }
	uint8_t* GetSampleCountImage() { return sampleCountImg; }
	uint8_t* GetIrradianceComputationImage() { return irradComp; }

	void ResetNumRenderedPixels() { numRenderedPixels = 0; }
	int  GetNumRenderedPixels() const { return numRenderedPixels; }
	void IncrementNumRenderPixel(int n) { numRenderedPixels += n; }
	bool IsRenderDone() const { return numRenderedPixels >= width * height; }

	void ComputeZBufferImage()
	{
		int size = width * height;
		if (zbufferImg) delete[] zbufferImg;
		zbufferImg = new uint8_t[size];

		float zmin = BIGFLOAT, zmax = 0;
		for (int i = 0; i < size; i++) {
			if (zbuffer[i] == BIGFLOAT) continue;
			if (zmin > zbuffer[i]) zmin = zbuffer[i];
			if (zmax < zbuffer[i]) zmax = zbuffer[i];
		}
		for (int i = 0; i < size; i++) {
			if (zbuffer[i] == BIGFLOAT) zbufferImg[i] = 0;
			else {
				float f = (zmax - zbuffer[i]) / (zmax - zmin);
				int c = int(f * 255);
				if (c < 0) c = 0;
				if (c > 255) c = 255;
				zbufferImg[i] = c;
			}
		}
	}

	int ComputeSampleCountImage()
	{
		int size = width * height;
		if (sampleCountImg) delete[] sampleCountImg;
		sampleCountImg = new uint8_t[size];

		uint8_t smin = 255, smax = 0;
		for (int i = 0; i < size; i++) {
			if (smin > sampleCount[i]) smin = sampleCount[i];
			if (smax < sampleCount[i]) smax = sampleCount[i];
		}
		if (smax == smin) {
			for (int i = 0; i < size; i++) sampleCountImg[i] = 0;
		}
		else {
			for (int i = 0; i < size; i++) {
				int c = (255 * (sampleCount[i] - smin)) / (smax - smin);
				if (c < 0) c = 0;
				if (c > 255) c = 255;
				sampleCountImg[i] = c;
			}
		}
		return smax;
	}

	bool SaveImage(char const* filename, uint8_t* data) const { return SavePNG(filename, data, 3); }
	bool SaveImage(char const* filename) const { return SavePNG(filename, &img[0].r, 3); }
	bool SaveZImage(char const* filename) const { return SavePNG(filename, zbufferImg, 1); }
	bool SaveSampleCountImage(char const* filename) const { return SavePNG(filename, sampleCountImg, 1); }
	bool SaveIrradianceComputationImage(char const* filename) const { return SavePNG(filename, irradComp, 1); }

private:
	bool SavePNG(char const* filename, uint8_t* data, int compCount) const
	{
		LodePNGColorType colortype;
		switch (compCount) {
		case 1: colortype = LCT_GREY; break;
		case 3: colortype = LCT_RGB;  break;
		default: return false;
		}
		unsigned int error = lodepng::encode(filename, data, width, height, colortype, 8);
		return error == 0;
	}
};
//-------------------------------------------------------------------------------

#endif
