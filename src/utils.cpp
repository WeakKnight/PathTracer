#include <math.h>
#include "cyVector.h"
#include "cyMatrix.h"
#include "cyColor.h"
#include <assert.h>
#include "utils.h"
#include "constants.h"
#include "string_utils.h"

using namespace cy;

void BranchlessONB(const Vec3f& n, Vec3f& b1, Vec3f& b2)
{
	float sign = copysignf(1.0f, n.z);
	const float a = -1.0f / (sign + n.z);
	const float b = n.x * n.y * a;
	b1 = Vec3f(1.0f + sign * n.x * n.x * a, sign * b, -sign * n.x);
	b2 = Vec3f(b, sign + n.y * n.y * a, -n.y);
}

void CommonOrthonormalBasis(const Vec3f& n, Vec3f& b1, Vec3f& b2)
{
	Vec3f randomVector = Vec3f(
		(static_cast <float> (rand()) / static_cast <float> (RAND_MAX)),
		(static_cast <float> (rand()) / static_cast <float> (RAND_MAX)),
		(static_cast <float> (rand()) / static_cast <float> (RAND_MAX))).GetNormalized();

	while (randomVector.Dot(n) >= (1.0f - RANDOM_THRESHOLD))
	{
		randomVector = Vec3f(
			(static_cast <float> (rand()) / static_cast <float> (RAND_MAX)),
			(static_cast <float> (rand()) / static_cast <float> (RAND_MAX)),
			(static_cast <float> (rand()) / static_cast <float> (RAND_MAX))).GetNormalized();
	}

	b1 = randomVector.Cross(n).GetNormalized();
	b2 = b1.Cross(n).GetNormalized();
}


Vec3f RandomInUnitSphere()
{
	Vec3f result;
	do
	{
		result = 2.0f * Vec3f(
			(static_cast <float> (rand()) / static_cast <float> (RAND_MAX)),
			(static_cast <float> (rand()) / static_cast <float> (RAND_MAX)),
			(static_cast <float> (rand()) / static_cast <float> (RAND_MAX)))
			- Vec3f(1.0f, 1.0f, 1.0f);
	} while (result.LengthSquared() >= 1.0f);

	return result;
}

Vec2f NonUniformRandomPointInCircle(float radius)
{
	float r = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))* radius;
	float theta = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))* TWO_PI;

	float x = r * cos(theta);
	float y = r * sin(theta);

	return Vec2f(x, y);
}

Vec2f RandomPointInCircle(float radius)
{
	// generate a random value between 0 to Radius as the value of Cumulative Distribution Function
	float S = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
	// S = r2 / R2, choose r based on F
	float r = sqrtf(S) * radius;
	float theta = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))* TWO_PI;

	float x = r * cos(theta);
	float y = r * sin(theta);

	return Vec2f(x, y);
}

Vec3f UniformRandomPointOnHemiSphere()
{
	float cosTheta = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
	float sinTheta = std::sqrt(1.0f - (cosTheta * cosTheta));
	if (sinTheta < 0.0f)
	{
		sinTheta = 0.0f;
	}

	float beta = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * TWO_PI;

	// z = 1 * cosTheta, r = 1 * sinTheta, x = cosBeta * sinTheta, y = sinBeta * sinTheta
	return Vec3f(sinTheta * cos(beta), sinTheta * sin(beta), cosTheta);
}

Vec3f CosineWeightedRandomPointOnHemiSphere()
{
    float F = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
    float cosine2Theta = 1.0f  - 2.0f * F;
    float theta = 0.5f * acos(cosine2Theta);
    float cosTheta = cos(theta);
    float sinTheta = sin(theta);
    
    float beta = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * TWO_PI;
    
    // z = 1 * cosTheta, r = 1 * sinTheta, x = cosBeta * sinTheta, y = sinBeta * sinTheta
    return Vec3f(sinTheta * cos(beta), sinTheta * sin(beta), cosTheta);
}

Vec3f ImportanceSampleGGX(float roughness, float& probability)
{
	float a = roughness * roughness;

	float F = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 0.99999f;
	float theta = acos(
		sqrt(
		(1.0f - F)/(F * (a*a - 1.0f) + 1.0f)
		)
	);

	float cosTheta = cos(theta);
	float sinTheta = sin(theta);

	if (cosTheta < 0.0f)
	{
		cosTheta = 0.0f;
	}

	float beta = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))* TWO_PI;

	float bottom = (a * a - 1.0f) * cosTheta * cosTheta + 1.0f;
	bottom = bottom * bottom;

	if (bottom <= 0.0f)
	{
		bottom = 0.001f;
	}


	probability = a * a * cosTheta * sinTheta * INVERSE_PI / bottom;
	if (probability <= 0.0f)
	{
		probability = 0.001f;
	}

	if (isnan(probability))
	{
		int a = 1;
	}
	return Vec3f(sinTheta * cos(beta), sinTheta * sin(beta), cosTheta);
}

int MIS2(float p1, float p2)
{
	float total = p1 + p2;
	float random = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))* total;
	if (random <= p1)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

int MIS3(float p1, float p2, float p3)
{
	float total = p1 + p2 + p3;
	float random = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))* total;

	if (random <= p1)
	{
		return 0;
	}
	else if(random > p1 && random <= (p1 + p2))
	{
		return 1;
	}
	else
	{
		return 2;
	}
}

float LightFallOffFactor(const Vec3f& p1, const Vec3f& p2)
{
	float distanceSquare = (p2 - p1).LengthSquared();
	float distance = sqrt(distanceSquare);
	return 1.0f / distanceSquare;
	// return 1.0f / (1.0f + 0.12f * distance + 0.032f * distanceSquare);
}

float LightFallOffFactor(float distance)
{
	float distanceSquare = distance * distance;
	return 1.0f / distanceSquare;
	// return 1.0f / (1.0f + 0.12f * distance + 0.032f * distanceSquare);
}

Vec2f UniformSampleTriangle()
{
	float u0 = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
	float u1 = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
	float su0 = std::sqrt(u0);
	return Vec2f(1.0f - su0, u1 * su0);
}

CDF::CDF()
	:total(0.0f)
{
	Init();
}

void CDF::Init()
{
	cd = std::vector<float>({0.0f});
	total = (+0.0f);
}

// rand from 0 to tatal, return the correspond id
int CDF::Sample() const
{
	if (cd.size() == 1)
	{
		return 0;
	}

	float random = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))* total;
	for (int i = 1; i < cd.size(); i++)
	{
		if (random <= cd[i] && random >= cd[i - (size_t)1])
		{
			return i - 1;
		}
	}

	assert(false);
	return 0;
}

void CDF::Add(float val)
{
	total += val;
	cd.push_back(total);
}

Vec3f ParseVec3f(std::string& str)
{
	std::string currentLine = str;
	
	StringUtils::trim(currentLine);

	std::vector<std::string> tokens;
	std::string res = "";

	for (size_t i = 0; i < currentLine.size(); i++)
	{
		if (currentLine[i] != ' ')
		{
			res += currentLine[i];
		}
		else
		{
			if (res.size() != 0)
			{
				tokens.push_back(res);
			}
			res = "";
		}
	}

	if (res.size() != 0)
	{
		tokens.push_back(res);
	}

	float r = std::stof(tokens[0]);
	float g = std::stof(tokens[1]);
	float b = std::stof(tokens[2]);

	return Vec3f(r, g, b);
}