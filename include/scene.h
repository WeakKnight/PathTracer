
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

#include "lodepng.h"

#include "cyVector.h"
#include "cyMatrix.h"
#include "cyColor.h"
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
class Ray
{
public:
    Vec3f p, dir;
    
    Ray() {}
    Ray( Vec3f const &_p, Vec3f const &_dir ) : p(_p), dir(_dir) {}
    Ray( Ray const &r ) : p(r.p), dir(r.dir) {}
    void Normalize() { dir.Normalize(); }
};

//-------------------------------------------------------------------------------

class Box
{
public:
    Vec3f pmin, pmax;
    
    // Constructors
    Box() { Init(); }
    Box(Vec3f const &_pmin, Vec3f const &_pmax) : pmin(_pmin), pmax(_pmax) {}
    Box(float xmin, float ymin, float zmin, float xmax, float ymax, float zmax ) : pmin(xmin,ymin,zmin), pmax(xmax,ymax,zmax) {}
    Box(float const *dim) : pmin(dim[0],dim[1],dim[2]), pmax(dim[3],dim[4],dim[5]) {}
    
    // Initializes the box, such that there exists no point inside the box (i.e. it is empty).
    void Init() { pmin.Set(BIGFLOAT,BIGFLOAT,BIGFLOAT); pmax.Set(-BIGFLOAT,-BIGFLOAT,-BIGFLOAT); }
    
    // Returns true if the box is empty; otherwise, returns false.
    bool IsEmpty() const { return pmin.x>pmax.x || pmin.y>pmax.y || pmin.z>pmax.z; }
    
    // Returns one of the 8 corner point of the box in the following order:
    // 0:(x_min,y_min,z_min), 1:(x_max,y_min,z_min)
    // 2:(x_min,y_max,z_min), 3:(x_max,y_max,z_min)
    // 4:(x_min,y_min,z_max), 5:(x_max,y_min,z_max)
    // 6:(x_min,y_max,z_max), 7:(x_max,y_max,z_max)
    Vec3f Corner( int i ) const // 8 corners of the box
    {
        Vec3f p;
        p.x = (i & 1) ? pmax.x : pmin.x;
        p.y = (i & 2) ? pmax.y : pmin.y;
        p.z = (i & 4) ? pmax.z : pmin.z;
        return p;
    }
    
    // Enlarges the box such that it includes the given point p.
    void operator += (Vec3f const &p)
    {
        for ( int i=0; i<3; i++ ) {
            if ( pmin[i] > p[i] ) pmin[i] = p[i];
            if ( pmax[i] < p[i] ) pmax[i] = p[i];
        }
    }
    
    // Enlarges the box such that it includes the given box b.
    void operator += (const Box &b)
    {
        for ( int i=0; i<3; i++ ) {
            if ( pmin[i] > b.pmin[i] ) pmin[i] = b.pmin[i];
            if ( pmax[i] < b.pmax[i] ) pmax[i] = b.pmax[i];
        }
    }
    
    // Returns true if the point is inside the box; otherwise, returns false.
    bool IsInside(Vec3f const &p) const { for ( int i=0; i<3; i++ ) if ( pmin[i] > p[i] || pmax[i] < p[i] ) return false; return true; }
    
    // Returns true if the ray intersects with the box for any parameter that is smaller than t_max; otherwise, returns false.
    bool IntersectRay(Ray const &r, float t_max) const;
};

//-------------------------------------------------------------------------------

class Node;

#define HIT_NONE            0
#define HIT_FRONT           1
#define HIT_BACK            2
#define HIT_FRONT_AND_BACK  (HIT_FRONT|HIT_BACK)

struct HitInfo
{
    float       z;      // the distance from the ray center to the hit point
    Vec3f       p;      // position of the hit point
    Vec3f       N;      // surface normal at the hit point
	Vec3f	    Tangent;
	Vec3f       Bitangent;
    Vec3f       uvw;    // texture coordinate at the hit point
    Vec3f       duvw[2];// derivatives of the texture coordinate
    Node const *node;   // the object node that was hit
    bool        front;  // true if the ray hits the front side, false if the ray hits the back side
    int         mtlID;  // sub-material index
    
    HitInfo() { Init(); }
    void Init() { z=BIGFLOAT; node=nullptr; front=true; uvw.Set(0.5f,0.5f,0.5f); duvw[0].Zero(); duvw[1].Zero(); mtlID=0; }
	
	void Copy(const HitInfo& other)
	{
		z = other.z;
		p = other.p;
		N = other.N;
		Tangent = other.Tangent;
		Bitangent = other.Bitangent;
		uvw = other.uvw;
		duvw[0] = other.duvw[0];
		duvw[1] = other.duvw[1];
		node = other.node;
		front = other.front;
		mtlID = other.mtlID;
	}

	void CopyForDiffRay(const HitInfo& other)
	{
		z = other.z;
		p = other.p;
		N = other.N;
	}
};

struct HitInfoContext
{
    HitInfoContext()
    {
        mainHitInfo.Init();
        rightHitInfo.Init();
        topHitInfo.Init();
    }
    
    HitInfo mainHitInfo;
    HitInfo rightHitInfo;
    HitInfo topHitInfo;

	bool screenRay = false;
	int screenX = -1;
	int screenY = -1;

	void SetAsScreenInfo(int x, int y)
	{
		screenRay = true;
		screenX = x;
		screenY = y;
	}
};

struct RayContext
{
    Ray cameraRay;
    // Ray Differential
    Ray rightRay;
    Ray topRay;
    float delta;
    bool hasDiff = true;
    
    RayContext() {}
    RayContext( RayContext const &r ) :
    cameraRay(r.cameraRay),
    rightRay(r.rightRay),
    topRay(r.topRay),
    delta(r.delta)
    {
        
    }
};

//-------------------------------------------------------------------------------

class ItemBase
{
private:
    char *name;                 // The name of the item
    
public:
    ItemBase() : name(nullptr) {}
    virtual ~ItemBase() { if ( name ) delete [] name; }
    
    char const* GetName() const { return name ? name : ""; }
    void SetName(char const *newName)
    {
        if ( name ) delete [] name;
        if ( newName ) {
            int n = strlen(newName);
            name = new char[n+1];
            for ( int i=0; i<n; i++ ) name[i] = newName[i];
            name[n] = '\0';
        } else { name = nullptr; }
    }
};

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

class Transformation
{
private:
    Matrix3f tm;            // Transformation matrix to the local space
    Vec3f    pos;           // Translation part of the transformation matrix
    mutable Matrix3f itm;   // Inverse of the transformation matrix (cached)
public:
    Transformation() : pos(0,0,0) { tm.SetIdentity(); itm.SetIdentity(); }
    Matrix3f const& GetTransform       () const { return tm; }
    Vec3f    const& GetPosition        () const { return pos; }
    Matrix3f const& GetInverseTransform() const { return itm; }
    
    Vec3f TransformTo  ( Vec3f const &p ) const { return itm * (p - pos); } // Transform to the local coordinate system
    Vec3f TransformFrom( Vec3f const &p ) const { return tm*p + pos; }  // Transform from the local coordinate system
    
    // Transforms a vector to the local coordinate system (same as multiplication with the inverse transpose of the transformation)
    Vec3f VectorTransformTo( Vec3f const &dir ) const { return TransposeMult(tm,dir); }
    
    // Transforms a vector from the local coordinate system (same as multiplication with the inverse transpose of the transformation)
    Vec3f VectorTransformFrom( Vec3f const &dir ) const { return TransposeMult(itm,dir); }
    
    void Translate( Vec3f const &p ) { pos+=p; }
    void Rotate   ( Vec3f const &axis, float degrees ) { Matrix3f m; m.SetRotation(axis,degrees*(float)M_PI/180.0f); Transform(m); }
    void Scale    ( float sx, float sy, float sz )     { Matrix3f m; m.Zero(); m[0]=sx; m[4]=sy; m[8]=sz; Transform(m); }
    void Transform( Matrix3f const &m ) { tm=m*tm; pos=m*pos; tm.GetInverse(itm); }
    
    void InitTransform() { pos.Zero(); tm.SetIdentity(); itm.SetIdentity(); }
    
private:
    // Multiplies the given vector with the transpose of the given matrix
    static Vec3f TransposeMult( Matrix3f const &m, Vec3f const &dir )
    {
        Vec3f d;
        d.x = m.GetColumn(0) % dir;
        d.y = m.GetColumn(1) % dir;
        d.z = m.GetColumn(2) % dir;
        return d;
    }
};

//-------------------------------------------------------------------------------

class Material;

// Base class for all object types
class Object
{
public:
    virtual bool IntersectRay( Ray const &ray, HitInfo &hInfo, int hitSide=HIT_FRONT ) const=0;
    virtual bool IntersectRay(RayContext &rayContext, HitInfoContext& hInfoContext, int hitSide= HIT_FRONT) const=0;
    virtual Box  GetBoundBox() const=0;
    virtual void ViewportDisplay(const Material *mtl) const {}  // used for OpenGL display
};

typedef ItemFileList<Object> ObjFileList;

//-------------------------------------------------------------------------------

class Light : public ItemBase
{
public:
    virtual Color Illuminate(Vec3f const &p, Vec3f const &N) const=0;
    virtual Vec3f Direction (Vec3f const &p) const=0;
    virtual bool  IsAmbient () const { return false; }
    virtual void  SetViewportLight(int lightID) const {}    // used for OpenGL display
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
	virtual Color IndirectLightShade(RayContext const& rayContext, const HitInfoContext& hInfoContext, const LightList& lights, int bounceCount, int indirectLightBounce) const =0;
    virtual void SetViewportMaterial(int subMtlID=0) const {}   // used for OpenGL display
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
	
	Vec3f SphereCalculateCoord(Vec3f& p, float reverseLength) const
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
		return Sample(SphereCalculateCoord(dir * 1.0f, 1.0f));
      /*  float z = asinf(-dir.z)/float(M_PI)+0.5f;
        float x = dir.x / (fabs(dir.x)+fabs(dir.y));
        float y = dir.y / (fabs(dir.x)+fabs(dir.y));
        return Sample( Vec3f(0.5f,0.5f,0.0f) + z*(x*Vec3f(0.5f,0.5f,0) + y*Vec3f(-0.5f,0.5f,0)) );*/
    }
};

//-------------------------------------------------------------------------------

class Node : public ItemBase, public Transformation
{
private:
    Node **child;               // Child nodes
    int numChild;               // The number of child nodes
    Object *obj;                // Object reference (merely points to the object, but does not own the object, so it doesn't get deleted automatically)
    Material *mtl;              // Material used for shading the object
    Box childBoundBox;          // Bounding box of the child nodes, which does not include the object of this node, but includes the objects of the child nodes
	Node* parent;
public:
    Node() : child(nullptr), numChild(0), obj(nullptr), mtl(nullptr) {}
    virtual ~Node() { DeleteAllChildNodes(); }
    
    void Init() { DeleteAllChildNodes(); obj=nullptr; mtl=nullptr; childBoundBox.Init(); SetName(nullptr); InitTransform(); } // Initialize the node deleting all child nodes
    
	// 
	Node* GetParent()
	{
		return parent;
	}
	void SetParent(Node* node)
	{
		parent = node;
	}

    // Hierarchy management
    int  GetNumChild() const { return numChild; }
    void SetNumChild(int n, int keepOld=false)
    {
        if ( n < 0 ) n=0;    // just to be sure
        Node **nc = nullptr;    // new child pointer
        if ( n > 0 ) nc = new Node*[n];
        for ( int i=0; i<n; i++ ) nc[i] = nullptr;
        if ( keepOld ) {
            int sn = Min(n,numChild);
            for ( int i=0; i<sn; i++ ) nc[i] = child[i];
        }
        if ( child ) delete [] child;
        child = nc;
        numChild = n;
    }
    Node const* GetChild( int i ) const       { return child[i]; }
    Node*       GetChild( int i )             { return child[i]; }
    
	void        SetChild( int i, Node *node ) 
	{ 
		child[i]=node; 
	}
    
	void        AppendChild( Node *node )     
	{ 
		SetNumChild(numChild+1,true); 
		SetChild(numChild-1,node); 
		node->SetParent(this);
	}
    
	void        RemoveChild( int i )          { for ( int j=i; j<numChild-1; j++) child[j]=child[j+1]; SetNumChild(numChild-1); }
    void        DeleteAllChildNodes()         { for ( int i=0; i<numChild; i++ ) { child[i]->DeleteAllChildNodes(); delete child[i]; } SetNumChild(0); }
    
    // Bounding Box
    const Box& ComputeChildBoundBox()
    {
        childBoundBox.Init();
        for ( int i=0; i<numChild; i++ ) {
            Box childBox = child[i]->ComputeChildBoundBox();
            Object *cobj = child[i]->GetNodeObj();
            if ( cobj ) childBox += cobj->GetBoundBox();
            if ( ! childBox.IsEmpty() ) {
                // transform the box from child coordinates
                for ( int j=0; j<8; j++ ) childBoundBox += child[i]->TransformFrom( childBox.Corner(j) );
            }
        }
        return childBoundBox;
    }
    const Box& GetChildBoundBox() const { return childBoundBox; }
    
    // Object management
    Object const * GetNodeObj() const { return obj; }
    Object*        GetNodeObj()       { return obj; }
    void           SetNodeObj(Object *object) { obj=object; }
    
    // Material management
    const Material* GetMaterial() const 
	{
		if (mtl == nullptr && parent != nullptr)
		{
			return parent->GetMaterial();
		}

		return mtl; 
	}
    void            SetMaterial(Material *material) { mtl=material; }
    
    // Transformations
    Ray ToNodeCoords( Ray const &ray ) const
    {
        Ray r;
        r.p   = TransformTo(ray.p);
        r.dir = TransformTo(ray.p + ray.dir) - r.p;
        return r;
    }
    
    RayContext ToNodeCoords(RayContext const & rayContext) const
    {
        RayContext result;
        
        result.cameraRay = ToNodeCoords(rayContext.cameraRay);
        result.rightRay = ToNodeCoords(rayContext.rightRay);
        result.topRay = ToNodeCoords(rayContext.topRay);
        result.delta = rayContext.delta;
        result.hasDiff = rayContext.hasDiff;
        
        return result;
    }
    
    void FromNodeCoords( HitInfo &hInfo ) const
    {
        hInfo.p = TransformFrom(hInfo.p);
        hInfo.N = VectorTransformFrom(hInfo.N).GetNormalized();
		hInfo.Tangent = VectorTransformFrom(hInfo.Tangent).GetNormalized();
		hInfo.Bitangent = VectorTransformFrom(hInfo.Bitangent).GetNormalized();
    }
    
    void FromNodeCoords( HitInfoContext &hInfoContext ) const
    {
        FromNodeCoords(hInfoContext.mainHitInfo);
        FromNodeCoords(hInfoContext.rightHitInfo);
        // assert(!isnan(hInfoContext.rightHitInfo.N.x));
        FromNodeCoords(hInfoContext.topHitInfo);
        // assert(!isnan(hInfoContext.topHitInfo.N.x));
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
		img = new Color24[width * height];
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
