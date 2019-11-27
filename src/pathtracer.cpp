#include "pathtracer.h"
#include "renderimagehelper.h"
#include "sampler.h"
#include <atomic>
#include "spdlog/spdlog.h"
#include "render.h"

extern Node rootNode;
extern RenderImage renderImage;
extern Color24* normalPixels;
extern std::atomic<bool> outputing;

void PathTracer::Init(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
	size = width * height;
	haltonSampler = new HaltonSampler();

	pixelData.resize(size);
}

void PathTracer::Run()
{
	std::size_t cores =
		//1;
		 std::thread::hardware_concurrency() - 1;

	for (std::size_t i = 0; i < cores; i++)
	{
		auto worker = new RenderWorker(i, cores, this);
		// worker->Run();
		workers.push_back(worker);
	}
}

void PathTracer::Join()
{
	for (int i = 0; i < workers.size(); i++)
	{
		workers[i]->Join();
	}
}

void RenderWorker::Join()
{
	thread->join();
}

RenderWorker::RenderWorker(int _index, int _cores, PathTracer* _render)
{
	originalIndex = _index;
	cores = _cores;
	
	pixelData.resize(_render->size);

	haltonSampler = new HaltonSampler;

	width = _render->width;
	height = _render->height;
	size = _render->size;

	thread = new std::thread(&RenderWorker::Run, this);
}

void RenderWorker::Run()
{
	int index = originalIndex;

	while (true)
	{
		int y = index / renderImage.GetWidth();
		int x = index - y * renderImage.GetWidth();

		PixelContext& historyContext = pixelData[x + y * width];
		historyContext.CurrentSampleNum += 1;

		float factor = (1.0f / (float)(historyContext.CurrentSampleNum));

		RayContext primaryRay = haltonSampler->SamplePixel(x, y, historyContext.offset, historyContext.CurrentSampleNum - 1);

		auto renderResult = RenderPixel(primaryRay, x, y);

		historyContext.color
			// = sampleResult.color;
			= ((float)(historyContext.CurrentSampleNum - 1) * historyContext.color + (renderResult.color)) * factor;

		// historyContext.color.ClampMax();

		const auto& finalColor = historyContext.color;

		historyContext.z = historyContext.z + (renderResult.z * factor);
		historyContext.normal = historyContext.normal + (renderResult.normal * factor);

		if (outputing.load())
		{
			spdlog::info("Worker Break!");
			break;
		}

		RenderImageHelper::SetPixel(renderImage, x, y, Color24(finalColor.r * 255.0f, finalColor.g * 255.0f, finalColor.b * 255.0f));

		index += cores;

		if (index > (size - 1))
		{
			index = originalIndex;
		}
	}
}