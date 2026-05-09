#include "../TicketOffice.h"
#include <catch2/catch_all.hpp>

#include <thread>

TEST_CASE("Single-threaded operations", "[TicketOffice]")
{
	TicketOffice office(10);
	REQUIRE(office.GetTicketsLeft() == 10);

	REQUIRE(office.SellTickets(3) == 3);
	REQUIRE(office.GetTicketsLeft() == 7);

	REQUIRE(office.SellTickets(10) == 7);
	REQUIRE(office.GetTicketsLeft() == 0);

	REQUIRE(office.SellTickets(1) == 0);
	REQUIRE(office.GetTicketsLeft() == 0);
}

TEST_CASE("Negative or zero ticketsToBuy throws", "[TicketOffice]")
{
	TicketOffice office(5);
	REQUIRE_THROWS_AS(office.SellTickets(0), std::invalid_argument);
	REQUIRE_THROWS_AS(office.SellTickets(-1), std::invalid_argument);
}

TEST_CASE("Multi-threaded total sales are correct", "[TicketOffice]")
{
	constexpr int totalTickets = 1000;
	constexpr int threadCount = 16;
	constexpr int attemptsPerThread = 500;

	TicketOffice office(totalTickets);
	std::vector<std::thread> threads;
	std::atomic totalSold{ 0 };

	for (int i = 0; i < threadCount; ++i)
	{
		threads.emplace_back([&office, &totalSold]() {
			int localSold = 0;
			for (int j = 0; j < attemptsPerThread; ++j)
			{
				localSold += office.SellTickets(1);
			}
			totalSold.fetch_add(localSold, std::memory_order_relaxed);
		});
	}

	for (auto& t : threads)
	{
		t.join();
	}

	const int left = office.GetTicketsLeft();
	const int sold = totalSold.load();
	REQUIRE(sold + left == totalTickets);
	REQUIRE(sold <= totalTickets);
	REQUIRE(left >= 0);
}

TEST_CASE("Multi-threaded overdemand", "[TicketOffice]")
{
	constexpr int total = 500;
	TicketOffice office(total);
	constexpr int threads = 8;
	constexpr int reqPerThread = 100;

	std::vector<std::thread> thr;
	std::atomic totalSold{ 0 };

	for (int i = 0; i < threads; ++i)
	{
		thr.emplace_back([&office, &totalSold]() {
			const auto s = office.SellTickets(reqPerThread);
			totalSold.fetch_add(s, std::memory_order_relaxed);
		});
	}
	for (auto& t : thr)
	{
		t.join();
	}

	const int left = office.GetTicketsLeft();
	const int sold = totalSold.load();
	REQUIRE(sold + left == total);
	REQUIRE(sold <= total);
}
