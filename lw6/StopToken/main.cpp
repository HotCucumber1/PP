#include "StopSource.h"

#include <iostream>
#include <thread>

void worker(const StopToken& token, const int id)
{
	while (!token.StopRequested())
	{
		std::cout << "Worker " << id << " is working...\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	std::cout << "Worker " << id << " stopped.\n";
}

// TODO почитать перед сдачей

int main()
{
	const StopSource source;
	auto token = source.GetToken();

	std::jthread t1(worker, token, 1);
	std::jthread t2(worker, token, 2);

	std::this_thread::sleep_for(std::chrono::seconds(1));
	source.RequestStop();

	return 0;
}