#include "Histogram.h"
#include "RowsProcessor.h"

#include <algorithm>
#include <numeric>
#include <execution>
#include <thread>

struct HistData
{
	std::vector<unsigned> counts;
	std::vector<float>& hist;
};

void TotalNormalize(const HistData& r, const HistData& g, const HistData& b);
void NormalizeHistogram(const std::vector<unsigned>& counts, std::vector<float>& hist);

void SequentialHistogram(const Image& image, std::vector<float>& histR, std::vector<float>& histG, std::vector<float>& histB)
{
	const auto width = image.GetWidth();
	const auto height = image.GetHeight();
	const auto data = image.GetData();

	std::vector<unsigned> countsR(Image::RGB_MAX + 1, 0);
	std::vector<unsigned> countsG(Image::RGB_MAX + 1, 0);
	std::vector<unsigned> countsB(Image::RGB_MAX + 1, 0);

	for (int y = 0; y < height; ++y)
	{
		const auto row = data + y * width * 3;
		for (int x = 0; x < width; ++x)
		{
			countsR[row[x * 3]]++;
			countsG[row[x * 3 + 1]]++;
			countsB[row[x * 3 + 2]]++;
		}
	}
	TotalNormalize(
		{ countsR, histR },
		{ countsG, histG },
		{ countsB, histB });
}

void AtomicHistogramInterleaved(
	const Image& image,
	int numThreads,
	std::vector<float>& histR,
	std::vector<float>& histG,
	std::vector<float>& histB)
{
	const auto width = image.GetWidth();
	const auto height = image.GetHeight();
	const auto data = image.GetData();
	const auto totalRows = height;
	numThreads = std::min(numThreads, totalRows);

	std::vector<std::atomic<unsigned>> atomicCounters((Image::RGB_MAX + 1) * 3);
	for (auto& counter : atomicCounters)
	{
		counter.store(0, std::memory_order_relaxed);
	}
	std::vector<std::jthread> threads;
	threads.reserve(numThreads);

	const int rowsPerThread = totalRows / numThreads;
	const int remainder = totalRows % numThreads;

	{
		int startRow = 0;
		for (int i = 0; i < numThreads; i++)
		{
			const int extra = (i < remainder) ? 1 : 0;
			int endRow = startRow + rowsPerThread + extra;
			threads.emplace_back(
				ProcessRowsAtomicInterleaved,
				data,
				width,
				startRow,
				endRow,
				std::span(atomicCounters.data(), atomicCounters.size()));
			startRow = endRow;
		}
	}

	std::vector<unsigned int> countsR(Image::RGB_MAX + 1, 0);
	std::vector<unsigned int> countsG(Image::RGB_MAX + 1, 0);
	std::vector<unsigned int> countsB(Image::RGB_MAX + 1, 0);
	for (int i = 0; i < Image::RGB_MAX + 1; ++i)
	{
		countsR[i] = atomicCounters[i * 3 + 0].load(std::memory_order_relaxed);
		countsG[i] = atomicCounters[i * 3 + 1].load(std::memory_order_relaxed);
		countsB[i] = atomicCounters[i * 3 + 2].load(std::memory_order_relaxed);
	}

	TotalNormalize(
		{ countsR, histR },
		{ countsG, histG },
		{ countsB, histB });
}

void AtomicHistogramSeparate(
	const Image& image,
	int numThreads,
	std::vector<float>& histR,
	std::vector<float>& histG,
	std::vector<float>& histB)
{
	const auto width = image.GetWidth();
	const auto height = image.GetHeight();
	const auto data = image.GetData();
	const auto totalRows = height;
	numThreads = std::min(numThreads, totalRows);

	std::vector<std::atomic<unsigned int>> atomicR(Image::RGB_MAX + 1);
	std::vector<std::atomic<unsigned int>> atomicG(Image::RGB_MAX + 1);
	std::vector<std::atomic<unsigned int>> atomicB(Image::RGB_MAX + 1);
	for (int i = 0; i < Image::RGB_MAX + 1; ++i)
	{
		atomicR[i].store(0, std::memory_order_relaxed);
		atomicG[i].store(0, std::memory_order_relaxed);
		atomicB[i].store(0, std::memory_order_relaxed);
	}
	std::vector<std::jthread> threads;
	threads.reserve(numThreads);

	const auto rowsPerThread = totalRows / numThreads;
	const auto remainder = totalRows % numThreads;

	{
		int startRow = 0;
		for (int t = 0; t < numThreads; t++)
		{
			const auto extra = (t < remainder) ? 1 : 0;
			int endRow = startRow + rowsPerThread + extra;
			threads.emplace_back(
				ProcessRowsAtomicSeparate,
				data, width, startRow, endRow,
				std::span(atomicR.data(), atomicR.size()),
				std::span(atomicG.data(), atomicG.size()),
				std::span(atomicB.data(), atomicB.size()));
			startRow = endRow;
		}
	}

	std::vector<unsigned> countsR(Image::RGB_MAX + 1);
	std::vector<unsigned> countsG(Image::RGB_MAX + 1);
	std::vector<unsigned> countsB(Image::RGB_MAX + 1);
	for (int i = 0; i < Image::RGB_MAX + 1; ++i)
	{
		countsR[i] = atomicR[i].load(std::memory_order_relaxed);
		countsG[i] = atomicG[i].load(std::memory_order_relaxed);
		countsB[i] = atomicB[i].load(std::memory_order_relaxed);
	}

	TotalNormalize(
		{ countsR, histR },
		{ countsG, histG },
		{ countsB, histB });
}

void LocalHistogram(const Image& image, int numThreads, std::vector<float>& histR, std::vector<float>& histG, std::vector<float>& histB)
{
	const auto width = image.GetWidth();
	const auto height = image.GetHeight();
	const auto data = image.GetData();
	const auto totalRows = height;
	numThreads = std::min(numThreads, totalRows);

	std::vector<LocalCounters> locals(numThreads);
	std::vector<std::jthread> threads;
	threads.reserve(numThreads);

	const auto rowsPerThread = totalRows / numThreads;
	const auto remainder = totalRows % numThreads;
	{
		int startRow = 0;
		for (int t = 0; t < numThreads; t++)
		{
			const auto extra = (t < remainder) ? 1 : 0;
			int endRow = startRow + rowsPerThread + extra;
			threads.emplace_back([&locals, t, data, width, startRow, endRow]() {
				ProcessRowsLocal(data, width, startRow, endRow, locals[t]);
			});
			startRow = endRow;
		}
	}

	std::vector<unsigned> totalR(Image::RGB_MAX + 1, 0);
	std::vector<unsigned> totalG(Image::RGB_MAX + 1, 0);
	std::vector<unsigned> totalB(Image::RGB_MAX + 1, 0);
	for (const auto& local : locals)
	{
		for (int i = 0; i < Image::RGB_MAX + 1; ++i)
		{
			totalR[i] += local.r[i];
			totalG[i] += local.g[i];
			totalB[i] += local.b[i];
		}
	}

	TotalNormalize(
		{ totalR, histR },
		{ totalG, histG },
		{ totalB, histB });
}

void ParallelForEachHistogram(const Image& image, std::vector<float>& histR, std::vector<float>& histG, std::vector<float>& histB)
{
	const auto width = image.GetWidth();
	const auto height = image.GetHeight();
	const auto data = image.GetData();

	std::vector<std::atomic<unsigned int>> atomicR(Image::RGB_MAX + 1);
	std::vector<std::atomic<unsigned int>> atomicG(Image::RGB_MAX + 1);
	std::vector<std::atomic<unsigned int>> atomicB(Image::RGB_MAX + 1);

	for (int i = 0; i < Image::RGB_MAX + 1; ++i)
	{
		atomicR[i].store(0, std::memory_order_relaxed);
		atomicG[i].store(0, std::memory_order_relaxed);
		atomicB[i].store(0, std::memory_order_relaxed);
	}

	std::vector<int> rows(height);
	std::iota(rows.begin(), rows.end(), 0);

	std::for_each(std::execution::par, rows.begin(), rows.end(),
		[&](const int y) {
			const uint8_t* row = data + y * width * 3;
			for (int x = 0; x < width; ++x)
			{
				atomicR[row[x * 3]].fetch_add(1, std::memory_order_relaxed);
				atomicG[row[x * 3 + 1]].fetch_add(1, std::memory_order_relaxed);
				atomicB[row[x * 3 + 2]].fetch_add(1, std::memory_order_relaxed);
			}
		});

	std::vector<unsigned> countsR(Image::RGB_MAX + 1);
	std::vector<unsigned> countsG(Image::RGB_MAX + 1);
	std::vector<unsigned> countsB(Image::RGB_MAX + 1);
	for (int i = 0; i < Image::RGB_MAX + 1; ++i)
	{
		countsR[i] = atomicR[i].load(std::memory_order_relaxed);
		countsG[i] = atomicG[i].load(std::memory_order_relaxed);
		countsB[i] = atomicB[i].load(std::memory_order_relaxed);
	}

	NormalizeHistogram(countsR, histR);
	NormalizeHistogram(countsG, histG);
	NormalizeHistogram(countsB, histB);
}

void TotalNormalize(const HistData& r, const HistData& g, const HistData& b)
{
	NormalizeHistogram(r.counts, r.hist);
	NormalizeHistogram(g.counts, g.hist);
	NormalizeHistogram(b.counts, b.hist);
}

void NormalizeHistogram(
	const std::vector<unsigned>& counts,
	std::vector<float>& hist)
{
	const auto totalPixels = std::accumulate(counts.begin(), counts.end(), 0u);
	hist.resize(Image::RGB_MAX + 1);
	if (totalPixels == 0)
	{
		return;
	}
	for (int i = 0; i < Image::RGB_MAX + 1; i++)
	{
		hist[i] = static_cast<float>(counts[i]) / totalPixels;
	}
}
