#pragma once
#include <atomic>
#include <condition_variable>

class Warehouse
{
public:
	explicit Warehouse(size_t capacity);
	~Warehouse();

	Warehouse(const Warehouse&) = delete;
	Warehouse& operator=(const Warehouse&) = delete;

	bool AddGoods(size_t amount);

	bool RemoveGoods(size_t amount);

	size_t GetCurrentStock() const;

	size_t GetTotalSupplied() const
	{
		return m_totalSupplied.load();
	}

	size_t GetTotalPurchased() const
	{
		return m_totalPurchased.load();
	}

	void Shutdown();

private:
	const size_t m_capacity;
	size_t m_currentStock;

	std::atomic<size_t> m_totalSupplied = 0;
	std::atomic<size_t> m_totalPurchased = 0;
	std::atomic<bool> m_running = true;

	mutable std::mutex m_mutex;
	std::condition_variable m_cv;
};