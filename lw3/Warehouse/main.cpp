#include "Warehouse.h"

#include <iostream>
#include <random>
#include <vector>

static std::atomic g_running{ true };
static Warehouse* g_warehouse = nullptr;

void supplierThread(Warehouse& warehouse, int id)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<size_t> amountDist(1, 10);
	std::uniform_int_distribution<int> sleepDist(100, 500);

	size_t supplied = 0;

	while (g_running)
	{
		size_t amount = amountDist(gen);

		if (warehouse.AddGoods(amount))
		{
			supplied += amount;
			std::cout << "📦 Supplier " << id << " delivered " << amount
					  << " units (total: " << supplied << ")" << std::endl;
		}
		else
		{
			std::cout << "⏳ Supplier " << id << " waiting - warehouse full" << std::endl;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(sleepDist(gen)));
	}

	std::cout << "📊 Supplier " << id << " total supplied: " << supplied << std::endl;
	warehouse.GetTotalSupplied() += supplied; // Обновляем глобальную статистику
}

void clientThread(Warehouse& warehouse, int id)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<size_t> amountDist(1, 8);
	std::uniform_int_distribution<int> sleepDist(150, 600); // миллисекунды

	size_t purchased = 0;

	while (g_running)
	{
		size_t amount = amountDist(gen);

		if (warehouse.RemoveGoods(amount))
		{
			purchased += amount;
			std::cout << "🛒 Client " << id << " bought " << amount
					  << " units (total: " << purchased << ")" << std::endl;
		}
		else
		{
			std::cout << "⏳ Client " << id << " waiting - out of stock" << std::endl;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(sleepDist(gen)));
	}

	std::cout << "📊 Client " << id << " total purchased: " << purchased << std::endl;
	warehouse.GetTotalPurchased() += purchased;
}

void auditorThread(const Warehouse& warehouse, const int id)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution sleepDist(800, 2000);

	while (g_running)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(sleepDist(gen)));

		size_t currentStock = warehouse.GetCurrentStock();
		std::cout << "🔍 Auditor " << id << " reports: " << currentStock
				  << " units in stock" << std::endl;
	}
}

int main(const int argc, char* argv[])
{
	if (argc != 4)
	{
		std::cerr << "Usage: " << argv[0]
				  << " NUM_SUPPLIERS NUM_CLIENTS NUM_AUDITORS" << std::endl;
		return 1;
	}

	const int numSuppliers = std::stoi(argv[1]);
	const int numClients = std::stoi(argv[2]);
	const int numAuditors = std::stoi(argv[3]);

	if (numSuppliers < 0 || numClients < 0 || numAuditors < 0)
	{
		std::cerr << "All arguments must be non-negative" << std::endl;
		return 1;
	}

	Warehouse warehouse(100);
	g_warehouse = &warehouse;

	std::cout << "🚀 Starting warehouse simulation with "
			  << numSuppliers << " suppliers, "
			  << numClients << " clients, "
			  << numAuditors << " auditors" << std::endl;
	std::cout << "Warehouse capacity: 100 units" << std::endl;
	std::cout << "Press Ctrl+C to stop..." << std::endl;
	std::cout << "----------------------------------------" << std::endl;

	std::vector<std::thread> threads;

	// Запуск потоков-поставщиков
	for (int i = 0; i < numSuppliers; ++i)
	{
		threads.emplace_back(supplierThread, std::ref(warehouse), i);
	}

	// Запуск потоков-покупателей
	for (int i = 0; i < numClients; ++i)
	{
		threads.emplace_back(clientThread, std::ref(warehouse), i);
	}

	// Запуск потоков-аудиторов
	for (int i = 0; i < numAuditors; ++i)
	{
		threads.emplace_back(auditorThread, std::ref(warehouse), i);
	}

	// Ожидание завершения (или сигнала)
	for (auto& thread : threads)
	{
		if (thread.joinable())
		{
			thread.join();
		}
	}

	std::cout << "\n========== FINAL REPORT ==========" << std::endl;
	std::cout << "Total supplied to warehouse: " << warehouse.GetTotalSupplied() << std::endl;
	std::cout << "Total purchased from warehouse: " << warehouse.GetTotalPurchased() << std::endl;
	std::cout << "Remaining in warehouse: " << warehouse.GetCurrentStock() << std::endl;

	size_t expected = warehouse.GetTotalSupplied() - warehouse.GetTotalPurchased();
	if (warehouse.GetCurrentStock() == expected)
	{
		std::cout << "Inventory is consistent!" << std::endl;
	}
	else
	{
		std::cout << "Inventory mismatch! Expected: " << expected << std::endl;
	}

	return 0;
}