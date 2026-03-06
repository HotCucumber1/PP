#include "../Bank.h"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <thread>
#include <vector>

struct BankWithAccounts
{
	Bank bank;
	AccountId account1;
	AccountId account2;
	AccountId account3;

	explicit BankWithAccounts(const Money cash = 1000)
		: bank(cash)
		, account1(bank.OpenAccount())
		, account2(bank.OpenAccount())
		, account3(bank.OpenAccount())
	{
	}

	void DepositToAll(const Money amount)
	{
		bank.DepositMoney(account1, amount);
		bank.DepositMoney(account2, amount);
		bank.DepositMoney(account3, amount);
	}
};

TEST_CASE("Bank constructor validates cash amount", "[Bank][constructor]")
{
	SECTION("Positive cash creates bank successfully")
	{
		REQUIRE_NOTHROW(Bank(1000));
	}
	SECTION("Zero cash creates bank successfully")
	{
		REQUIRE_NOTHROW(Bank(0));
	}
	SECTION("Negative cash throws BankOperationError")
	{
		REQUIRE_THROWS_AS(Bank(-100), BankOperationError);
	}
}

TEST_CASE("Bank::OpenAccount creates accounts with unique IDs", "[Bank][OpenAccount]")
{
	Bank bank(1000);

	AccountId id1 = bank.OpenAccount();
	AccountId id2 = bank.OpenAccount();
	AccountId id3 = bank.OpenAccount();

	SECTION("Account IDs are unique")
	{
		REQUIRE(id1 != id2);
		REQUIRE(id1 != id3);
		REQUIRE(id2 != id3);
	}

	SECTION("New accounts start with zero balance")
	{
		REQUIRE(bank.GetAccountBalance(id1) == 0);
		REQUIRE(bank.GetAccountBalance(id2) == 0);
		REQUIRE(bank.GetAccountBalance(id3) == 0);
	}

	SECTION("OpenAccount increments operation count")
	{
		auto initialOps = bank.GetOperationsCount();
		bank.OpenAccount();
		REQUIRE(bank.GetOperationsCount() == initialOps + 1);
	}
}

TEST_CASE("Bank::GetAccountBalance validation", "[Bank][GetAccountBalance]")
{
	Bank bank(1000);
	const AccountId validAccount = bank.OpenAccount();

	SECTION("Returns balance for existing account")
	{
		bank.DepositMoney(validAccount, 500);
		REQUIRE(bank.GetAccountBalance(validAccount) == 500);
	}
	SECTION("Throws for non-existent account")
	{
		AccountId invalidId = 99999;
		REQUIRE_THROWS_AS(bank.GetAccountBalance(invalidId), BankOperationError);
	}
	SECTION("Increments operation count")
	{
		const auto initialOps = bank.GetOperationsCount();
		bank.GetAccountBalance(validAccount);
		REQUIRE(bank.GetOperationsCount() == initialOps + 1);
	}
}

TEST_CASE("Bank::GetCash returns current cash amount", "[Bank][GetCash]")
{
	SECTION("Initial cash is correct")
	{
		Bank bank(500);
		REQUIRE(bank.GetCash() == 500);
	}
	SECTION("GetCash increments operation count")
	{
		Bank bank(500);
		auto initialOps = bank.GetOperationsCount();
		bank.GetCash();
		REQUIRE(bank.GetOperationsCount() == initialOps + 1);
	}
}


TEST_CASE("Bank::DepositMoney operations", "[Bank][DepositMoney]")
{
	BankWithAccounts test(1000);

	SECTION("Valid deposit increases account balance and decreases cash")
	{
		auto initialBalance = test.bank.GetAccountBalance(test.account1);
		auto initialCash = test.bank.GetCash();

		test.bank.DepositMoney(test.account1, 300);

		REQUIRE(test.bank.GetAccountBalance(test.account1) == initialBalance + 300);
		REQUIRE(test.bank.GetCash() == initialCash - 300);
	}
	SECTION("Deposit increments operation count")
	{
		auto initialOps = test.bank.GetOperationsCount();
		test.bank.DepositMoney(test.account1, 100);
		REQUIRE(test.bank.GetOperationsCount() == initialOps + 1);
	}
	SECTION("Cannot deposit more than available cash")
	{
		REQUIRE_THROWS_AS(test.bank.DepositMoney(test.account1, 2000), BankOperationError);
	}
	SECTION("Cannot deposit negative amount")
	{
		REQUIRE_THROWS_AS(test.bank.DepositMoney(test.account1, -50), std::out_of_range);
	}
	SECTION("Cannot deposit to non-existent account")
	{
		REQUIRE_THROWS_AS(test.bank.DepositMoney(99999, 100), BankOperationError);
	}
}

TEST_CASE("Bank::WithdrawMoney operations", "[Bank][WithdrawMoney]")
{
	BankWithAccounts test(1000);
	test.bank.DepositMoney(test.account1, 500);

	SECTION("Valid withdrawal decreases account balance and increases cash")
	{
		auto initialBalance = test.bank.GetAccountBalance(test.account1);
		auto initialCash = test.bank.GetCash();

		test.bank.WithdrawMoney(test.account1, 200);

		REQUIRE(test.bank.GetAccountBalance(test.account1) == initialBalance - 200);
		REQUIRE(test.bank.GetCash() == initialCash + 200);
	}
	SECTION("Withdraw increments operation count")
	{
		auto initialOps = test.bank.GetOperationsCount();
		test.bank.WithdrawMoney(test.account1, 100);
		REQUIRE(test.bank.GetOperationsCount() == initialOps + 1);
	}
	SECTION("Cannot withdraw more than account balance")
	{
		REQUIRE_THROWS_AS(test.bank.WithdrawMoney(test.account1, 1000), BankOperationError);
	}
	SECTION("Cannot withdraw negative amount")
	{
		REQUIRE_THROWS_AS(test.bank.WithdrawMoney(test.account1, -50), std::out_of_range);
	}
	SECTION("Cannot withdraw from non-existent account")
	{
		REQUIRE_THROWS_AS(test.bank.WithdrawMoney(99999, 100), BankOperationError);
	}
}

TEST_CASE("Bank::TryWithdrawMoney operations", "[Bank][TryWithdrawMoney]")
{
	BankWithAccounts test(1000);
	test.bank.DepositMoney(test.account1, 500);

	SECTION("Valid withdrawal returns true and updates balances")
	{
		auto initialBalance = test.bank.GetAccountBalance(test.account1);
		auto initialCash = test.bank.GetCash();

		bool result = test.bank.TryWithdrawMoney(test.account1, 200);

		REQUIRE(result == true);
		REQUIRE(test.bank.GetAccountBalance(test.account1) == initialBalance - 200);
		REQUIRE(test.bank.GetCash() == initialCash + 200);
	}
	SECTION("Withdrawal exceeding balance returns false and doesn't change anything")
	{
		auto initialBalance = test.bank.GetAccountBalance(test.account1);
		auto initialCash = test.bank.GetCash();

		bool result = test.bank.TryWithdrawMoney(test.account1, 1000);

		REQUIRE(result == false);
		REQUIRE(test.bank.GetAccountBalance(test.account1) == initialBalance);
		REQUIRE(test.bank.GetCash() == initialCash);
	}
	SECTION("TryWithdraw increments operation count on success")
	{
		auto initialOps = test.bank.GetOperationsCount();
		test.bank.TryWithdrawMoney(test.account1, 100);
		REQUIRE(test.bank.GetOperationsCount() == initialOps + 1);
	}
	SECTION("TryWithdraw does NOT increment operation count on failure")
	{
		auto initialOps = test.bank.GetOperationsCount();
		test.bank.TryWithdrawMoney(test.account1, 1000);
		REQUIRE(test.bank.GetOperationsCount() == initialOps);
	}
	SECTION("Cannot try withdraw negative amount")
	{
		REQUIRE_THROWS_AS(test.bank.TryWithdrawMoney(test.account1, -50), std::out_of_range);
	}
	SECTION("Cannot try withdraw from non-existent account")
	{
		REQUIRE_THROWS_AS(test.bank.TryWithdrawMoney(99999, 100), BankOperationError);
	}
}

TEST_CASE("Bank::SendMoney operations", "[Bank][SendMoney]")
{
	BankWithAccounts test(1000);
	test.bank.DepositMoney(test.account1, 500);
	test.bank.DepositMoney(test.account2, 100);

	SECTION("Valid transfer updates both accounts")
	{
		auto srcInitial = test.bank.GetAccountBalance(test.account1);
		auto dstInitial = test.bank.GetAccountBalance(test.account2);

		test.bank.SendMoney(test.account1, test.account2, 200);

		REQUIRE(test.bank.GetAccountBalance(test.account1) == srcInitial - 200);
		REQUIRE(test.bank.GetAccountBalance(test.account2) == dstInitial + 200);
	}
	SECTION("SendMoney increments operation count")
	{
		auto initialOps = test.bank.GetOperationsCount();
		test.bank.SendMoney(test.account1, test.account2, 100);
		REQUIRE(test.bank.GetOperationsCount() == initialOps + 1);
	}
	SECTION("Cannot send more than source balance")
	{
		REQUIRE_THROWS_AS(test.bank.SendMoney(test.account1, test.account2, 1000), std::out_of_range);
	}
	SECTION("Cannot send negative amount")
	{
		REQUIRE_THROWS_AS(test.bank.SendMoney(test.account1, test.account2, -50), std::out_of_range);
	}
	SECTION("Cannot send from non-existent account")
	{
		REQUIRE_THROWS_AS(test.bank.SendMoney(99999, test.account2, 100), BankOperationError);
	}
	SECTION("Cannot send to non-existent account")
	{
		REQUIRE_THROWS_AS(test.bank.SendMoney(test.account1, 99999, 100), BankOperationError);
	}
	SECTION("Can send entire balance (account becomes zero)")
	{
		auto srcInitial = test.bank.GetAccountBalance(test.account1);
		test.bank.SendMoney(test.account1, test.account2, srcInitial);

		REQUIRE(test.bank.GetAccountBalance(test.account1) == 0);
		REQUIRE(test.bank.GetAccountBalance(test.account2) == 100 + srcInitial);
	}
}

TEST_CASE("Bank::TrySendMoney operations", "[Bank][TrySendMoney]")
{
	BankWithAccounts test(1000);
	test.bank.DepositMoney(test.account1, 500);
	test.bank.DepositMoney(test.account2, 100);

	SECTION("Valid transfer returns true and updates accounts")
	{
		auto srcInitial = test.bank.GetAccountBalance(test.account1);
		auto dstInitial = test.bank.GetAccountBalance(test.account2);

		bool result = test.bank.TrySendMoney(test.account1, test.account2, 200);

		REQUIRE(result == true);
		REQUIRE(test.bank.GetAccountBalance(test.account1) == srcInitial - 200);
		REQUIRE(test.bank.GetAccountBalance(test.account2) == dstInitial + 200);
	}
	SECTION("Transfer exceeding balance returns false and doesn't change anything")
	{
		auto srcInitial = test.bank.GetAccountBalance(test.account1);
		auto dstInitial = test.bank.GetAccountBalance(test.account2);

		bool result = test.bank.TrySendMoney(test.account1, test.account2, 1000);

		REQUIRE(result == false);
		REQUIRE(test.bank.GetAccountBalance(test.account1) == srcInitial);
		REQUIRE(test.bank.GetAccountBalance(test.account2) == dstInitial);
	}
	SECTION("TrySend increments operation count on success")
	{
		auto initialOps = test.bank.GetOperationsCount();
		test.bank.TrySendMoney(test.account1, test.account2, 100);
		REQUIRE(test.bank.GetOperationsCount() == initialOps + 1);
	}
	SECTION("TrySend does NOT increment operation count on failure")
	{
		auto initialOps = test.bank.GetOperationsCount();
		test.bank.TrySendMoney(test.account1, test.account2, 1000);
		REQUIRE(test.bank.GetOperationsCount() == initialOps);
	}
	SECTION("Cannot try send negative amount")
	{
		REQUIRE_THROWS_AS(test.bank.TrySendMoney(test.account1, test.account2, -50), std::out_of_range);
	}
	SECTION("Cannot try send from non-existent account")
	{
		REQUIRE_THROWS_AS(test.bank.TrySendMoney(99999, test.account2, 100), BankOperationError);
	}
	SECTION("Cannot try send to non-existent account")
	{
		REQUIRE_THROWS_AS(test.bank.TrySendMoney(test.account1, 99999, 100), BankOperationError);
	}
}

TEST_CASE("Bank::CloseAccount operations", "[Bank][CloseAccount]")
{
	BankWithAccounts test(1000);
	test.bank.DepositMoney(test.account1, 500);

	SECTION("Closing account returns balance and adds it to cash")
	{
		auto initialCash = test.bank.GetCash();
		auto balance = test.bank.GetAccountBalance(test.account1);

		Money returned = test.bank.CloseAccount(test.account1);

		REQUIRE(returned == balance);
		REQUIRE(test.bank.GetCash() == initialCash + balance);
	}
	SECTION("Account is removed after closing")
	{
		test.bank.CloseAccount(test.account1);
		REQUIRE_THROWS_AS(test.bank.GetAccountBalance(test.account1), BankOperationError);
	}
	SECTION("CloseAccount increments operation count")
	{
		auto initialOps = test.bank.GetOperationsCount();
		test.bank.CloseAccount(test.account1);
		REQUIRE(test.bank.GetOperationsCount() == initialOps + 1);
	}
	SECTION("Cannot close non-existent account")
	{
		REQUIRE_THROWS_AS(test.bank.CloseAccount(99999), BankOperationError);
	}
	SECTION("Closing zero-balance account returns zero and doesn't affect cash")
	{
		BankWithAccounts test2(1000);
		AccountId emptyAccount = test2.bank.OpenAccount();
		auto initialCash = test2.bank.GetCash();

		Money returned = test2.bank.CloseAccount(emptyAccount);

		REQUIRE(returned == 0);
		REQUIRE(test2.bank.GetCash() == initialCash);
	}
}

TEST_CASE("Bank::GetOperationsCount tracks all operations", "[Bank][GetOperationsCount]")
{
	Bank bank(1000);
	AccountId acc1 = bank.OpenAccount();
	AccountId acc2 = bank.OpenAccount();

	SECTION("OpenAccount increments counter")
	{
		auto before = bank.GetOperationsCount();
		bank.OpenAccount();
		REQUIRE(bank.GetOperationsCount() == before + 1);
	}
	SECTION("DepositMoney increments counter")
	{
		bank.DepositMoney(acc1, 100);
		auto afterDeposit = bank.GetOperationsCount();

		bank.DepositMoney(acc2, 200);
		REQUIRE(bank.GetOperationsCount() == afterDeposit + 1);
	}
	SECTION("WithdrawMoney increments counter")
	{
		bank.DepositMoney(acc1, 500);
		auto before = bank.GetOperationsCount();

		bank.WithdrawMoney(acc1, 100);
		REQUIRE(bank.GetOperationsCount() == before + 1);
	}
	SECTION("SendMoney increments counter")
	{
		bank.DepositMoney(acc1, 500);
		auto before = bank.GetOperationsCount();

		bank.SendMoney(acc1, acc2, 100);
		REQUIRE(bank.GetOperationsCount() == before + 1);
	}
	SECTION("CloseAccount increments counter")
	{
		bank.DepositMoney(acc1, 100);
		auto before = bank.GetOperationsCount();

		bank.CloseAccount(acc1);
		REQUIRE(bank.GetOperationsCount() == before + 1);
	}
	SECTION("Multiple operations increment counter multiple times")
	{
		auto before = bank.GetOperationsCount();

		bank.DepositMoney(acc1, 100);
		bank.DepositMoney(acc2, 200);
		bank.SendMoney(acc1, acc2, 50);
		bank.WithdrawMoney(acc2, 100);

		REQUIRE(bank.GetOperationsCount() == before + 4);
	}
}

TEST_CASE("Complex money flow scenarios", "[Bank][integration]")
{
	Bank bank(1000);
	AccountId homer = bank.OpenAccount();
	AccountId marge = bank.OpenAccount();
	AccountId apu = bank.OpenAccount();
	AccountId burns = bank.OpenAccount();

	bank.DepositMoney(homer, 500);
	bank.DepositMoney(marge, 300);
	bank.DepositMoney(apu, 200);

	SECTION("Money should be conserved across multiple transfers")
	{
		bank.SendMoney(homer, marge, 100);
		bank.SendMoney(marge, apu, 50);
		bank.SendMoney(apu, burns, 30);
		bank.SendMoney(burns, homer, 20);

		Money totalInAccounts = bank.GetAccountBalance(homer) + bank.GetAccountBalance(marge) + bank.GetAccountBalance(apu) + bank.GetAccountBalance(burns);
		REQUIRE(totalInAccounts + bank.GetCash() == 1000);
	}
	SECTION("Try operations should not break money conservation on failure")
	{
		auto cashBefore = bank.GetCash();
		auto balancesBefore = std::array{
			bank.GetAccountBalance(homer),
			bank.GetAccountBalance(marge),
			bank.GetAccountBalance(apu),
			bank.GetAccountBalance(burns)
		};

		bool result = bank.TrySendMoney(homer, marge, 1000);
		REQUIRE(result == false);

		REQUIRE(bank.GetCash() == cashBefore);
		REQUIRE(bank.GetAccountBalance(homer) == balancesBefore[0]);
		REQUIRE(bank.GetAccountBalance(marge) == balancesBefore[1]);
		REQUIRE(bank.GetAccountBalance(apu) == balancesBefore[2]);
		REQUIRE(bank.GetAccountBalance(burns) == balancesBefore[3]);
	}
}

TEST_CASE("Bank is thread-safe", "[Bank][multithreading]")
{
	constexpr int NUM_THREADS = 10;
	constexpr int OPS_PER_THREAD = 100;
	constexpr int INITIAL_CASH = 10000;

	Bank bank(INITIAL_CASH);
	std::vector<AccountId> accounts;

	for (int i = 0; i < NUM_THREADS; ++i)
	{
		accounts.push_back(bank.OpenAccount());
		bank.DepositMoney(accounts.back(), 1000);
	}

	SECTION("Concurrent deposits and withdrawals should maintain money conservation")
	{
		std::vector<std::thread> threads;

		for (int i = 0; i < NUM_THREADS; ++i)
		{
			threads.emplace_back([&bank, &accounts, i, OPS_PER_THREAD]() {
				for (int j = 0; j < OPS_PER_THREAD; ++j)
				{
					int from = j % accounts.size();
					int to = (j + 1) % accounts.size();

					if (j % 3 == 0)
					{
						bank.TrySendMoney(accounts[from], accounts[to], 10);
					}
					else if (j % 3 == 1)
					{
						bank.TryWithdrawMoney(accounts[from], 5);
					}
					else
					{
						try
						{
							bank.DepositMoney(accounts[to], 5);
						}
						catch (...)
						{
						}
					}
				}
			});
		}

		for (auto& t : threads)
		{
			t.join();
		}

		Money totalInAccounts = 0;
		for (auto acc : accounts)
		{
			totalInAccounts += bank.GetAccountBalance(acc);
		}

		REQUIRE(totalInAccounts + bank.GetCash() == INITIAL_CASH);
	}
}

TEST_CASE("Bank edge cases", "[Bank][edge]")
{
	Bank bank(100);
	AccountId acc = bank.OpenAccount();

	SECTION("Maximum money values")
	{
		Money maxMoney = std::numeric_limits<Money>::max();
		REQUIRE_THROWS_AS(bank.DepositMoney(acc, maxMoney), BankOperationError);
	}
	SECTION("Zero amount operations")
	{
		bank.DepositMoney(acc, 0);
		REQUIRE(bank.GetAccountBalance(acc) == 0);

		REQUIRE_NOTHROW(bank.WithdrawMoney(acc, 0));
		REQUIRE(bank.GetAccountBalance(acc) == 0);

		REQUIRE_NOTHROW(bank.SendMoney(acc, acc, 0));
		REQUIRE(bank.GetAccountBalance(acc) == 0);
	}
	SECTION("Send to same account")
	{
		bank.DepositMoney(acc, 100);
		auto before = bank.GetAccountBalance(acc);

		bank.SendMoney(acc, acc, 50);

		REQUIRE(bank.GetAccountBalance(acc) == before);
	}
}