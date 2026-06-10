#include "AsyncFile.h"
#include "Task.h"

#include <iostream>
#include <vector>

Task AsyncCopyFile(
	Dispatcher& dispatcher,
	const std::string& from,
	const std::string& to)
{
	AsyncFile input = co_await AsyncOpenFile(dispatcher, from, OpenMode::Read);
	AsyncFile output = co_await AsyncOpenFile(dispatcher, to, OpenMode::Write);

	std::vector<char> buffer(1024);

	for (unsigned bytesRead;
		(bytesRead = co_await input.ReadAsync(dispatcher, buffer.data(), buffer.size())) != 0;)
	{
		co_await output.AsyncWrite(dispatcher, buffer.data(), bytesRead);
	}
}

Task AsyncCopyTwoFiles(Dispatcher& dispatcher)
{
	auto t1 = AsyncCopyFile(dispatcher, "a.in", "a.out");
	auto t2 = AsyncCopyFile(dispatcher, "b.in", "b.out");

	co_await t1;
	co_await t2;
}

int main()
{
	system("echo 'Some data in file A' > a.in");
	system("echo 'Another data in file B ' > b.in");

	try
	{
		Dispatcher dispatcher;

		auto mainTask = AsyncCopyTwoFiles(dispatcher);

		while (dispatcher.HasActiveOps())
		{
			dispatcher.Poll();
		}

		std::cout << "Files copied successfully!" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}
