#include "TicketOfficeWithLock.h"

#include <stdexcept>

TicketOfficeWithLock::TicketOfficeWithLock(const int numTickets)
	: m_numTickets(numTickets)
{
	if (numTickets < 0)
	{
		throw std::invalid_argument("numTickets must be non-negative");
	}
}

int TicketOfficeWithLock::SellTickets(const int ticketsToBuy)
{
	if (ticketsToBuy <= 0)
	{
		throw std::invalid_argument("ticketsToBuy must be positive");
	}
	std::lock_guard lock(m_mutex);
	const int toSell = std::min(m_numTickets, ticketsToBuy);
	m_numTickets -= toSell;

	return toSell;
}

int TicketOfficeWithLock::GetTicketsLeft() const noexcept
{
	std::lock_guard lock(m_mutex);
	return m_numTickets;
}
