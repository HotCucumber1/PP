#pragma once
#include <coroutine>
#include <liburing.h>
#include <stdexcept>

enum class OpenMode
{
	Read,
	Write
};

struct IoOperation
{
	std::coroutine_handle<> handle = nullptr;
	int result = 0;
};

class Dispatcher
{
public:
	explicit Dispatcher(const unsigned entries = 256)
	{
		if (io_uring_queue_init(entries, &m_ring, 0) < 0)
		{
			throw std::runtime_error("Failed to initialize io_uring");
		}
	}

	~Dispatcher()
	{
		io_uring_queue_exit(&m_ring);
	}

	io_uring* GetRing()
	{
		return &m_ring;
	}

	void Submit()
	{
		io_uring_submit(&m_ring);
		m_activeOps++;
	}

	void Poll()
	{
		io_uring_cqe* cqe;
		if (io_uring_wait_cqe(&m_ring, &cqe) == 0)
		{
			auto* op = static_cast<IoOperation*>(io_uring_cqe_get_data(cqe));
			op->result = cqe->res;

			io_uring_cqe_seen(&m_ring, cqe);
			m_activeOps--;

			if (op->handle)
			{
				op->handle.resume();
			}
		}
	}

	bool HasActiveOps() const
	{
		return m_activeOps > 0;
	}

private:
	io_uring m_ring{};
	int m_activeOps = 0;
};
