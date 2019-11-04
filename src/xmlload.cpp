
//-------------------------------------------------------------------------------
///
/// \file       xmlload.cpp 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    10.0
/// \date       August 21, 2019
///
/// \brief Example source for CS 6620 - University of Utah.
///
//-------------------------------------------------------------------------------
 
#include "scene.h"
#include "objects.h"
#include "materials.h"
#include "lights.h"
#include "texture.h"
#include "tinyxml/tinyxml.h"
#include "xmlload.h"
#include <unordered_map>

//-------------------------------------------------------------------------------
 
extern Node rootNode;
extern Camera camera;
extern RenderImage renderImage;
extern MaterialList materials;
extern LightList lights;
extern ObjFileList objList;
extern TexturedColor background;
extern TexturedColor environment;
extern TextureList textureList;
//-------------------------------------------------------------------------------
 
#ifdef WIN32
#define COMPARE(a,b) (_stricmp(a,b)==0)
#else
#define COMPARE(a,b) (strcasecmp(a,b)==0)
#endif
 
//-------------------------------------------------------------------------------
 
void LoadScene(TiXmlElement *element);
void LoadNode(Node *node, TiXmlElement *element, int level=0);
void LoadTransform( Transformation *trans, TiXmlElement *element, int level );
void LoadMaterial(TiXmlElement *element);
void LoadLight(TiXmlElement *element);
void ReadVector(TiXmlElement *element, Vec3f &v);
void ReadColor (TiXmlElement *element, Color  &c);
void ReadFloat (TiXmlElement *element, float  &f, char const *name="value");
TextureMap* ReadTexture(TiXmlElement *element);
Texture* ReadTexture(char const *filename);
 
//-------------------------------------------------------------------------------
 
struct NodeMtl
{
    Node *node;
    char const *mtlName;
};
 
std::vector<NodeMtl> nodeMtlList;
 
//-------------------------------------------------------------------------------
 
int LoadScene(char const *filename)
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
 
    nodeMtlList.clear();
    rootNode.Init();
    materials.DeleteAll();
    lights.DeleteAll();
    objList.Clear();
    textureList.Clear();
    LoadScene( scene );
 
    rootNode.ComputeChildBoundBox();
 
    // Assign materials
    int numNodes = nodeMtlList.size();
    for ( int i=0; i<numNodes; i++ ) {
        Material *mtl = materials.Find( nodeMtlList[i].mtlName );
        if ( mtl ) nodeMtlList[i].node->SetMaterial(mtl);
    }
    nodeMtlList.clear();
 
    // Load Camera
    camera.Init();
    camera.dir += camera.pos;
    TiXmlElement *camChild = cam->FirstChildElement();
    while ( camChild ) {
        if      ( COMPARE( camChild->Value(), "position"  ) ) ReadVector(camChild,camera.pos);
        else if ( COMPARE( camChild->Value(), "target"    ) ) ReadVector(camChild,camera.dir);
        else if ( COMPARE( camChild->Value(), "up"        ) ) ReadVector(camChild,camera.up);
        else if ( COMPARE( camChild->Value(), "fov"       ) ) ReadFloat (camChild,camera.fov);
        else if ( COMPARE( camChild->Value(), "focaldist" ) ) ReadFloat (camChild,camera.focaldist);
        else if ( COMPARE( camChild->Value(), "dof"       ) ) ReadFloat (camChild,camera.dof);
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
 
void PrintIndent(int level) { for ( int i=0; i<level; i++) printf("   "); }
 
//-------------------------------------------------------------------------------
 
void LoadScene(TiXmlElement *element)
{
    for ( TiXmlElement *child = element->FirstChildElement(); child!=nullptr; child = child->NextSiblingElement() ) {
 
        if ( COMPARE( child->Value(), "background" ) ) {
            Color c(1,1,1);
            ReadColor( child, c );
            background.SetColor(c);
            printf("Background %f %f %f\n",c.r,c.g,c.b);
            background.SetTexture( ReadTexture(child) );
        } else if ( COMPARE( child->Value(), "environment" ) ) {
            Color c(1,1,1);
            ReadColor( child, c );
            environment.SetColor(c);
            printf("Environment %f %f %f\n",c.r,c.g,c.b);
            environment.SetTexture( ReadTexture(child) );
        } else if ( COMPARE( child->Value(), "object" ) ) {
            LoadNode( &rootNode, child );
        } else if ( COMPARE( child->Value(), "material" ) ) {
            LoadMaterial( child );
        } else if ( COMPARE( child->Value(), "light" ) ) {
            LoadLight( child );
        }
    }
}
 
//-------------------------------------------------------------------------------
 
void LoadNode(Node *parent, TiXmlElement *element, int level)
{
    Node *node = new Node;
    parent->AppendChild(node);
 
    // name
    char const* name = element->Attribute("name");
    node->SetName(name);
    PrintIndent(level);
    printf("object [");
    if ( name ) printf("%s",name);
    printf("]");
 
    // material
    char const* mtlName = element->Attribute("material");
    if ( mtlName ) {
        printf(" <%s>", mtlName);
        NodeMtl nm;
        nm.node = node;
        nm.mtlName = mtlName;
        nodeMtlList.push_back(nm);
    }
 
    // type
    char const* type = element->Attribute("type");
    if ( type ) {
        if ( COMPARE(type,"sphere") ) {
			Sphere* sphere = new Sphere;
            node->SetNodeObj( sphere);
            printf(" - Sphere");
        } else if ( COMPARE(type,"plane") ) {
			Plane* plane = new Plane;
            node->SetNodeObj( plane);
            printf(" - Plane");
        } else if ( COMPARE(type,"obj") ) {
            printf(" - OBJ");
            Object *obj = objList.Find(name);
            if ( obj == nullptr ) { // object is not on the list, so we should load it now
                TriObj *tobj = new TriObj;
                if ( ! tobj->Load( name, mtlName==nullptr ) ) {
                    printf(" -- ERROR: Cannot load file \"%s.\"", name);
                    delete tobj;
                } else {
                    objList.Append(tobj,name);  // add to the list
                    obj = tobj;
                    // generate multi-material
                    if ( tobj->NM() > 0 ) {
                        if ( materials.Find(name) == nullptr ) {
                            MultiMtl *mm = new MultiMtl;
                            for ( unsigned int i=0; i<tobj->NM(); i++ ) {
                                MtlBlinn *m = new MtlBlinn;
                                const cyTriMesh::Mtl &mtl = tobj->M(i);
                                m->SetDiffuse( Color(mtl.Kd) );
                                m->SetSpecular( Color(mtl.Ks) );
                                m->SetGlossiness( mtl.Ns );
                                m->SetRefractionIndex( mtl.Ni );
                                if ( mtl.map_Kd.data != nullptr ) m->SetDiffuseTexture( new TextureMap(ReadTexture(mtl.map_Kd.data)) );
                                if ( mtl.map_Ks.data != nullptr ) m->SetDiffuseTexture( new TextureMap(ReadTexture(mtl.map_Ks.data)) );
                                if ( mtl.illum > 2 && mtl.illum <= 7 ) {
                                    m->SetReflection( Color(mtl.Ks) );
                                    if ( mtl.map_Ks.data != nullptr ) m->SetReflectionTexture( new TextureMap(ReadTexture(mtl.map_Ks.data)) );
                                    float gloss = acosf(powf(2,1/mtl.Ns));
                                    m->SetReflectionGlossiness(gloss);
                                    if ( mtl.illum >= 6 ) {
                                        m->SetRefraction( 1 - Color(mtl.Tf) );
                                        m->SetRefractionGlossiness(gloss);
                                    }
                                }
                                mm->AppendMaterial(m);
                            }
                            mm->SetName(name);
                            materials.push_back(mm);
                            NodeMtl nm;
                            nm.node = node;
                            nm.mtlName = name;
                            nodeMtlList.push_back(nm);
                        }
                    }
                }
            }
            node->SetNodeObj( obj );
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
 
void LoadMaterial(TiXmlElement *element)
{
    Material *mtl = nullptr;
 
    // name
    char const* name = element->Attribute("name");
    printf("Material [");
    if ( name ) printf("%s",name);
    printf("]");
 
    // type
    char const* type = element->Attribute("type");
    if ( type ) {
        if ( COMPARE(type,"blinn") ) {
            printf(" - Blinn\n");
            MtlBlinn *m = new MtlBlinn();
            mtl = m;
            for ( TiXmlElement *child = element->FirstChildElement(); child!=nullptr; child = child->NextSiblingElement() ) {
                Color c(1,1,1);
                float f=1;
                if ( COMPARE( child->Value(), "diffuse" ) ) {
                    ReadColor( child, c );
                    m->SetDiffuse(c);
                    printf("   diffuse %f %f %f\n",c.r,c.g,c.b);
                    m->SetDiffuseTexture( ReadTexture(child) );
                }
				else if (COMPARE(child->Value(), "normal")) {
					m->SetNormalTexture(ReadTexture(child));
				}
				else if (COMPARE(child->Value(), "ao")) {
					m->SetAOTexture(ReadTexture(child));
				}
				else if ( COMPARE( child->Value(), "specular" ) ) {
                    ReadColor( child, c );
                    m->SetSpecular(c);
                    printf("   specular %f %f %f\n",c.r,c.g,c.b);
                    m->SetSpecularTexture( ReadTexture(child) );
                } else if ( COMPARE( child->Value(), "glossiness" ) ) {
                    ReadFloat( child, f );
                    m->SetGlossiness(f);
                    printf("   glossiness %f\n",f);
                } else if ( COMPARE( child->Value(), "reflection" ) ) {
                    ReadColor( child, c );
                    m->SetReflection(c);
                    printf("   reflection %f %f %f\n",c.r,c.g,c.b);
                    m->SetReflectionTexture( ReadTexture(child) );
                    f = 0;
                    ReadFloat( child, f, "glossiness" );
                    m->SetReflectionGlossiness(f);
                    if ( f > 0 ) printf(" (glossiness %f)",f);
                    printf("\n");
                } else if ( COMPARE( child->Value(), "refraction" ) ) {
                    ReadColor( child, c );
                    m->SetRefraction(c);
                    ReadFloat( child, f, "index" );
                    m->SetRefractionIndex(f);
                    printf("   refraction %f %f %f (index %f)\n",c.r,c.g,c.b,f);
                    m->SetRefractionTexture( ReadTexture(child) );
                    f = 0;
                    ReadFloat( child, f, "glossiness" );
                    m->SetRefractionGlossiness(f);
                    if ( f > 0 ) printf(" (glossiness %f)",f);
                    printf("\n");
                } else if ( COMPARE( child->Value(), "absorption" ) ) {
                    ReadColor( child, c );
                    m->SetAbsorption(c);
                    printf("   absorption %f %f %f\n",c.r,c.g,c.b);
                }
            }
        } else {
            printf(" - UNKNOWN\n");
        }
    }
 
    if ( mtl ) {
        mtl->SetName(name);
        materials.push_back(mtl);
    }
}
 
//-------------------------------------------------------------------------------
 
void LoadLight(TiXmlElement *element)
{
    Light *light = nullptr;
 
    // name
    char const* name = element->Attribute("name");
    printf("Light [");
    if ( name ) printf("%s",name);
    printf("]");
 
    // type
    char const* type = element->Attribute("type");
    if ( type ) {
        if ( COMPARE(type,"ambient") ) {
            printf(" - Ambient\n");
            AmbientLight *l = new AmbientLight();
            light = l;
            for ( TiXmlElement *child = element->FirstChildElement(); child!=nullptr; child = child->NextSiblingElement() ) {
                if ( COMPARE( child->Value(), "intensity" ) ) {
                    Color c(1,1,1);
                    ReadColor( child, c );
                    l->SetIntensity(c);
                    printf("   intensity %f %f %f\n",c.r,c.g,c.b);
                }
            }
        } else if ( COMPARE(type,"direct") ) {
            printf(" - Direct\n");
            DirectLight *l = new DirectLight();
            light = l;
            for ( TiXmlElement *child = element->FirstChildElement(); child!=nullptr; child = child->NextSiblingElement() ) {
                if ( COMPARE( child->Value(), "intensity" ) ) {
                    Color c(1,1,1);
                    ReadColor( child, c );
                    l->SetIntensity(c);
                    printf("   intensity %f %f %f\n",c.r,c.g,c.b);
                } else if ( COMPARE( child->Value(), "direction" ) ) {
                    Vec3f v(1,1,1);
                    ReadVector( child, v );
                    l->SetDirection(v);
                    printf("   direction %f %f %f\n",v.x,v.y,v.z);
                }
            }
        } else if ( COMPARE(type,"point") ) {
            printf(" - Point\n");
            PointLight *l = new PointLight();
            light = l;
            for ( TiXmlElement *child = element->FirstChildElement(); child!=nullptr; child = child->NextSiblingElement() ) {
                if ( COMPARE( child->Value(), "intensity" ) ) {
                    Color c(1,1,1);
                    ReadColor( child, c );
                    l->SetIntensity(c);
                    printf("   intensity %f %f %f\n",c.r,c.g,c.b);
                } else if ( COMPARE( child->Value(), "position" ) ) {
                    Vec3f v(0,0,0);
                    ReadVector( child, v );
                    l->SetPosition(v);
                    printf("   position %f %f %f\n",v.x,v.y,v.z);
                } else if ( COMPARE( child->Value(), "size" ) ) {
                    float f = 0;
                    ReadFloat( child, f );
                    l->SetSize(f);
                    printf("   size %f\n",f);
                }
            }
        } else {
            printf(" - UNKNOWN\n");
        }
    }
 
    if ( light ) {
        light->SetName(name);
        lights.push_back(light);
    }
 
}
 
//-------------------------------------------------------------------------------
 
void ReadVector(TiXmlElement *element, Vec3f &v)
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
 
void ReadColor(TiXmlElement *element, Color &c)
{
    double r = (double) c.r;
    double g = (double) c.g;
    double b = (double) c.b;
    element->QueryDoubleAttribute( "r", &r );
    element->QueryDoubleAttribute( "g", &g );
    element->QueryDoubleAttribute( "b", &b );
    c.r = (float) r;
    c.g = (float) g;
    c.b = (float) b;
 
    float f=1;
    ReadFloat( element, f );
    c *= f;
}
 
//-------------------------------------------------------------------------------
 
void ReadFloat (TiXmlElement *element, float &f, char const *name)
{
    double d = (double) f;
    element->QueryDoubleAttribute( name, &d );
    f = (float) d;
}
 
//-------------------------------------------------------------------------------
 
TextureMap* ReadTexture(TiXmlElement *element)
{
    char const* texName = element->Attribute("texture");
    if ( texName == nullptr ) return nullptr;
 
    Texture *tex = nullptr;
    if ( COMPARE(texName,"checkerboard") ) {
        TextureChecker *ctex = new TextureChecker;
        tex = ctex;
        printf("      Texture: Checker Board\n");
        for ( TiXmlElement *child = element->FirstChildElement(); child!=nullptr; child = child->NextSiblingElement() ) {
            if ( COMPARE( child->Value(), "color1" ) ) {
                Color c(0,0,0);
                ReadColor( child, c );
                ctex->SetColor1(c);
                printf("         color1 %f %f %f\n",c.r,c.g,c.b);
            } else if ( COMPARE( child->Value(), "color2" ) ) {
                Color c(0,0,0);
                ReadColor( child, c );
                ctex->SetColor2(c);
                printf("         color2 %f %f %f\n",c.r,c.g,c.b);
            }
        }
        textureList.Append( tex, texName );
    } else {
        tex = ReadTexture( texName );
    }
 
    TextureMap *map = new TextureMap(tex);
    LoadTransform(map,element,1);
    return map;
}
 
//-------------------------------------------------------------------------------
 
Texture* ReadTexture(char const *texName)
{
    printf("      Texture: File \"%s\"",texName);
    Texture *tex = textureList.Find( texName );
    if ( tex == nullptr ) {
        TextureFile *ftex = new TextureFile;
        tex = ftex;
        ftex->SetName(texName);
        if ( ! ftex->Load() ) {
            printf(" -- Error loading file!");
            delete tex;
            tex = nullptr;
        } else {
            textureList.Append( tex, texName );
        }
    }
    printf("\n");
 
    return tex;
}
 
//-------------------------------------------------------------------------------