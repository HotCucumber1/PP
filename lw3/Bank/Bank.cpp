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
	if (srcAccountId == dstAccountId)
	{
		m_operationsCount.fetch_add(1, std::memory_order_release);
		return;
	}

	Account* srcAccount = nullptr;
	Account* dstAccount = nullptr;

	{
		std::shared_lock lock(m_accountsMutex);
		const auto srcIt = m_bankAccounts.find(srcAccountId);
		const auto dstIt = m_bankAccounts.find(dstAccountId);

		if (srcIt == m_bankAccounts.end() || dstIt == m_bankAccounts.end())
		{
			throw BankOperationError("Account not found");
		}

		srcAccount = srcIt->second.get();
		dstAccount = dstIt->second.get();
	}

	// TODO scoped_lock
	std::scoped_lock lock(srcAccount->mutex, dstAccount->mutex);

	if (amount > srcAccount->balance)
	{
		throw std::out_of_range("Insufficient funds");
	}

	srcAccount->balance -= amount;
	dstAccount->balance += amount;

	m_operationsCount.fetch_add(1, std::memory_order_release);
}

bool Bank::TrySendMoney(
	const AccountId srcAccountId,
	const AccountId dstAccountId,
	const Money amount)
{
	AssertAmountNotNegative(amount);
	if (srcAccountId == dstAccountId)
	{
		m_operationsCount.fetch_add(1, std::memory_order_release);
		return true;
	}

	Account* srcAccount = nullptr;
	Account* dstAccount = nullptr;

	{
		std::shared_lock lock(m_accountsMutex);
		const auto srcIt = m_bankAccounts.find(srcAccountId);
		const auto dstIt = m_bankAccounts.find(dstAccountId);

		if (srcIt == m_bankAccounts.end() || dstIt == m_bankAccounts.end())
		{
			throw BankOperationError("Account not found");
		}

		srcAccount = srcIt->second.get();
		dstAccount = dstIt->second.get();
	}

	std::scoped_lock lock(srcAccount->mutex, dstAccount->mutex);

	if (amount > srcAccount->balance)
	{
		return false;
	}

	srcAccount->balance -= amount;
	dstAccount->balance += amount;

	m_operationsCount.fetch_add(1, std::memory_order_release);
	return true;
}

Money Bank::GetCash() const
{
	std::shared_lock lock(m_cashMutex);

	m_operationsCount.fetch_add(1, std::memory_order_release);
	return m_cash;
}

Money Bank::GetAccountBalance(const AccountId accountId) const
{
	Account* account = nullptr;

	{
		std::shared_lock lock(m_accountsMutex);
		const auto it = m_bankAccounts.find(accountId);
		if (it == m_bankAccounts.end())
		{
			throw BankOperationError("Account " + std::to_string(accountId) + " not found");
		}
		account = it->second.get();
	}
	std::shared_lock accountLock(account->mutex);
	m_operationsCount.fetch_add(1, std::memory_order_release);
	return account->balance;
}

void Bank::WithdrawMoney(const AccountId accountId, const Money amount)
{
	AssertAmountNotNegative(amount);

	Account* account = nullptr;

	{
		std::shared_lock lock(m_accountsMutex);
		const auto it = m_bankAccounts.find(accountId);
		if (it == m_bankAccounts.end())
		{
			throw BankOperationError("Account " + std::to_string(accountId) + " not found");
		}
		account = it->second.get();
	}

	std::unique_lock cashLock(m_cashMutex);
	std::unique_lock accountLock(account->mutex);
	if (amount > account->balance)
	{
		throw BankOperationError("No money(");
	}

	account->balance -= amount;
	m_cash += amount;
	m_operationsCount.fetch_add(1, std::memory_order_release);
}

bool Bank::TryWithdrawMoney(const AccountId accountId, const Money amount)
{
	AssertAmountNotNegative(amount);

	Account* account = nullptr;

	{
		std::shared_lock lock(m_accountsMutex);
		const auto it = m_bankAccounts.find(accountId);
		if (it == m_bankAccounts.end())
		{
			throw BankOperationError("Account " + std::to_string(accountId) + " not found");
		}
		account = it->second.get();
	}
	std::unique_lock cashLock(m_cashMutex);
	std::unique_lock accountLock(account->mutex);

	if (amount > account->balance)
	{
		return false;
	}

	account->balance -= amount;
	m_cash += amount;
	m_operationsCount.fetch_add(1, std::memory_order_release);
	return true;
}

void Bank::DepositMoney(const AccountId accountId, const Money amount)
{
	AssertAmountNotNegative(amount);

	Account* account = nullptr;
	{
		std::shared_lock lock(m_accountsMutex);
		const auto it = m_bankAccounts.find(accountId);
		if (it == m_bankAccounts.end())
		{
			throw BankOperationError("Account " + std::to_string(accountId) + " not found");
		}
		account = it->second.get();
	}
	std::unique_lock cashLock(m_cashMutex);
	if (amount > m_cash)
	{
		throw BankOperationError("There is no so much cash");
	}
	std::unique_lock accountLock(account->mutex);

	m_cash -= amount;
	account->balance += amount;
	m_operationsCount.fetch_add(1, std::memory_order_release);
}

AccountId Bank::OpenAccount()
{
	AccountId newAccountId;
	{
		std::unique_lock lock(m_nextAccountMutex);
		newAccountId = m_nextAccount++;
	}

	{
		constexpr Money defaultAmount = 0;
		auto newAccount = std::make_unique<Account>(defaultAmount);
		std::unique_lock lock(m_accountsMutex);
		m_bankAccounts[newAccountId] = std::move(newAccount);
	}

	m_operationsCount.fetch_add(1, std::memory_order_release);
	return newAccountId;
}

Money Bank::CloseAccount(const AccountId accountId)
{
	std::unique_ptr<Account> accountPtr;

	{
		std::unique_lock lock(m_accountsMutex);
		const auto it = m_bankAccounts.find(accountId);
		if (it == m_bankAccounts.end())
		{
			throw BankOperationError("Account " + std::to_string(accountId) + " not found");
		}
		accountPtr = std::move(it->second);
		m_bankAccounts.erase(it);
	}

	const Money balance = accountPtr->balance;
	{
		std::unique_lock cashLock(m_cashMutex);
		m_cash += balance;
	}

	m_operationsCount.fetch_add(1, std::memory_order_release);
	return balance;
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
