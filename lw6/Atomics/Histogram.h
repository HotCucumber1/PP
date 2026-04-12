#pragma once
#include "Image.h"

#include <vector>

void SequentialHistogram(
	const Image& image,
	std::vector<float>& histR,
	std::vector<float>& histG,
	std::vector<float>& histB);

void AtomicHistogramInterleaved(
	const Image& image,
	int numThreads,
	std::vector<float>& histR,
	std::vector<float>& histG,
	std::vector<float>& histB);

void AtomicHistogramSeparate(
	const Image& image,
	int numThreads,
	std::vector<float>& histR,
	std::vector<float>& histG,
	std::vector<float>& histB);

void LocalHistogram(
	const Image& image,
	int numThreads,
	std::vector<float>& histR,
	std::vector<float>& histG,
	std::vector<float>& histB);

void ParallelForEachHistogram(
	const Image& image,
	std::vector<float>& histR,
	std::vector<float>& histG,
	std::vector<float>& histB);
