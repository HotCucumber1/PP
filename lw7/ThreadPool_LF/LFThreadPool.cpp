#include "LFThreadPool.h"

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
		m_workersQueue.push(nullptr); // TODO точно ли?
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
	auto deleter = [](const std::function<void()>* f) {
		(*f)();
		delete f;
	}; // TODO че это
	auto ptr = std::unique_ptr<
		std::function<void()>,
		decltype(deleter)>(
		new std::function(std::move(task)),
		deleter);

	while (!m_workersQueue.push(ptr.release()))
	{
		std::this_thread::yield();
	}
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
			(*task)();
			delete task;
		}
		else
		{
			std::this_thread::yield();
			if (m_stopFlag.load(std::memory_order_acquire))
			{
				break;
			}
		}
	}
}
