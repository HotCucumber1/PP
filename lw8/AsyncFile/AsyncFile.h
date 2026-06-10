#pragma once
#include "Dispatcher.h"
#include <unistd.h>

class AsyncFile
{
public:
	AsyncFile() = default;

	explicit AsyncFile(const int fd)
		: m_fd(fd)
	{
	}

	AsyncFile(const AsyncFile&) = delete;
	AsyncFile& operator=(const AsyncFile&) = delete;

	AsyncFile(AsyncFile&& other) noexcept
		: m_fd(other.m_fd)
		, m_offset(other.m_offset)
	{
		other.m_fd = -1;
		other.m_offset = 0;
	}

	~AsyncFile()
	{
		if (m_fd >= 0)
		{
			close(m_fd);
		}
	}

	auto ReadAsync(Dispatcher& disp, char* buffer, const size_t size)
	{
		struct ReadAwaiter : IoOperation
		{
			Dispatcher& disp;
			AsyncFile* file{};
			char* buf{};
			size_t size{};

			bool await_ready() noexcept
			{
				return false;
			}

			void await_suspend(const std::coroutine_handle<> h)
			{
				handle = h;
				auto* sqe = io_uring_get_sqe(disp.GetRing());
				io_uring_prep_read(sqe, file->m_fd, buf, size, file->m_offset);
				io_uring_sqe_set_data(sqe, this);
				disp.Submit();
			}

			unsigned await_resume() const
			{
				if (result < 0)
				{
					throw std::runtime_error("Read failed");
				}
				file->m_offset += result;
				return static_cast<unsigned>(result);
			}
		};
		return ReadAwaiter{ {}, disp, this, buffer, size };
	}

	auto AsyncWrite(Dispatcher& disp, const char* buffer, const size_t size)
	{
		struct WriteAwaiter : IoOperation
		{
			Dispatcher& disp;
			AsyncFile* file{};
			const char* buf{};
			size_t size{};

			static bool await_ready() noexcept
			{
				return false;
			}

			void await_suspend(const std::coroutine_handle<> h)
			{
				handle = h;
				io_uring_sqe* sqe = io_uring_get_sqe(disp.GetRing());
				io_uring_prep_write(sqe, file->m_fd, buf, size, file->m_offset);
				io_uring_sqe_set_data(sqe, this);
				disp.Submit();
			}

			void await_resume() const
			{
				if (result < 0)
				{
					throw std::runtime_error("Write failed");
				}
				file->m_offset += result;
			}
		};
		return WriteAwaiter{ {}, disp, this, buffer, size };
	}

private:
	int m_fd = -1;
	off_t m_offset = 0;
};

inline auto AsyncOpenFile(
	Dispatcher& disp,
	std::string path,
	const OpenMode mode)
{
	struct OpenAwaiter : IoOperation
	{
		Dispatcher& disp;
		std::string path;
		OpenMode mode;

		bool await_ready() noexcept
		{
			return false;
		}

		void await_suspend(std::coroutine_handle<> h)
		{
			handle = h;
			io_uring_sqe* sqe = io_uring_get_sqe(disp.GetRing());

			const int flags = (mode == OpenMode::Read)
				? O_RDONLY
				: (O_WRONLY | O_CREAT | O_TRUNC);

			io_uring_prep_openat(sqe, AT_FDCWD, path.c_str(), flags, 0644);
			io_uring_sqe_set_data(sqe, this);
			disp.Submit();
		}

		AsyncFile await_resume() const
		{
			if (result < 0)
			{
				throw std::runtime_error("Failed to open file: " + path);
			}
			return AsyncFile{ result };
		}
	};
	return OpenAwaiter{ {}, disp, std::move(path), mode };
}