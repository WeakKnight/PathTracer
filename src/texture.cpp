
//-------------------------------------------------------------------------------
///
/// \file       texture.cpp 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    1.0
/// \date       August 21, 2019
///
/// \brief Example source for CS 6620 - University of Utah.
///
//-------------------------------------------------------------------------------
 
#include "texture.h"
#include "lodepng.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

 
//-------------------------------------------------------------------------------
 
int ReadLine( FILE *fp, int size, char *buffer )
{
    int i;
    for ( i=0; i<size; i++ ) {
        buffer[i] = fgetc(fp);
        if ( feof(fp) || buffer[i] == '\n' || buffer[i] == '\r' ) {
            buffer[i] = '\0';
            return i+1;
        }
    }
    return i;
}
 
bool TextureFile::Load()
{
    data.clear();
    width = 0;
    height = 0;
    char const *name = GetName();
    if ( name[0] == '\0' ) return false;
 
    int len = (int) strlen(name);
    if ( len < 3 ) return false;
 
    bool success = false;
 
    char ext[3] = { (char)tolower(name[len-3]), (char)tolower(name[len-2]), (char)tolower(name[len-1]) };
	
    if ( strncmp(ext,"png",3) == 0 
		|| strncmp(ext, "jpg", 3) == 0
		|| strncmp(ext, "hdr", 3) == 0
		) 
	{
		int width, height, nrChannels;
		// if (strncmp(ext, "hdr", 3) == 0)
		{
			stbi_set_flip_vertically_on_load(true);
		}
		float* rawData = stbi_loadf(name, &width, &height, &nrChannels, 3);
		// if (strncmp(ext, "hdr", 3) == 0)
		{
			stbi_set_flip_vertically_on_load(false);
		}
		if (rawData)
		{
			this->width = width;
			this->height = height;

			success = true;
			data.resize(width * height);
			memcpy(data.data(), rawData, width * height * 3 * sizeof(float));
			stbi_image_free(rawData);
		}
    }
 
    return success;
}
 
//-------------------------------------------------------------------------------
 
Color TextureFile::Sample(Vec3f const &uvw) const
{
    if ( width + height == 0 ) return Color(0,0,0);
 
    Vec3f u = TileClamp(uvw);
    float x = width * u.x;
    float y = height * u.y;
    int ix = (int)x;
    int iy = (int)y;
    float fx = x - ix;
    float fy = y - iy;
 
    if ( ix < 0 ) ix -= (ix/width - 1)*width;
    if ( ix >= width ) ix -= (ix/width)*width;
    int ixp = ix+1;
    if ( ixp >= width ) ixp -= width;
 
    if ( iy < 0 ) iy -= (iy/height - 1)*height;
    if ( iy >= height ) iy -= (iy/height)*height;
    int iyp = iy+1;
    if ( iyp >= height ) iyp -= height;
 
    return  data[iy *width+ix ] * ((1-fx)*(1-fy)) +
            data[iy *width+ixp] * (   fx *(1-fy)) +
            data[iyp*width+ix ] * ((1-fx)*   fy ) +
            data[iyp*width+ixp] * (   fx *   fy );
}
 
//-------------------------------------------------------------------------------
 
Color TextureChecker::Sample(Vec3f const &uvw) const
{
    Vec3f u = TileClamp(uvw);
    if ( u.x <= 0.5f ) {
        return u.y <= 0.5f ? color1 : color2;
    } else {
        return u.y <= 0.5f ? color2 : color1;
    }
}
 
//-------------------------------------------------------------------------------