#pragma once

#include "scene.h"
#include <vector>
#include <random>
#include <mutex> 

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
    
    void SetMinimumSampleCount(int count)
    {
        minimumSampleNum = count;
    }
    
    void SetColorTolerance(float tolerance)
    {
        colorTolerance = tolerance;
    }
    
private:
    
    bool ContinueSamplingCondition(std::vector<SampleResult>& sampleResult);
    
    float colorTolerance;
    
    std::vector<SampleResult>* results;
    // use for storing count info
    std::vector<int> sampleCountResult;
    unsigned int sampleNum;
    unsigned int minimumSampleNum;
    int haltonXBase = 2;
    int haltonYBase = 3;
    std::mt19937_64 rng;
};

class QuasyMonteCarloCircleSampler 
{
public:
	QuasyMonteCarloCircleSampler()
	{
		thetaOffset = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * Pi<float>() * 2.0f;
		sOffset = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
	}

	float RandomGlossAngleFactor()
	{
		float result = Halton(sIndex, haltonSBase) + sOffset;
		
		mtx.lock();
		sIndex++;
		mtx.unlock();

		if (result > 1.0f)
		{
			result -= 1.0f;
		}
		return result;
	}

	float RandomTheta()
	{
		float result = Halton(thetaIndex, haltonThetaBase) + sOffset;
		
		mtx.lock();
		thetaIndex++;
		mtx.unlock();

		if (result > 1.0f)
		{
			result -= 1.0f;
		}

		return result * Pi<float>() * 2.0f;
	}

	Vec2f RandomPointInCircle(float radius)
	{
		// generate a random value between 0 to Radius as the value of Cumulative Distribution Function
		float S = Halton(sIndex, haltonSBase) + sOffset;
		mtx.lock();
		sIndex++;
		mtx.unlock();

		if (S > 1.0f)
		{
			S -= 1.0f;
		}
		// S = r2 / R2, choose r based on F
		float r = sqrtf(S) * radius;

		float theta = Halton(thetaIndex, haltonThetaBase) * Pi<float>() * 2.0f + thetaOffset;
		mtx.lock();
		thetaIndex++;
		mtx.unlock();

		if (theta > (Pi<float>() * 2.0f))
		{
			theta -= (Pi<float>() * 2.0f);
		}

		float x = r * cos(theta);
		float y = r * sin(theta);

		return Vec2f(x, y);
	}

	float sOffset = 0.0f;
	float thetaOffset = 0.0f;

	int haltonSBase = 2;
	int haltonThetaBase = 3;

	int sIndex = 0;
	int thetaIndex = 0;

	std::mutex mtx;
};