#pragma once

#include "scene.h"
#include <vector>
#include <random>

#define ONE_MINUS_EPSILON 0x1.fffffep-1

struct SampleResult {
    SampleResult()
    {
        avgColor = Color::Black();
        avgZ = 0.0f;
        avgN = Vec3f(0.0f, 0.0f, 0.0f);
    }
    
    Color avgColor;
    float avgZ;
    Vec3f avgN;
    int x;
    int y;
};

class HaltonSampler{
public:
    
    HaltonSampler();
    
    SampleResult SamplePixel(int x, int y);
    void BeginSampling();
    
    void SetSampleCount(int count)
    {
        sampleNum = count;
    }
    
    void SetColorTolerance(float tolerance)
    {
        colorTolerance = tolerance;
    }
    
private:
    
    bool ContinueSamplingCondition(std::vector<SampleResult>& sampleResult);
    
    float colorTolerance;
    
    std::vector<SampleResult> results;
    // use for storing count info
    std::vector<int> sampleCountResult;
    unsigned int sampleNum;
    unsigned int minimumSampleNum;
    int haltonXBase = 2;
    int haltonYBase = 3;
    std::mt19937_64 rng;
};
