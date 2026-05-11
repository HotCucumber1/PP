#include "StopSource.h"

#include <iostream>
#include <thread>

void Worker(const StopToken& token, const int id)
{
	while (!token.StopRequested())
	{
		std::cout << "Worker " << id << " is working...\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	std::cout << "Worker " << id << " stopped.\n";
}

int main()
{
	const StopSource source;
	auto token = source.GetToken();

	std::jthread t1(Worker, token, 1);
	std::jthread t2(Worker, token, 2);

	std::this_thread::sleep_for(std::chrono::seconds(1));
	source.RequestStop();

	return 0;
}
