#include "Bank.h"

#include <mutex>

void AssertAmountNotNegative(Money amount);

Bank::Bank(const Money cash)
	: m_cash(cash)
{
	if (m_cash < 0)
	{
		throw BankOperationError("Negative amount of cash");
	}
}

unsigned long long Bank::GetOperationsCount() const
{
	return m_operationsCount.load(std::memory_order_acquire);
}

void Bank::SendMoney(
	const AccountId srcAccountId,
	const AccountId dstAccountId,
	const Money amount)
{
	AssertAmountNotNegative(amount);

	std::unique_lock lock(m_mutex);

	AssertAccountExists(srcAccountId);
	AssertAccountExists(dstAccountId);

	const auto srcAccountMoney = m_bankAccounts.at(srcAccountId);
	if (amount > srcAccountMoney)
	{
		throw std::out_of_range("Insufficient funds");
	}

	m_bankAccounts[srcAccountId] -= amount;
	m_bankAccounts[dstAccountId] += amount;
	m_operationsCount.fetch_add(1, std::memory_order_release);
}

bool Bank::TrySendMoney(
	const AccountId srcAccountId,
	const AccountId dstAccountId,
	const Money amount)
{
	AssertAmountNotNegative(amount);

	std::unique_lock lock(m_mutex);

	AssertAccountExists(srcAccountId);
	AssertAccountExists(dstAccountId);

	const auto srcAccountMoney = m_bankAccounts.at(srcAccountId);
	if (amount > srcAccountMoney)
	{
		return false;
	}

	m_bankAccounts[srcAccountId] -= amount;
	m_bankAccounts[dstAccountId] += amount;
	m_operationsCount.fetch_add(1, std::memory_order_release);
	return true;
}

Money Bank::GetCash() const
{
	std::shared_lock lock(m_mutex);

	m_operationsCount.fetch_add(1, std::memory_order_release);
	return m_cash;
}

Money Bank::GetAccountBalance(const AccountId accountId) const
{
	std::shared_lock lock(m_mutex);

	AssertAccountExists(accountId);
	m_operationsCount.fetch_add(1, std::memory_order_release);
	return m_bankAccounts.at(accountId);
}

void Bank::WithdrawMoney(const AccountId account, const Money amount)
{
	AssertAmountNotNegative(amount);

	std::unique_lock lock(m_mutex);

	AssertAccountExists(account);
	const auto accountMoney = m_bankAccounts.at(account);

	if (amount > accountMoney)
	{
		throw BankOperationError("No money(");
	}
	m_bankAccounts[account] -= amount;
	m_cash += amount;
	m_operationsCount.fetch_add(1, std::memory_order_release);
}

bool Bank::TryWithdrawMoney(const AccountId account, const Money amount)
{
	AssertAmountNotNegative(amount);

	std::unique_lock lock(m_mutex);

	AssertAccountExists(account);
	const auto accountMoney = m_bankAccounts.at(account);
	if (amount > accountMoney)
	{
		return false;
	}
	m_bankAccounts[account] -= amount;
	m_cash += amount;
	m_operationsCount.fetch_add(1, std::memory_order_release);
	return true;
}

void Bank::DepositMoney(const AccountId account, const Money amount)
{
	AssertAmountNotNegative(amount);

	std::unique_lock lock(m_mutex);

	AssertAccountExists(account);
	if (amount > m_cash)
	{
		throw BankOperationError("There is no so much cash");
	}
	m_cash -= amount;
	m_bankAccounts[account] += amount;
	m_operationsCount.fetch_add(1, std::memory_order_release);
}

AccountId Bank::OpenAccount()
{
	constexpr int defaultAmount = 0;

	std::unique_lock lock(m_mutex);

	const auto newAccount = m_nextAccount++;
	m_bankAccounts[newAccount] = defaultAmount;
	m_operationsCount.fetch_add(1, std::memory_order_release);
	return newAccount;
}

Money Bank::CloseAccount(const AccountId accountId)
{
	std::unique_lock lock(m_mutex);

	AssertAccountExists(accountId);

	const auto money = m_bankAccounts.at(accountId);
	m_bankAccounts.erase(accountId);
	m_cash += money;
	m_operationsCount.fetch_add(1, std::memory_order_release);
	return money;
}

void Bank::AssertAccountExists(const AccountId account) const
{
	const auto accountIt = m_bankAccounts.find(account);
	if (accountIt == m_bankAccounts.end())
	{
		throw BankOperationError("Account " + std::to_string(account) + " not found");
	}
}

void AssertAmountNotNegative(const Money amount)
{
	if (amount < 0)
	{
		throw std::out_of_range("Negative amount of money");
	}
}
