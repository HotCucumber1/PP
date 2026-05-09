#pragma once
#include <mutex>

class TicketOfficeWithLock
{
public:
	explicit TicketOfficeWithLock(int numTickets);

	TicketOfficeWithLock(const TicketOfficeWithLock&) = delete;
	TicketOfficeWithLock& operator=(const TicketOfficeWithLock&) = delete;

	/**
	 * Выполняет продажу билета.
	 * Возвращает количество фактически проданных билетов.
	 * Если ticketsToBuy <= 0, выбрасывается исключение std::invalid_argument
	 */
	int SellTickets(int ticketsToBuy);

	int GetTicketsLeft() const noexcept;

private:
	mutable std::mutex m_mutex;
	int m_numTickets;
};
