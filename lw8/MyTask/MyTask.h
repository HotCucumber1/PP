#pragma once
#include <coroutine>
#include <exception>
#include <string>
#include <utility>

class MyTask
{
public:
	struct promise_type
	{
		std::string res;

		MyTask get_return_object()
		{
			return MyTask(
				std::coroutine_handle<promise_type>::from_promise(*this));
		}

		void return_value(std::string value)
		{
			res = std::move(value);
		}

		std::suspend_never initial_suspend() noexcept
		{
			return {};
		}

		std::suspend_always final_suspend() noexcept
		{
			return {};
		}

		void unhandled_exception()
		{
			std::terminate();
		}
	};

	using HandleType = std::coroutine_handle<promise_type>;

	explicit MyTask(const HandleType handle)
		: m_handle(handle)
	{
	}

	~MyTask()
	{
		if (m_handle)
		{
			m_handle.destroy();
		}
	}

	MyTask(const MyTask&) = delete;
	MyTask& operator=(const MyTask&) = delete;

	MyTask(MyTask&& other) noexcept
		: m_handle(std::exchange(other.m_handle, nullptr))
	{
	}

	MyTask& operator=(MyTask&& other) noexcept
	{
		if (this != &other)
		{
			if (m_handle)
			{
				m_handle.destroy();
			}
			m_handle = std::exchange(other.m_handle, nullptr);
		}
		return *this;
	}

	std::string GetResult() const
	{
		if (m_handle)
		{
			return m_handle.promise().res;
		}
		return "";
	}

private:
	HandleType m_handle;
};
