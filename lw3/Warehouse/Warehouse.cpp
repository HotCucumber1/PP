#include "Warehouse.h"

Warehouse::Warehouse(const size_t capacity)
	: m_capacity(capacity)
	, m_currentStock(0)
{
}

Warehouse::~Warehouse()
{
	Shutdown();
}

bool Warehouse::AddGoods(size_t amount)
{
	std::unique_lock lock(m_mutex);

	const auto result = m_cv.wait_for(lock, std::chrono::milliseconds(100), [this, amount]() {
		return !m_running || (m_currentStock + amount <= m_capacity);
	});

	if (!m_running || !result)
	{
		return false;
	}

	if (m_currentStock + amount <= m_capacity)
	{
		m_currentStock += amount;
		m_totalSupplied += amount;

		m_cv.notify_all();
		return true;
	}

	return false;
}

bool Warehouse::RemoveGoods(size_t amount)
{
	std::unique_lock lock(m_mutex);

	const auto result = m_cv.wait_for(lock, std::chrono::milliseconds(100), [this, amount]() {
		return !m_running || (m_currentStock >= amount);
	});

	if (!m_running || !result)
	{
		return false;
	}

	if (m_currentStock >= amount)
	{
		m_currentStock -= amount;
		m_totalPurchased += amount;

		m_cv.notify_all();
		return true;
	}

	return false;
}

size_t Warehouse::GetCurrentStock() const
{
	std::lock_guard lock(m_mutex);
	return m_currentStock;
}

void Warehouse::Shutdown()
{
	m_running = false;
	m_cv.notify_all();
}