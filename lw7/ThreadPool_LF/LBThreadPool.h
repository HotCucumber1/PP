#pragma once
#include <condition_variable>
#include <functional>
#include <queue>

class LBThreadPool
{
public:
	explicit LBThreadPool(int numThreads);

	~LBThreadPool();

	LBThreadPool(const LBThreadPool&) = delete;
	LBThreadPool& operator=(const LBThreadPool&) = delete;

	void Submit(std::function<void()> task);

private:
	void WorkerThread();

	std::vector<std::thread> m_threads;
	std::queue<std::function<void()>> m_tasks;
	std::mutex m_mutex;
	std::condition_variable m_cv;
	bool m_stopFlag;
};
