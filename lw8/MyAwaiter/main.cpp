#include <coroutine>
#include <iostream>

struct MyAwaiter
{
	int a;
	int b;

	static bool await_ready() noexcept
	{
		return false;
	}

	static void await_suspend(std::coroutine_handle<>) noexcept
	{
	}

	int await_resume() const noexcept
	{
		return a + b;
	}
};

class MyTask
{
public:
	struct promise_type
	{
		MyTask get_return_object()
		{
			return MyTask{ std::coroutine_handle<promise_type>::from_promise(*this) };
		}

		static std::suspend_never initial_suspend() noexcept
		{
			return {};
		}

		static std::suspend_always final_suspend() noexcept
		{
			return {};
		}

		static void return_void() noexcept
		{
		}

		static void unhandled_exception()
		{
			std::terminate();
		}
	};

	using HandleType = std::coroutine_handle<promise_type>;

	explicit MyTask(const HandleType h)
		: m_handle(h)
	{
	}

	~MyTask()
	{
		if (m_handle)
		{
			m_handle.destroy();
		}
	}

	void Resume() const
	{
		if (m_handle && !m_handle.done())
		{
			m_handle.resume();
		}
	}

	MyTask(const MyTask&) = delete;
	MyTask& operator=(const MyTask&) = delete;

	MyTask(MyTask&& other) noexcept
		: m_handle(other.m_handle)
	{
		other.m_handle = nullptr;
	}

	MyTask& operator=(MyTask&& other) noexcept
	{
		if (this != &other)
		{
			m_handle = other.m_handle;
		}
		other.m_handle = nullptr;
		return *this;
	}

private:
	HandleType m_handle;
};

MyTask CoroutineWithAwait(const int x, const int y)
{
	std::cout << "Before await\n";
	const int result = co_await MyAwaiter{ x, y };
	std::cout << result << "\n";
	std::cout << "After await\n";
}

int main()
{
	const auto task = CoroutineWithAwait(30, 12);
	std::cout << "Before resume\n";
	task.Resume();
	std::cout << "After resume\n";
	CoroutineWithAwait(5, 10).Resume();
	std::cout << "End of main\n";
}
