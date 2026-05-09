#include "../AtomicMax.h"
#include <catch2/catch_all.hpp>
#include <thread>

TEST_CASE("AtomicMax single-threaded", "[AtomicMax]")
{
	SECTION("Initial value")
	{
		AtomicMax max(42);
		REQUIRE(max.GetValue() == 42);
	}

	SECTION("Update with larger value")
	{
		AtomicMax max(10);
		max.Update(20);
		REQUIRE(max.GetValue() == 20);
	}

	SECTION("Update with smaller value does nothing")
	{
		AtomicMax max(100);
		max.Update(50);
		REQUIRE(max.GetValue() == 100);
	}

	SECTION("Multiple updates")
	{
		AtomicMax max(0);
		max.Update(5);
		max.Update(3);
		max.Update(7);
		max.Update(6);
		REQUIRE(max.GetValue() == 7);
	}

	SECTION("Negative numbers")
	{
		AtomicMax max(-100);
		max.Update(-50);
		max.Update(-200);
		REQUIRE(max.GetValue() == -50);
	}

	SECTION("Floating point")
	{
		AtomicMax max(1.5);
		max.Update(2.7);
		max.Update(1.9);
		REQUIRE(max.GetValue() == 2.7);
	}

	SECTION("Unsigned type")
	{
		AtomicMax<uint64_t> max(100);
		max.Update(200);
		max.Update(50);
		REQUIRE(max.GetValue() == 200);
	}
}

TEST_CASE("AtomicMax multi-threaded correctness", "[AtomicMax]")
{
	constexpr int numThreads = 8;
	constexpr int updatesPerThread = 10000;

	AtomicMax max(-1);
	std::vector<std::thread> threads;

	std::atomic ready{ false };
	std::atomic startCount{ 0 }; // TODO для чего это

	for (int t = 0; t < numThreads; ++t)
	{
		threads.emplace_back([&max, t, &ready, &startCount]() {
			startCount.fetch_add(1, std::memory_order_acquire);
			while (!ready.load(std::memory_order_acquire))
			{
				std::this_thread::yield();
			}

			for (int i = 0; i < updatesPerThread; ++i)
			{
				const int value = t * updatesPerThread + i;
				max.Update(value);
			}
		});
	}

	while (startCount.load(std::memory_order_acquire) < numThreads)
	{
		std::this_thread::yield();
	}
	ready.store(true, std::memory_order_release);

	for (auto& th : threads)
	{
		th.join();
	}
	constexpr int expectedMax = (numThreads - 1) * updatesPerThread + (updatesPerThread - 1);
	REQUIRE(max.GetValue() == expectedMax);
}

TEST_CASE("AtomicMax concurrent same value", "[AtomicMax]")
{
	constexpr int numThreads = 16;
	constexpr int iterations = 5000;

	AtomicMax max(0);
	std::vector<std::thread> threads;

	for (int t = 0; t < numThreads; ++t)
	{
		threads.emplace_back([&max]() {
			for (int i = 0; i < iterations; ++i)
			{
				max.Update(100);
				max.Update(i % 10);
			}
		});
	}

	for (auto& th : threads)
	{
		th.join();
	}

	REQUIRE(max.GetValue() == 100);
}
