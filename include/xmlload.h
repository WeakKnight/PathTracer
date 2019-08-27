//-------------------------------------------------------------------------------
///
/// \file       xmlload.cpp 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    1.0
/// \date       August 21, 2019
///
/// \brief Example source for CS 6620 - University of Utah.
///
//-------------------------------------------------------------------------------
 
#include "scene.h"
#include "objects.h"
#include "tinyxml.h"

namespace RayTracing{

//-------------------------------------------------------------------------------
 
extern Node rootNode;
extern Camera camera;
extern RenderImage renderImage;
 
//-------------------------------------------------------------------------------
 
#ifdef WIN32
#define COMPARE(a,b) (_stricmp(a,b)==0)
#else
#define COMPARE(a,b) (strcasecmp(a,b)==0)
#endif
 
//-------------------------------------------------------------------------------
 
void LoadScene    ( TiXmlElement *element );
void LoadNode     ( Node *node, TiXmlElement *element, int level=0 );
void LoadTransform( Transformation *trans, TiXmlElement *element, int level );
void ReadVector   ( TiXmlElement *element, Vec3f &v );
void ReadFloat    ( TiXmlElement *element, float  &f, char const *name="value" );
 
//-------------------------------------------------------------------------------
 
int LoadScene( char const *filename )
{
    TiXmlDocument doc(filename);
    if ( ! doc.LoadFile() ) {
        printf("Failed to load the file \"%s\"\n", filename);
        return 0;
    }
 
    TiXmlElement *xml = doc.FirstChildElement("xml");
    if ( ! xml ) {
        printf("No \"xml\" tag found.\n");
        return 0;
    }
 
    TiXmlElement *scene = xml->FirstChildElement("scene");
    if ( ! scene ) {
        printf("No \"scene\" tag found.\n");
        return 0;
    }
 
    TiXmlElement *cam = xml->FirstChildElement("camera");
    if ( ! cam ) {
        printf("No \"camera\" tag found.\n");
        return 0;
    }
 
    rootNode.Init();
    LoadScene( scene );
 
    // Load Camera
    camera.Init();
    camera.dir += camera.pos;
    TiXmlElement *camChild = cam->FirstChildElement();
    while ( camChild ) {
        if      ( COMPARE( camChild->Value(), "position"  ) ) ReadVector(camChild,camera.pos);
        else if ( COMPARE( camChild->Value(), "target"    ) ) ReadVector(camChild,camera.dir);
        else if ( COMPARE( camChild->Value(), "up"        ) ) ReadVector(camChild,camera.up);
        else if ( COMPARE( camChild->Value(), "fov"       ) ) ReadFloat (camChild,camera.fov);
        else if ( COMPARE( camChild->Value(), "width"     ) ) camChild->QueryIntAttribute("value", &camera.imgWidth);
        else if ( COMPARE( camChild->Value(), "height"    ) ) camChild->QueryIntAttribute("value", &camera.imgHeight);
        camChild = camChild->NextSiblingElement();
    }
    camera.dir -= camera.pos;
    camera.dir.Normalize();
    Vec3f x = camera.dir ^ camera.up;
    camera.up = (x ^ camera.dir).GetNormalized();
 
    renderImage.Init( camera.imgWidth, camera.imgHeight );
 
    return 1;
}
 
//-------------------------------------------------------------------------------
 
void PrintIndent( int level ) { for ( int i=0; i<level; i++) printf("   "); }
 
//-------------------------------------------------------------------------------
 
void LoadScene( TiXmlElement *element )
{
    for ( TiXmlElement *child = element->FirstChildElement(); child!=nullptr; child = child->NextSiblingElement() ) {
 
        if ( COMPARE( child->Value(), "object" ) ) {
            LoadNode( &rootNode, child );
        }
    }
}
 
//-------------------------------------------------------------------------------
 
void LoadNode( Node *parent, TiXmlElement *element, int level )
{
    Node *node = new Node;
    parent->AppendChild(node);
 
    // name
    char const *name = element->Attribute("name");
    node->SetName(name);
    PrintIndent(level);
    printf("object [");
    if ( name ) printf("%s",name);
    printf("]");
 
    // type
    char const *type = element->Attribute("type");
    if ( type ) {
        if ( COMPARE(type,"sphere") ) {
            node->SetNodeObj( &theSphere );
            printf(" - Sphere");
        } else {
            printf(" - UNKNOWN TYPE");
        }
    }
 
 
    printf("\n");
 
 
    for ( TiXmlElement *child = element->FirstChildElement(); child!=nullptr; child = child->NextSiblingElement() ) {
        if ( COMPARE( child->Value(), "object" ) ) {
            LoadNode(node,child,level+1);
        }
    }
    LoadTransform( node, element, level );
 
}
 
//-------------------------------------------------------------------------------
 
void LoadTransform( Transformation *trans, TiXmlElement *element, int level )
{
    for ( TiXmlElement *child = element->FirstChildElement(); child!=nullptr; child = child->NextSiblingElement() ) {
        if ( COMPARE( child->Value(), "scale" ) ) {
            Vec3f s(1,1,1);
            ReadVector( child, s );
            trans->Scale(s.x,s.y,s.z);
            PrintIndent(level);
            printf("   scale %f %f %f\n",s.x,s.y,s.z);
        } else if ( COMPARE( child->Value(), "rotate" ) ) {
            Vec3f s(0,0,0);
            ReadVector( child, s );
            s.Normalize();
            float a;
            ReadFloat(child,a,"angle");
            trans->Rotate(s,a);
            PrintIndent(level);
            printf("   rotate %f degrees around %f %f %f\n", a, s.x, s.y, s.z);
        } else if ( COMPARE( child->Value(), "translate" ) ) {
            Vec3f t(0,0,0);
            ReadVector(child,t);
            trans->Translate(t);
            PrintIndent(level);
            printf("   translate %f %f %f\n",t.x,t.y,t.z);
        }
    }
}
 
//-------------------------------------------------------------------------------
 
void ReadVector( TiXmlElement *element, Vec3f &v )
{
    double x = (double) v.x;
    double y = (double) v.y;
    double z = (double) v.z;
    element->QueryDoubleAttribute( "x", &x );
    element->QueryDoubleAttribute( "y", &y );
    element->QueryDoubleAttribute( "z", &z );
    v.x = (float) x;
    v.y = (float) y;
    v.z = (float) z;
 
    float f=1;
    ReadFloat( element, f );
    v *= f;
}
 
//-------------------------------------------------------------------------------
 
void ReadFloat ( TiXmlElement *element, float &f, char const *name )
{
    double d = (double) f;
    element->QueryDoubleAttribute( name, &d );
    f = (float) d;
}
 
//-------------------------------------------------------------------------------
}
