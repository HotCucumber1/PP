#pragma once
#include <boost/lockfree/queue.hpp>
#include <functional>
#include <thread>

class LFThreadPool
{
public:
	explicit LFThreadPool(int numThreads);

	~LFThreadPool();

	LFThreadPool(const LFThreadPool&) = delete;
	LFThreadPool& operator=(const LFThreadPool&) = delete;

	bool Submit(std::function<void()> task);

private:
	void WorkerThread();

	boost::lockfree::queue<std::function<void()>*> m_workersQueue;
	std::vector<std::thread> m_threads;
	std::atomic<bool> m_stopFlag;
};
