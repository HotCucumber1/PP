#include "Histogram.h"
#include "Image.h"

#include <catch2/catch_all.hpp>
#include <numeric>


TEST_CASE("Histogram benchmark", "[benchmark]")
{
	constexpr int width = 8000;
	constexpr int height = 6000;
	Image img(width, height);
	img.GenerateRandom();

	std::vector<float> histR;
	std::vector<float> histG;
	std::vector<float> histB;

	BENCHMARK("Sequential")
	{
		SequentialHistogram(img, histR, histG, histB);
	};

	// for (const auto& numThreads : { 1, 2, 4, 8, 16 })
	// {
	// 	BENCHMARK("AtomicInterleaved threads=" + std::to_string(numThreads))
	// 	{
	// 		AtomicHistogramInterleaved(img, numThreads, histR, histG, histB);
	// 	};
	// 	BENCHMARK("AtomicSeparate threads=" + std::to_string(numThreads))
	// 	{
	// 		AtomicHistogramSeparate(img, numThreads, histR, histG, histB);
	// 	};
	// 	BENCHMARK("LocalHistogram threads=" + std::to_string(numThreads))
	// 	{
	// 		LocalHistogram(img, numThreads, histR, histG, histB);
	// 	};
	// }

	BENCHMARK("ParallelForEach (std::execution::par)")
	{
		ParallelForEachHistogram(img, histR, histG, histB);
	};
}