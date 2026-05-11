#pragma once
#include <atomic>
#include <memory>

class StopToken;

class StopSource
{
public:
	StopSource()
		: m_state(std::make_shared<std::atomic<bool>>(false))
	{
	}

	void RequestStop() const
	{
		m_state->store(true, std::memory_order_release);
	}

	StopToken GetToken() const;

private:
	std::shared_ptr<std::atomic<bool>> m_state;
};

class StopToken
{
public:
	StopToken()
		: m_state(nullptr)
	{
	}

	bool StopRequested() const
	{
		if (!m_state)
		{
			return false;
		}
		return m_state->load(std::memory_order_acquire);
	}

private:
	friend class StopSource;

	explicit StopToken(std::shared_ptr<std::atomic<bool>> state)
		: m_state(std::move(state))
	{
	}

	std::shared_ptr<std::atomic<bool>> m_state;
};

inline StopToken StopSource::GetToken() const
{
	return StopToken(m_state);
}
