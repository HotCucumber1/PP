#include "BitonicSorter.h"


std::vector<int> GenerateRandomArray(const size_t size)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(1, size);

	std::vector<int> arr(size);
	for (size_t i = 0; i < size; i++)
	{
		arr[i] = dis(gen);
	}
	return arr;
}

bool IsSorted(const std::vector<int>& arr)
{
	for (size_t i = 1; i < arr.size(); i++)
	{
		if (arr[i] < arr[i - 1])
		{
			return false;
		}
	}
	return true;
}

int main()
{
	const std::vector<size_t> testSizes = { 1000, 10000, 100000, 500000, 1000000 };

	for (const auto& size : testSizes)
	{
		std::cout << "\n--- Testing size: " << size << " ---" << std::endl;

		const auto original = GenerateRandomArray(size);


		std::vector<int> gpuData = original;
		BitonicSorter sorter;

		auto gpuStart = std::chrono::high_resolution_clock::now();
		sorter.Sort(gpuData);
		auto gpuEnd = std::chrono::high_resolution_clock::now();
		const double gpuTime = std::chrono::duration<double, std::milli>(gpuEnd - gpuStart).count();



		std::vector<int> cpuData = original;
		auto cpuStart = std::chrono::high_resolution_clock::now();
		std::ranges::sort(cpuData);
		auto cpuEnd = std::chrono::high_resolution_clock::now();
		const double cpuTime = std::chrono::duration<double, std::milli>(cpuEnd - cpuStart).count();



		const bool correct = (gpuData == cpuData) && IsSorted(gpuData);

		std::cout << "CPU time: " << std::fixed << std::setprecision(2) << cpuTime << " ms" << std::endl;
		std::cout << "GPU time: " << gpuTime << " ms" << std::endl;
		std::cout << "Speedup: " << (cpuTime / gpuTime) << "x" << std::endl;
		std::cout << "Result: " << (correct ? "CORRECT" : "INCORRECT") << std::endl;
	}

	return 0;
}
