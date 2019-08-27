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

namespace RayTracing
{
    class RenderImageHelper
    {
        public:
        static void SetPixel(RenderImage& image, int x, int y, cyColor24 color)
        {
            Color24* pixels = image.GetPixels();
            int width = image.GetWidth();
            pixels[x + y * width] = color;
        }
        
        static void SetDepth(RenderImage& image, int x, int y, float depth)
        {
            float* zBuffer = image.GetZBuffer();
            int width = image.GetWidth();
            zBuffer[x + y * width] = depth;
        }
        
        static void CalculateMyDepthImg(Color24* img, RenderImage& image)
        {
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
}

#endif /* renderimagehelper_h */
