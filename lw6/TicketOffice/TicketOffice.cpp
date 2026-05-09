#include "TicketOffice.h"

#include <stdexcept>

TicketOffice::TicketOffice(const int numTickets)
	: m_numTickets(numTickets)
{
	if (numTickets < 0)
	{
		throw std::invalid_argument("numTickets must be non-negative");
	}
}

int TicketOffice::SellTickets(const int ticketsToBuy)
{
	if (ticketsToBuy <= 0)
	{
		throw std::invalid_argument("Can not buy negative amount of tickets");
	}

	int available = m_numTickets.load(std::memory_order_relaxed);
	while (true)
	{
		if (available == 0)
		{
			return 0;
		}
		const int toSell = std::min(ticketsToBuy, available);

		if (m_numTickets.compare_exchange_weak(
				available, available - toSell,
				std::memory_order_relaxed, std::memory_order_relaxed))
		{
			return toSell;
		}
	}
}

int TicketOffice::GetTicketsLeft() const noexcept
{
	return m_numTickets.load(std::memory_order_relaxed);
}
