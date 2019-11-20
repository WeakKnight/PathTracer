#pragma once

#include "scene.h"
#include <vector>
#include <random>
#include <mutex> 

#include "pathtracer.h"

#define ONE_MINUS_EPSILON 0x1.fffffep-1

class HaltonSampler{
public:
    
    HaltonSampler();
    
	RayContext SamplePixel(int x, int y, Vec2f& randomOffset, int index) const;
    
private:
 
    int haltonXBase = 2;
    int haltonYBase = 3;
    std::mt19937_64 rng;
};

// 0 to 1
class Quasy2DSampler
{
public:
	Quasy2DSampler()
	{
		xOffset = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
		yOffset = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
	}
	Vec2f GenRandom2DVector()
	{
		float x = Halton(xIndex, haltonXBase) + xOffset;
		mtx.lock();
		xIndex++;
		mtx.unlock();
		if (x > 1.0f)
		{
			x -= 1.0f;
		}

		float y = Halton(yIndex, haltonYBase) + yOffset;
		mtx.lock();
		yIndex++;
		mtx.unlock();
		if (y > 1.0f)
		{
			y -= 1.0f;
		}

		return Vec2f(x, y);
	}
private:
	float xOffset = 0.0f;
	float yOffset = 0.0f;
	int xIndex = 0;
	int yIndex = 0;
	int haltonXBase = 2;
	int haltonYBase = 3;
	std::mutex mtx;
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

class QuasyMonteCarloHemiSphereSampler
{
public:
	QuasyMonteCarloHemiSphereSampler()
	{
		BetaOffset = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))* Pi<float>() * 2.0f;
		FOffset = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
	}

	Vec3f CosineWeightedSample()
	{
		float F = Halton(FIndex, FBase) + FOffset;
		FIndex++;
		if (F > 1.0f)
		{
			F -= 1.0f;
		}

		float cosine2Theta = 1.0f - 2.0f * F;
		float theta = 0.5f * acos(cosine2Theta);
		float cosTheta = cos(theta);
		float sinTheta = sin(theta);

		float beta = Halton(BetaIndex, BetaBase) * Pi<float>() * 2.0f + BetaOffset;
		BetaIndex++;
		if (beta > Pi<float>() * 2.0f)
		{
			beta -= Pi<float>() * 2.0f;
		}

		// z = 1 * cosTheta, r = 1 * sinTheta, x = cosBeta * sinTheta, y = sinBeta * sinTheta
		return Vec3f(sinTheta * cos(beta), sinTheta * sin(beta), cosTheta);
	}

private:
	float FOffset = 0.0f;
	float BetaOffset = 0.0f;
	int FIndex = 0;
	int BetaIndex = 0;
	int FBase = 2;
	int BetaBase = 3;
};