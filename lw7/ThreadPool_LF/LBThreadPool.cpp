#include "LBThreadPool.h"

LBThreadPool::LBThreadPool(const int numThreads)
	: m_stopFlag(false)
{
	for (size_t i = 0; i < numThreads; ++i)
	{
		m_threads.emplace_back(&LBThreadPool::WorkerThread, this);
	}
}

LBThreadPool::~LBThreadPool()
{
	{
		std::lock_guard lock(m_mutex);
		m_stopFlag = true;
	}
	m_cv.notify_all();

	for (auto& t : m_threads)
	{
		t.join();
	}
}

void LBThreadPool::Submit(std::function<void()> task)
{
	{
		std::lock_guard lock(m_mutex);
		m_tasks.push(std::move(task));
	}
	m_cv.notify_one();
}

void LBThreadPool::WorkerThread()
{
	while (true)
	{
		std::function<void()> task;
		{
			std::unique_lock lock(m_mutex);
			m_cv.wait(lock, [this] {
				return m_stopFlag || !m_tasks.empty();
			});

			if (m_stopFlag && m_tasks.empty())
			{
				return;
			}

			task = std::move(m_tasks.front());
			m_tasks.pop();
		}
		task();
	}
}
