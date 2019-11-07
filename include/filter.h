#pragma once
#include "scene.h"
#include <vector>
#include <random>
#include <memory>

class Filter{
public:
    Filter(Color24* input, unsigned int _width, unsigned int _height):
    width(_width),
    height(_height),
    inputColors(input)
    {
        outputColors = new Color24[_width * _height];
    }
    
    ~Filter()
    {
        delete outputColors;
    }
    
    virtual void Compute() = 0;
    
    inline Color24 GetOutputPixel(unsigned int x, unsigned int y) const
    {
        if(x < 0)
        {
            x = 0;
        }
        if(y < 0)
        {
            y = 0;
        }
        if(x >= width)
        {
            x = width - 1;
        }
        if(y >= height)
        {
            y = height - 1;
        }
        
        return outputColors[x + y * width];
    }
    
    inline Color24 GetInputPixel(unsigned int x, unsigned int y) const
    {
        if(x < 0)
        {
            x = 0;
        }
        if(y < 0)
        {
            y = 0;
        }
        if(x >= width)
        {
            x = width - 1;
        }
        if(y >= height)
        {
            y = height - 1;
        }
        
        return inputColors[x + y * width];
    }
    
    inline Color24* GetOutput() const
    {
        return outputColors;
    }
    
    inline unsigned int GetHeight() const
    {
        return height;
    }
    
    inline unsigned int GetWidth() const
    {
        return width;
    }
    
protected:
    unsigned int width, height;
    
    inline void SetOutputPixel(int x, int y, Color24 color){
        outputColors[x + y * width] = color;
    }
    
private:
    
    Color24* inputColors;
    Color24* outputColors;
    
};

class ColorShiftFilter : public Filter{
public:
    
    ColorShiftFilter(Color24* input, unsigned int _width, unsigned int _height):Filter(input, _width, _height)
    {
        int64_t seed = 6000;
        std::seed_seq ss{uint32_t(seed & 0xffffffff), uint32_t(seed>>32)};
        rng.seed(ss);
    }
    
    virtual void Compute()
    {
        std::uniform_real_distribution<float> unif(0.0f, 1.0f);
        int offsetX = 0.02f * width;
        int offsetY = 0.052f * height;
        
        for(size_t i = 0; i < width * height; i++)
        {
            int y = i / width;
            int x = i - y * width;
            
            int rOffsetX = (unif(rng)) * width * 0.01f;
            int rOffsetY = (unif(rng)) * height * 0.02f;
            
            int bOffsetX = (unif(rng)) * width * 0.004f;
            int bOffsetY = (unif(rng)) * height * 0.002f;
            
            int fixedOffsetB = 0.05f * width;
            int fixedOffsetG = -0.03f * width;
            
            float r = GetInputPixel(x + offsetX + rOffsetX, y + offsetY + rOffsetY).ToColor().r;
            float g = GetInputPixel(x + offsetX, y + offsetY + fixedOffsetG).ToColor().g;
            float b = GetInputPixel(x + offsetX + bOffsetX + fixedOffsetB, y + offsetY + bOffsetY).ToColor().b;
            
//            r = r * 0.5f + r * 0.43f * 0.5f;
//            g = g * 0.5f + g * 0.26f * 0.5f;
//            b = b * 0.5f + b * 0.31f * 0.5f;
            
            SetOutputPixel(x, y, Color24(Color(r, g, b)));
        }
    }
    
private:
    std::mt19937_64 rng;
};

#define GAUSSIAN_MAX_RADIUS 10
class GaussianFilter : public Filter{
public:
    GaussianFilter(Color24* input, unsigned int _width, unsigned int _height, float _alpha, Vec2f _radius)
    :Filter(input, _width, _height),
    alpha(_alpha),
    radius(_radius)
    {
        assert(GAUSSIAN_MAX_RADIUS % 2 == 0);
        
        assert(radius.x < GAUSSIAN_MAX_RADIUS);
        assert(radius.y < GAUSSIAN_MAX_RADIUS);
        
        constantComponentX = -1.0f * std::exp(-alpha * radius.x * radius.x);
        constantComponentY = -1.0f * std::exp(-alpha * radius.y * radius.y);
        factorArray = new float[(GAUSSIAN_MAX_RADIUS + 1) * (GAUSSIAN_MAX_RADIUS + 1)];
    }
    
    ~GaussianFilter()
    {
        delete factorArray;
    }
    
    virtual void Compute()
    {
        int center = GAUSSIAN_MAX_RADIUS / 2;
        
        int left = floor(-radius.x);
        int right = ceil(radius.x);
        int top = floor(-radius.y);
        int bottom = ceil(radius.y);
        
        float factorSum = 0.0f;
        
        for(int offsetX = left; offsetX <= right; offsetX++)
        {
            for(int offsetY = top; offsetY <= bottom; offsetY++)
            {
                factorArray[(center + offsetX) + (center + offsetY) * width] = Gaussian(offsetX, constantComponentX) *  Gaussian(offsetY, constantComponentY);
                factorSum += factorArray[(center + offsetX) + (center + offsetY) * width];
            }
        }
        
        for(size_t i = 0; i < width * height; i++)
        {
            int y = i / width;
            int x = i - y * width;
            
            // calculate this pixel
            Color pixelResult = Color::Black();
            
            for(int offsetX = left; offsetX <= right; offsetX++)
            {
                for(int offsetY = top; offsetY <= bottom; offsetY++)
                {
                    float factor = factorArray[(center + offsetX) + (center + offsetY) * width];
                    pixelResult += factor * (GetInputPixel(x + offsetX, y +offsetY).ToColor());
                }
            }
            
            SetOutputPixel(x, y, Color24(pixelResult / factorSum));
        }
    }
    
private:
    
    float Gaussian(float x, float contantComponent)
    {
        float result = std::exp(-alpha * x * x) + contantComponent;
		if (result < 0)
		{
			result = 0;
		}
		return result;
    }
    
    float* factorArray;
    
    float alpha;
    Vec2f radius;
    float constantComponentX;
    float constantComponentY;
};
