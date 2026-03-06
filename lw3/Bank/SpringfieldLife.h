#pragma once
#include "Bank.h"

#include <random>

enum class AppMode
{
	SingleMode,
	MultiThread,
};

class SpringfieldLife
{
public:
	explicit SpringfieldLife(
		Money startCash,
		bool needLog = true);

	void Run(AppMode mode, double simulationTime);

private:
	void RunSingleMode(double simulationTime);

	void RunMultiThreadMode(double simulationTime);

	void HomerRoutine(const std::atomic<bool>& timeIsUp);
	void MargeRoutine(const std::atomic<bool>& timeIsUp);
	void BartAndLisaRoutine(const std::atomic<bool>& timeIsUp);
	void ApuRoutine(const std::atomic<bool>& timeIsUp);
	void MrBurnsRoutine(const std::atomic<bool>& timeIsUp);

	void HomerToMarge();
	void HomerForElectricity();
	void HomerWithdrawToChildren();

	void MargeBoughtFromApu();

	void ChildrenToApu();

	void ApuForElectricity();

	void MrBurnsToHomer();

private:
	static constexpr Money homerToMarge = 20;
	static constexpr Money homerTaxes = 50;
	static constexpr Money homerToChildren = 20;
	static constexpr Money margeToApu = 5;
	static constexpr Money apuTaxes = 100;

	Money m_startCash;
	Bank m_bank;

	bool m_needLog;
	AccountId m_homer;
	AccountId m_marge;
	AccountId m_apu;
	AccountId m_mrBurns;

	std::random_device m_rd;
	std::mt19937 m_gen;
};
