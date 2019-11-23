#pragma once

#include "cyMatrix.h"
#include "cyVector.h"

using namespace cy;

class ToneMapping 
{
private:
	Matrix3f inputMatrix;
	Matrix3f outputMatrix;

	Vec3f RttAndOdtFit(const Vec3f& input)
	{
		Vec3f a = input * (input + 0.0245786f) - 0.000090537f;
		Vec3f b = input * (0.983729f * input + 0.4329510f) + 0.238081f;
		return a / b;
	}

public:
	ToneMapping()
	{
		inputMatrix = Matrix3f(0.59719f, 0.35458f, 0.04823f, 0.07600f, 0.90834f, 0.01566f, 0.02840f, 0.13383f, 0.83777f);
		outputMatrix = Matrix3f(1.60475f, -0.53108f, -0.07367f, -0.10208f, 1.10813f, -0.00605f, -0.00327f, -0.07276f, 1.07602f);
	}

	Vec3f Clamp(Vec3f input)
	{
		input.Clamp(0.0f, 1.0f);
		return input;
	}

	Vec3f ACES(Vec3f input)
	{
		input = inputMatrix * input;
		input = RttAndOdtFit(input);

		input = outputMatrix * input;
		input.Clamp(0.0f, 1.0f);
		return input;
	}
};