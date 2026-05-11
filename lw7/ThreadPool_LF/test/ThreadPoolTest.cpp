#include "../LFThreadPool.h"
#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <random>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

TEST_CASE("LFThreadPool submits and executes single task", "[LFThreadPool]")
{
	LFThreadPool pool(2);
	std::atomic executed = false;

	const bool result = pool.Submit([&executed]() {
		executed = true;
	});

	REQUIRE(result == true);

	std::this_thread::sleep_for(100ms);
	REQUIRE(executed == true);
}

TEST_CASE("LFThreadPool executes multiple tasks", "[LFThreadPool]")
{
	LFThreadPool pool(4);
	std::atomic counter = 0;
	constexpr int TASKS_COUNT = 100;

	for (int i = 0; i < TASKS_COUNT; ++i)
	{
		pool.Submit([&counter]() {
			++counter;
		});
	}

	std::this_thread::sleep_for(500ms);
	REQUIRE(counter == TASKS_COUNT);
}

TEST_CASE("LFThreadPool rejects null task", "[LFThreadPool]")
{
	LFThreadPool pool(2);
	const std::function<void()> nullTask = nullptr;

	const bool result = pool.Submit(nullTask);
	REQUIRE(result == false);
}

TEST_CASE("LFThreadPool processes tasks in parallel", "[LFThreadPool]")
{
	LFThreadPool pool(4);
	std::atomic concurrentCount = 0;
	std::atomic maxConcurrent = 0;
	std::atomic completedTasks = 0;
	constexpr int TASKS_COUNT = 8;

	for (int i = 0; i < TASKS_COUNT; ++i)
	{
		pool.Submit([&]() {
			const int current = ++concurrentCount;
			int oldMax = maxConcurrent.load();
			while (current > oldMax && !maxConcurrent.compare_exchange_weak(oldMax, current))
			{
			}

			std::this_thread::sleep_for(50ms);
			--concurrentCount;
			++completedTasks;
		});
	}

	std::this_thread::sleep_for(1s);

	REQUIRE(completedTasks == TASKS_COUNT);
	REQUIRE(maxConcurrent >= 2);
}

TEST_CASE("LFThreadPool handles heavy load", "[LFThreadPool]")
{
	LFThreadPool pool(8);
	std::atomic counter = 0;
	constexpr int TASKS_COUNT = 1000;

	for (int i = 0; i < TASKS_COUNT; ++i)
	{
		pool.Submit([&counter]() {
			++counter;
		});
	}

	std::this_thread::sleep_for(2s);
	REQUIRE(counter == TASKS_COUNT);
}

TEST_CASE("LFThreadPool destructor waits for tasks completion", "[LFThreadPool]")
{
	std::atomic executedTasks = 0;
	constexpr int TASKS_COUNT = 50;

	{
		LFThreadPool pool(4);

		for (int i = 0; i < TASKS_COUNT; ++i)
		{
			pool.Submit([&executedTasks, i]() {
				std::this_thread::sleep_for(10ms);
				++executedTasks;
			});
		}
	}

	REQUIRE(executedTasks == TASKS_COUNT);
}

TEST_CASE("LFThreadPool respects queue size limit", "[LFThreadPool]")
{
	LFThreadPool pool(2);
	std::atomic executed = 0;
	std::atomic submitted = 0;
	constexpr int TASKS_COUNT = 2000;

	for (int i = 0; i < TASKS_COUNT; ++i)
	{
		pool.Submit([&executed]() {
			std::this_thread::sleep_for(1ms);
			++executed;
		});
		++submitted;

		if (i % 100 == 0)
		{
			std::this_thread::sleep_for(1ms);
		}
	}

	std::this_thread::sleep_for(3s);
	REQUIRE(executed == submitted);
}

TEST_CASE("LFThreadPool maintains task order", "[LFThreadPool]")
{
	LFThreadPool pool(1);
	std::vector<int> executionOrder;
	std::mutex orderMutex;
	constexpr int TASKS_COUNT = 50;

	for (int i = 0; i < TASKS_COUNT; ++i)
	{
		pool.Submit([&executionOrder, &orderMutex, i]() {
			std::lock_guard lock(orderMutex);
			executionOrder.push_back(i);
		});
	}

	std::this_thread::sleep_for(1s);

	REQUIRE(executionOrder.size() == TASKS_COUNT);
	for (int i = 0; i < TASKS_COUNT; ++i)
	{
		REQUIRE(executionOrder[i] == i);
	}
}

TEST_CASE("LFThreadPool works with move-only tasks", "[LFThreadPool]")
{
	LFThreadPool pool(2);
	std::atomic value = 0;

	auto task = [&value]() {
		value = 42;
	};

	pool.Submit(std::move(task));
	std::this_thread::sleep_for(100ms);
	REQUIRE(value == 42);
}

TEST_CASE("LFThreadPool continues working after queue overflow attempts", "[LFThreadPool]")
{
	LFThreadPool pool(1);
	std::atomic executed = 0;
	constexpr int TASKS_COUNT = 1500;

	for (int i = 0; i < TASKS_COUNT; ++i)
	{
		pool.Submit([&executed, i]() {
			std::this_thread::sleep_for(5ms);
			++executed;
		});

		if (i % 10 == 0)
		{
			std::this_thread::sleep_for(1ms);
		}
	}

	std::this_thread::sleep_for(10s);
	REQUIRE(executed == TASKS_COUNT);
}

TEST_CASE("LFThreadPool handles zero threads gracefully", "[LFThreadPool]")
{
	LFThreadPool pool(0);
	std::atomic executed = false;

	const bool result = pool.Submit([&executed]() {
		executed = true;
	});

	REQUIRE(result == true);
	std::this_thread::sleep_for(100ms);
	REQUIRE(executed == false);
}
