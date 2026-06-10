#include "LFThreadPool.h"
#include <atomic>
#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <boost/lockfree/stack.hpp>

constexpr int TASKS_COUNT = 1000;

double BenchmarkLFThreadPool(const int numThreads)
{
	LFThreadPool pool(numThreads);
	std::atomic completed = 0;

	const auto start = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < TASKS_COUNT; ++i)
	{
		pool.Submit([&completed]() {
			volatile double sum = 0;
			for (int j = 0; j < 10000; ++j)
			{
				sum += j * 3.14;
			}
			++completed;
		});
	}

	while (completed.load() != TASKS_COUNT)
	{
		std::this_thread::yield();
	}

	const auto end = std::chrono::high_resolution_clock::now();
	return std::chrono::duration<double>(end - start).count();
}

double BenchmarkAsioThreadPool(const int numThreads)
{
	boost::asio::thread_pool pool(numThreads);
	std::atomic completed = 0;

	const auto start = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < TASKS_COUNT; ++i)
	{
		boost::asio::post(pool, [&completed]() {
			volatile double sum = 0;
			for (int j = 0; j < 10000; ++j)
			{
				sum += j * 3.14;
			}
			++completed;
		});
	}

	while (completed.load() != TASKS_COUNT)
	{
		std::this_thread::yield();
	}

	pool.join();

	const auto end = std::chrono::high_resolution_clock::now();
	return std::chrono::duration<double>(end - start).count();
}

int main()
{
	std::cout << std::fixed << std::setprecision(6);
	std::cout << "Threads,Time" << std::endl;

	for (int numThreads = 1; numThreads <= 30; ++numThreads)
	{
		const double time = BenchmarkLFThreadPool(numThreads);
		std::cout << numThreads << "," << time << std::endl;
	}

	std::cout << std::endl;
	std::cout << "Threads,Time" << std::endl;
	for (int numThreads = 1; numThreads <= 30; ++numThreads)
	{
		const double time = BenchmarkAsioThreadPool(numThreads);
		std::cout << numThreads << "," << time << std::endl;
	}

	return 0;
}
