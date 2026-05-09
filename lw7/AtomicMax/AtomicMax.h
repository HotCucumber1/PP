#pragma once
#include <atomic>

template <typename T>
class AtomicMax
{
public:
	explicit AtomicMax(T value)
		: m_value(value)
	{
	}

	void Update(T newValue) noexcept
	{
		auto current = GetValue();
		while (newValue > current)
		{
			if (m_value.compare_exchange_weak(
					current, newValue,
					std::memory_order_relaxed, std::memory_order_relaxed))
			{
				break;
			}
		}
	}

	T GetValue() const noexcept
	{
		return m_value.load(std::memory_order_relaxed);
	}

private:
	std::atomic<T> m_value;
};
