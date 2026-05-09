#pragma once
#include <atomic>

template <typename T>
class AtomicMaxWithLock
{
public:
	explicit AtomicMaxWithLock(T value) noexcept
		: m_value(value)
	{
	}

	void Update(T newValue) noexcept
	{
		std::lock_guard lock(m_mutex);
		if (newValue > m_value)
		{
			m_value = newValue;
		}
	}

	T GetValue() const noexcept
	{
		std::lock_guard lock(m_mutex);
		return m_value;
	}

private:
	mutable std::mutex m_mutex;
	T m_value;
};
