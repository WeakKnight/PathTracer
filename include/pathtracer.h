#pragma once

#include <thread>
#include <vector>
#include <future>
#include <mutex>

#include "scene.h"

class HaltonSampler;
class RenderWorker;

struct PixelContext
{
	PixelContext()
	{
		offset = Vec2f((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)),
			(static_cast <float> (rand()) / static_cast <float> (RAND_MAX)));
	}

	// store current sample count
	unsigned int CurrentSampleNum = 0;
	Color color = Color::Black();
	Vec3f normal = Vec3f(0.0f, 0.0f, 0.0f);
	float z = 0.0f;
	Vec2f offset;
};

class PathTracer
{
public:
	void Init(unsigned int _width, unsigned int _height);
	void Run();
	void Join();
public:
	unsigned int width = 0;
	unsigned int height = 0;
	unsigned int size = 0;

	std::vector<PixelContext> pixelData;
	HaltonSampler* haltonSampler;
	std::vector<RenderWorker*> workers;
};

class RenderWorker 
{
public:
	RenderWorker(int _index, int _cores, PathTracer* _render);
	
	void Run();
	void Join();

	int originalIndex;
	int cores;
	std::thread* thread;
	HaltonSampler* haltonSampler;
	std::vector<PixelContext> pixelData;

	std::mutex mtx;

	unsigned int width = 0;
	unsigned int height = 0;
	unsigned int size = 0;
};



