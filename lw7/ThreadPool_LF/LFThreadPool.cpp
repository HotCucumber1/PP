#include "LFThreadPool.h"

#include <iostream>

constexpr int QUEUE_SiZE = 1024;

LFThreadPool::LFThreadPool(const int numThreads)
	: m_workersQueue(QUEUE_SiZE)
	, m_stopFlag(false)
{
	for (int i = 0; i < numThreads; i++)
	{
		m_threads.emplace_back(&LFThreadPool::WorkerThread, this);
	}
}

LFThreadPool::~LFThreadPool()
{
	m_stopFlag.store(true, std::memory_order_relaxed);

	for (int i = 0; i < m_threads.size(); ++i)
	{
		m_workersQueue.bounded_push(nullptr);
	}

	for (auto& t : m_threads)
	{
		t.join();
	}
}

bool LFThreadPool::Submit(std::function<void()> task)
{
	if (!task)
	{
		return false;
	}

	auto taskPtr = std::make_unique<std::function<void()>>(std::move(task));
	while (!m_workersQueue.bounded_push(taskPtr.get()))
	{
		if (m_stopFlag.load(std::memory_order_relaxed))
		{
			taskPtr.release();
			return false;
		}
		std::this_thread::yield();
	}
	taskPtr.release();
	return true;
}

void LFThreadPool::WorkerThread()
{
	while (true)
	{
		std::function<void()>* task = nullptr;
		if (m_workersQueue.pop(task))
		{
			if (task == nullptr)
			{
				break;
			}
			std::unique_ptr<std::function<void()>> safeTask(task);
			try
			{
				(*safeTask)();
			}
			catch (const std::exception& e)
			{
				std::cout << e.what() << std::endl;
			}
		}
		else
		{
			std::this_thread::yield();
		}
	}
}
