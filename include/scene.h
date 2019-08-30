//-------------------------------------------------------------------------------
///
/// \file       scene.h 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    1.0
/// \date       August 21, 2019
///
/// \brief Example source for CS 6620 - University of Utah.
///
//-------------------------------------------------------------------------------
 
#ifndef _SCENE_H_INCLUDED_
#define _SCENE_H_INCLUDED_
 
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

#include "raytracer.h"

using namespace cy;
 
//-------------------------------------------------------------------------------
 
#define BIGFLOAT 1.0e30f
 
//-------------------------------------------------------------------------------

class Ray
{
public:
    Vec3f p, dir;
 
    Ray() {}
    Ray( Vec3f const &_p, Vec3f const &_dir) : p(_p), dir(_dir) {}
    Ray( Ray const &r) : p(r.p), dir(r.dir) {}
    void Normalize() { dir.Normalize(); }
};
 
//-------------------------------------------------------------------------------
 
class Node;
 
#define HIT_NONE           0
#define HIT_FRONT          1
#define HIT_BACK           2
#define HIT_FRONT_AND_BACK (HIT_FRONT|HIT_BACK)
 
struct HitInfo
{
    float       z;          // the distance from the ray center to the hit point
    Node const *node;       // the object node that was hit
    bool        front;      // true if the ray hits the front side, false if the ray hits the back side
 
    HitInfo() { Init(); }
    void Init() { z=BIGFLOAT; node=nullptr; front=true; }
};
 
//-------------------------------------------------------------------------------
 
class ItemBase
{
private:
    char *name;         // The name of the item
 
public:
    ItemBase() : name(nullptr) {}
    virtual ~ItemBase() { if ( name ) delete [] name; }
 
    char const * GetName() const { return name ? name : ""; }
    void SetName( char const *newName )
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
        FileInfo( T *_item, char const *name ) : item(_item) { SetName(name); }
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
    Matrix3f const & GetTransform       () const { return tm; }
    Vec3f    const & GetPosition        () const { return pos; }
    Matrix3f const & GetInverseTransform() const { return itm; }
 
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
    
// Base class for all object types
class Object
{
public:
    virtual bool IntersectRay( Ray const &ray, HitInfo &hInfo, int hitSide=HIT_FRONT ) const=0;
    virtual void ViewportDisplay() const {} // used for OpenGL display
};
 
typedef ItemFileList<Object> ObjFileList;
 
//-------------------------------------------------------------------------------
 
class Node : public ItemBase, public Transformation
{
private:
    Node **child;               // Child nodes
    int numChild;               // The number of child nodes
    Object *obj;                // Object reference (merely points to the object, but does not own the object, so it doesn't get deleted automatically)
public:
    Node() : child(nullptr), numChild(0), obj(nullptr) {}
    virtual ~Node() { DeleteAllChildNodes(); }
 
    void Init() { DeleteAllChildNodes(); obj=nullptr; SetName(nullptr); InitTransform(); } // Initialize the node deleting all child nodes
 
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
    void        SetChild( int i, Node *node ) { child[i]=node; }
    void        AppendChild( Node *node )     { SetNumChild(numChild+1,true); SetChild(numChild-1,node); }
    void        RemoveChild( int i )          { for ( int j=i; j<numChild-1; j++) child[j]=child[j+1]; SetNumChild(numChild-1); }
    void        DeleteAllChildNodes()         { for ( int i=0; i<numChild; i++ ) { child[i]->DeleteAllChildNodes(); delete child[i]; } SetNumChild(0); }
 
    // Object management
    Object const * GetNodeObj() const { return obj; }
    Object*        GetNodeObj()       { return obj; }
    void           SetNodeObj(Object *object) { obj=object; }
 
    // Transformations
    Ray ToNodeCoords( Ray const &ray ) const
    {
        Ray r;
        r.p   = TransformTo(ray.p);
        r.dir = TransformTo(ray.p + ray.dir) - r.p;
        return r;
    }
};
 
//-------------------------------------------------------------------------------
 
class Camera
{
public:
    Vec3f pos, dir, up;
    float fov;
    int imgWidth, imgHeight;
 
    void Init()
    {
        pos.Set(0,0,0);
        dir.Set(0,0,-1);
        up.Set(0,1,0);
        fov = 40;
        imgWidth = 200;
        imgHeight = 150;
    }
};
 
//-------------------------------------------------------------------------------
 
class RenderImage
{
private:
    Color24 *img;
    float   *zbuffer;
    uint8_t *zbufferImg;
    int      width, height;
    std::atomic<int> numRenderedPixels;
public:
    RenderImage() : img(nullptr), zbuffer(nullptr), zbufferImg(nullptr), width(0), height(0), numRenderedPixels(0) {}
    void Init(int w, int h)
    {
        width=w;
        height=h;
        if (img) delete [] img;
        img = new Color24[width*height];
        if (zbuffer) delete [] zbuffer;
        zbuffer = new float[width*height];
        if (zbufferImg) delete [] zbufferImg;
        zbufferImg = nullptr;
        ResetNumRenderedPixels();
    }
 
    int      GetWidth  () const { return width; }
    int      GetHeight () const { return height; }
    Color24* GetPixels ()       { return img; }
    float*   GetZBuffer()       { return zbuffer; }
    uint8_t* GetZBufferImage()  { return zbufferImg; }
 
    void ResetNumRenderedPixels ()       { numRenderedPixels=0; }
    int  GetNumRenderedPixels   () const { return numRenderedPixels; }
    void IncrementNumRenderPixel(int n)  { numRenderedPixels+=n; }
    bool IsRenderDone           () const { return numRenderedPixels >= width*height; }
 
    void ComputeZBufferImage()
    {
        int size = width * height;
        if (zbufferImg) delete [] zbufferImg;
        zbufferImg = new uint8_t[size];
 
        float zmin=BIGFLOAT, zmax=0;
        for ( int i=0; i<size; i++ ) {
            if ( zbuffer[i] == BIGFLOAT ) continue;
            if ( zmin > zbuffer[i] ) zmin = zbuffer[i];
            if ( zmax < zbuffer[i] ) zmax = zbuffer[i];
        }
        for ( int i=0; i<size; i++ ) {
            if ( zbuffer[i] == BIGFLOAT ) zbufferImg[i] = 0;
            else {
                float f = (zmax-zbuffer[i])/(zmax-zmin);
                int c = int(f * 255);
                if ( c < 0 ) c = 0;
                if ( c > 255 ) c = 255;
                zbufferImg[i] = c;
            }
        }
    }
 
    bool SaveImage ( char const *filename ) const { return SavePNG(filename,&img[0].r,3); }
    bool SaveZImage( char const *filename ) const { return SavePNG(filename,zbufferImg,1); }
 
private:
    bool SavePNG( char const *filename, uint8_t *data, int compCount ) const
    {
        LodePNGColorType colortype;
        switch( compCount ) {
            case 1: colortype = LCT_GREY; break;
            case 3: colortype = LCT_RGB;  break;
            default: return false;
        }
        unsigned int error = lodepng::encode(filename,data,width,height,colortype,8);
        return error == 0;
    }
};
//-------------------------------------------------------------------------------
 
#endif
