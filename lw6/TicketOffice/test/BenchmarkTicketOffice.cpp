#include "../TicketOffice.h"
#include "../TicketOfficeWithLock.h"

#include <catch2/catch_all.hpp>
#include <iostream>

#include <thread>
#include <vector>

constexpr int TOTAL_TICKETS = 1000000;

template <typename OfficeType>
void RunMultithreaded(OfficeType& office, const int numThreads, const int opsPerThread)
{
	std::vector<std::jthread> threads;
	for (int i = 0; i < numThreads; ++i)
	{
		threads.emplace_back([&office, opsPerThread]() {
			for (int j = 0; j < opsPerThread; ++j)
			{
				office.SellTickets(1);
			}
		});
	}
}

TEST_CASE("Ticket office benchmark")
{
	for (int numThreads = 1; numThreads <= 30; numThreads++)
	{
		TicketOffice office(TOTAL_TICKETS);
		TicketOfficeWithLock officeWithLock(TOTAL_TICKETS);
		BENCHMARK("Lock-free. Threads=" + std::to_string(numThreads))
		{
			RunMultithreaded(office, numThreads, TOTAL_TICKETS / numThreads);
		};

		BENCHMARK("With Lock. Threads=" + std::to_string(numThreads))
		{
			RunMultithreaded(officeWithLock, numThreads, TOTAL_TICKETS / numThreads);
		};
		std::cout << std::endl << "-------------------------------------------------------------------------------" << std::endl;
	}
}
