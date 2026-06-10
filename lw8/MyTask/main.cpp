#include "MyTask.h"

#include <iostream>

MyTask SimpleCoroutine()
{
	co_return "Hello from coroutine!";
}

int main()
{
	const auto task = SimpleCoroutine();
	std::cout << task.GetResult() << std::endl;
}