#include "Bank.h"
#include "SpringfieldLife.h"

#include <iostream>

constexpr Money START_CASH = 1000;
constexpr double SIMULATION_DURATION = 10;

int main()
{
	try
	{
		SpringfieldLife life(START_CASH, false);
		life.Run(AppMode::SingleMode, SIMULATION_DURATION);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
}
