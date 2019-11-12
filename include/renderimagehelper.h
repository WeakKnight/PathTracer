//
//  renderimagehelper.h
//  RayTracer
//
//  Created by TianyuLi on 2019/8/24.
//

#ifndef renderimagehelper_h
#define renderimagehelper_h

#include "cyColor.h"
#include "scene.h"

class RenderImageHelper
{
    public:
    static void SetPixel(RenderImage& image, int x, int y, cyColor24 color)
    {
        Color24* pixels = image.GetPixels();
        int width = image.GetWidth();
        pixels[x + y * width] = color;
    }
    
    static void SetSampleNum(RenderImage& image, int x, int y, int num)
    {
        uint8_t* pixels = image.GetSampleCount();
        int width = image.GetWidth();
        pixels[x + y * width] = (uint8_t)num;
    }
    
    static void SetNormal(Color24* pixels, RenderImage& image, int x, int y, Vec3f normal)
    {
		if (pixels == nullptr)
		{
			return;
		}
        int width = image.GetWidth();
        pixels[x + y * width] = Color24((0.5f + normal.x * 0.5f) * 255.0f, (0.5f + normal.y * 0.5f) * 255.0f, (0.5f + normal.z * 0.5f) * 255.0f);
    }

	static void SetIrradianceCache(Color24* pixels, RenderImage& image, int x, int y)
	{
		if (pixels == nullptr)
		{
			return;
		}
		int width = image.GetWidth();
		pixels[x + y * width] = Color24(255.0f, 255.0f, 255.0f);
	}
    
    static void SetDepth(RenderImage& image, int x, int y, float depth)
    {
        float* zBuffer = image.GetZBuffer();
        int width = image.GetWidth();
        zBuffer[x + y * width] = depth;
    }
    
    static void CalculateMySampleImg(Color24* img, RenderImage& image)
    {
		if (img == nullptr)
		{
			return;
		}
        auto width = image.GetWidth();
        auto height = image.GetHeight();
        
        int size = width * height;
        
        uint8_t* sampleCount = image.GetSampleCount();
        
        uint8_t smin=255, smax=0;
        for ( int i=0; i<size; i++ ) {
            if ( smin > sampleCount[i] ) smin = sampleCount[i];
            if ( smax < sampleCount[i] ) smax = sampleCount[i];
        }
        if ( smax == smin ) {
            for ( int i=0; i<size; i++ ) img[i] = Color24(0.0f, 0.0f, 0.0f);
        } else {
            for ( int i=0; i<size; i++ ) {
                int c = (255*(sampleCount[i]-smin))/(smax-smin);
                if ( c < 0 ) c = 0;
                if ( c > 255 ) c = 255;
                img[i] = Color24(c, c, c);
            }
        }
    }
    
    static void CalculateMyDepthImg(Color24* img, RenderImage& image)
    {
		if (img == nullptr)
		{
			return;
		}
        int width = image.GetWidth();
        int height = image.GetHeight();
        
        int size = width * height;
        
        auto zbuffer = image.GetZBuffer();
        
        float zmin=BIGFLOAT, zmax=0;
        for ( int i=0; i<size; i++ ) {
            if ( zbuffer[i] == BIGFLOAT ) continue;
            
            if ( zmin > zbuffer[i] ) zmin = zbuffer[i];
            if ( zmax < zbuffer[i] ) zmax = zbuffer[i];
        }
        for ( int i=0; i<width; i++ ) {
            for(int j=0; j<height; j++)
            {
                if ( zbuffer[i + j * width] == BIGFLOAT )
                {
                    img[i + j * width] = Color24::Black();
                }
                else {
                    // zmin white ----> zmax black
                    float zValue = zbuffer[i + j * width];
                    float colorUnit = (1.0f - (zValue - zmin)/(zmax - zmin));
                    img[i + j * width] = Color24(255.0f * colorUnit, 255.0f * colorUnit, 255.0f * colorUnit);
                }
            }
        }
    }
};


#endif /* renderimagehelper_h */
