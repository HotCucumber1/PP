#pragma once
#include "Image.h"

#include <atomic>
#include <span>

struct LocalCounters
{
	unsigned r[Image::RGB_MAX + 1]{};
	unsigned g[Image::RGB_MAX + 1]{};
	unsigned b[Image::RGB_MAX + 1]{};

	LocalCounters()
	{
		std::fill_n(r, Image::RGB_MAX + 1, 0);
		std::fill_n(g, Image::RGB_MAX + 1, 0);
		std::fill_n(b, Image::RGB_MAX + 1, 0);
	}
};

template <typename Processor>
void ProcessRows(
	const uint8_t* data,
	const int width,
	const int startRow,
	const int endRow,
	Processor&& processor)
{
	for (int y = startRow; y < endRow; ++y)
	{
		const auto row = data + y * width * 3;
		for (int x = 0; x < width; ++x)
		{
			const auto r = row[x * 3];
			const auto g = row[x * 3 + 1];
			const auto b = row[x * 3 + 2];

			processor(r, g, b);
		}
	}
}

inline void ProcessRowsAtomicInterleaved(
	const uint8_t* data,
	const int width,
	const int startRow,
	const int endRow,
	std::span<std::atomic<unsigned>> atomicCounters)
{
	ProcessRows(data, width, startRow, endRow,
		[&atomicCounters](const uint8_t r, const uint8_t g, const uint8_t b) {
			atomicCounters[r * 3 + 0].fetch_add(1, std::memory_order_relaxed);
			atomicCounters[g * 3 + 1].fetch_add(1, std::memory_order_relaxed);
			atomicCounters[b * 3 + 2].fetch_add(1, std::memory_order_relaxed);
		});
}

inline void ProcessRowsAtomicSeparate(
	const uint8_t* data,
	const int width,
	const int startRow,
	const int endRow,
	std::span<std::atomic<unsigned>> atomicR,
	std::span<std::atomic<unsigned>> atomicG,
	std::span<std::atomic<unsigned>> atomicB)
{
	ProcessRows(data, width, startRow, endRow,
		[&](const uint8_t r, const uint8_t g, const uint8_t b) {
			atomicR[r].fetch_add(1, std::memory_order_relaxed);
			atomicG[g].fetch_add(1, std::memory_order_relaxed);
			atomicB[b].fetch_add(1, std::memory_order_relaxed);
		});
}

inline void ProcessRowsLocal(const uint8_t* data,
	const int width,
	const int startRow,
	const int endRow,
	LocalCounters& local)
{
	ProcessRows(data, width, startRow, endRow,
		[&local](const uint8_t r, const uint8_t g, const uint8_t b) {
			local.g[r]++;
			local.r[g]++;
			local.b[b]++;
		});
}
