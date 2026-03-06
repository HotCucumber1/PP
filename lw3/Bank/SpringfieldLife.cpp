#include "SpringfieldLife.h"

#include <iostream>
#include <random>
#include <thread>
#include <vector>

SpringfieldLife::SpringfieldLife(
	const Money startCash,
	const bool needLog)
	: m_startCash(startCash)
	, m_bank(startCash)
	, m_needLog(needLog)
	, m_gen(m_rd())
{
	m_homer = m_bank.OpenAccount();
	m_marge = m_bank.OpenAccount();
	m_apu = m_bank.OpenAccount();
	m_mrBurns = m_bank.OpenAccount();
}

void SpringfieldLife::Run(const AppMode mode, const double simulationTime)
{
	switch (mode)
	{
	case AppMode::SingleMode:
		RunSingleMode(simulationTime);
		break;
	case AppMode::MultiThread:
		RunMultiThreadMode(simulationTime);
		break;
	default:
		break;
	}
	std::cout << "-------------------------------" << std::endl;
	std::cout << "Operations completed: " + std::to_string(m_bank.GetOperationsCount()) << std::endl;

	const auto totalMoney = m_bank.GetAccountBalance(m_homer)
		+ m_bank.GetAccountBalance(m_marge)
		+ m_bank.GetAccountBalance(m_apu)
		+ m_bank.GetAccountBalance(m_mrBurns)
		+ m_bank.GetCash();
	if (totalMoney == m_startCash)
	{
		std::cout << "System is fine!" << std::endl;
	}
	else
	{
		std::cout << "Lost " + std::to_string(totalMoney) << std::endl;
	}
}

void SpringfieldLife::RunSingleMode(const double simulationTime)
{
	const auto start = std::chrono::steady_clock::now();
	while (std::chrono::steady_clock::now() - start < std::chrono::duration<double>(simulationTime))
	{
		// homer
		HomerToMarge();
		HomerForElectricity();
		HomerWithdrawToChildren();

		// marge
		MargeBoughtFromApu();
		// bart and lisa
		ChildrenToApu();
		// apu
		ApuForElectricity();
		// mrBurns
		MrBurnsToHomer();
	}
}

void SpringfieldLife::RunMultiThreadMode(const double simulationTime)
{
	std::atomic timeIsUp{ false };

	std::jthread timerThread([&]() {
		std::this_thread::sleep_for(std::chrono::duration<double>(simulationTime));
		timeIsUp = true;
		if (m_needLog)
		{
			std::cout << "Simulation time is over!" << std::endl;
		}
	});

	std::vector<std::jthread> citizenThreads;

	citizenThreads.emplace_back(&SpringfieldLife::HomerRoutine, this, std::ref(timeIsUp));
	citizenThreads.emplace_back(&SpringfieldLife::MargeRoutine, this, std::ref(timeIsUp));
	citizenThreads.emplace_back(&SpringfieldLife::BartAndLisaRoutine, this, std::ref(timeIsUp));
	citizenThreads.emplace_back(&SpringfieldLife::ApuRoutine, this, std::ref(timeIsUp));
	citizenThreads.emplace_back(&SpringfieldLife::MrBurnsRoutine, this, std::ref(timeIsUp));
}

void SpringfieldLife::HomerRoutine(const std::atomic<bool>& timeIsUp)
{
	std::uniform_int_distribution sleepDist(10, 50);
	while (!timeIsUp)
	{
		HomerToMarge();
		HomerForElectricity();
		HomerWithdrawToChildren();
		// std::this_thread::sleep_for(std::chrono::milliseconds(sleepDist(m_gen)));
	}
}

void SpringfieldLife::MargeRoutine(const std::atomic<bool>& timeIsUp)
{
	std::uniform_int_distribution sleepDist(15, 45);
	while (!timeIsUp)
	{
		MargeBoughtFromApu();
		// std::this_thread::sleep_for(std::chrono::milliseconds(sleepDist(m_gen)));
	}
}

void SpringfieldLife::BartAndLisaRoutine(const std::atomic<bool>& timeIsUp)
{
	std::uniform_int_distribution sleepDist(20, 60);

	while (!timeIsUp)
	{
		ChildrenToApu();
		// std::this_thread::sleep_for(std::chrono::milliseconds(sleepDist(m_gen)));
	}
}

void SpringfieldLife::ApuRoutine(const std::atomic<bool>& timeIsUp)
{
	std::uniform_int_distribution sleepDist(25, 55);
	while (!timeIsUp)
	{
		ApuForElectricity();
		// std::this_thread::sleep_for(std::chrono::milliseconds(sleepDist(m_gen)));
	}
}

void SpringfieldLife::MrBurnsRoutine(const std::atomic<bool>& timeIsUp)
{
	std::uniform_int_distribution sleepDist(30, 70);

	while (!timeIsUp)
	{
		MrBurnsToHomer();
		// std::this_thread::sleep_for(std::chrono::milliseconds(sleepDist(m_gen)));
	}
}

void SpringfieldLife::HomerToMarge()
{
	if (m_bank.TrySendMoney(m_homer, m_marge, homerToMarge) && m_needLog)
	{
		std::cout << "Homer sent to Marge " + std::to_string(homerToMarge) << std::endl;
	}
}

void SpringfieldLife::HomerForElectricity()
{
	if (m_bank.TrySendMoney(m_homer, m_mrBurns, homerTaxes) && m_needLog)
	{
		std::cout << "Homer payed for electricity " + std::to_string(homerTaxes) << std::endl;
	}
}

void SpringfieldLife::HomerWithdrawToChildren()
{
	if (m_bank.TryWithdrawMoney(m_homer, homerToChildren) && m_needLog)
	{
		std::cout << "Homer withdraw to Bart and Lisa " + std::to_string(homerToChildren) << std::endl;
	}
}

void SpringfieldLife::MargeBoughtFromApu()
{
	if (m_bank.TrySendMoney(m_marge, m_apu, margeToApu) && m_needLog)
	{
		std::cout << "Marge bought from Apu for " + std::to_string(margeToApu) << std::endl;
	}
}

void SpringfieldLife::ChildrenToApu()
{
	try
	{
		m_bank.DepositMoney(m_apu, homerToChildren);
		if (m_needLog)
		{
			std::cout << "Bart and Lisa sent to Apu " + std::to_string(homerToChildren) << std::endl;
		}
	}
	catch (...)
	{
	}
}

void SpringfieldLife::ApuForElectricity()
{
	if (m_bank.TrySendMoney(m_apu, m_mrBurns, apuTaxes) && m_needLog)
	{
		std::cout << "Apu payed for electricity " + std::to_string(apuTaxes) << std::endl;
	}
}

void SpringfieldLife::MrBurnsToHomer()
{
	auto mrBurnsHas = m_bank.GetAccountBalance(m_mrBurns);
	if (mrBurnsHas > 0 && m_bank.TrySendMoney(m_mrBurns, m_homer, mrBurnsHas) && m_needLog)
	{
		std::cout << "Mr. Burns give Homer " + std::to_string(mrBurnsHas) << std::endl;
	}
}