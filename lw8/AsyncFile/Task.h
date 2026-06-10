#pragma once
#include <coroutine>
#include <exception>

class Task
{
public:
	struct promise_type
	{
		std::coroutine_handle<> continuation = nullptr;

		Task get_return_object()
		{
			return Task{ std::coroutine_handle<promise_type>::from_promise(*this) };
		}

		std::suspend_never initial_suspend() noexcept
		{
			return {};
		}

		auto final_suspend() noexcept
		{
			struct FinalAwaiter
			{
				bool await_ready() noexcept
				{
					return false;
				}

				void await_suspend(const std::coroutine_handle<promise_type> handle) noexcept
				{
					if (handle.promise().continuation)
					{
						handle.promise().continuation.resume();
					}
				}

				void await_resume() noexcept
				{
				}
			};
			return FinalAwaiter{};
		}

		void return_void() noexcept
		{
		}

		void unhandled_exception()
		{
			std::terminate();
		}
	};

	explicit Task(const std::coroutine_handle<promise_type> h)
		: handle(h)
	{
	}

	~Task()
	{
		if (handle)
		{
			handle.destroy();
		}
	}

	Task(const Task&) = delete;

	Task(Task&& other) noexcept
		: handle(other.handle)
	{
		other.handle = nullptr;
	}

	bool await_ready() const noexcept
	{
		return handle.done();
	}

	void await_suspend(const std::coroutine_handle<> caller) const noexcept
	{
		handle.promise().continuation = caller;
	}

	void await_resume() noexcept
	{
	}

private:
	std::coroutine_handle<promise_type> handle;
};