#include "../AtomicMax.h"
#include "../AtomicMaxWithLock.h"

#include <catch2/catch_all.hpp>
#include <iostream>
#include <thread>

template <typename MaxType>
void RunBenchmark(const int numThreads, int updatesPerThread)
{
	MaxType maxInstance(0);
	std::vector<std::thread> threads;

	for (int t = 0; t < numThreads; ++t)
	{
		threads.emplace_back([&maxInstance, updatesPerThread, t]() {
			for (int i = 0; i < updatesPerThread; ++i)
			{
				int value = t * updatesPerThread + i;
				maxInstance.Update(value);
				if (i % 10 == 0)
				{
					maxInstance.Update(value + 1);
				}
			}
		});
	}

	for (auto& th : threads)
	{
		th.join();
	}
}

TEST_CASE("Performance comparison", "[benchmark]")
{
	RunBenchmark<AtomicMax<int>>(2, 1000);
	RunBenchmark<AtomicMaxWithLock<int>>(2, 1000);

	for (int threads = 1; threads <= 30; threads++)
	{
		constexpr int totalUpdates = 200000;
		int updatesPerThread = totalUpdates / threads;

		BENCHMARK("Lock-free " + std::to_string(threads) + " threads")
		{
			RunBenchmark<AtomicMax<int>>(threads, updatesPerThread);
		};

		BENCHMARK("Lock-based " + std::to_string(threads) + " threads")
		{
			RunBenchmark<AtomicMaxWithLock<int>>(threads, updatesPerThread);
		};
		std::cout << std::endl
				  << "-------------------------------------------------------------------------------" << std::endl;
	}
}
