#include "Unpacker.h"

#include <chrono>
#include <iostream>
#include <string>

struct Args
{
	int processes = 1;
	std::string outFolder;
	std::string archiveName;
};

Args ParsArgs(int argc, char* argv[]);

int main(const int argc, char* argv[])
{
	try
	{
		const auto args = ParsArgs(argc, argv);
		const auto start = std::chrono::high_resolution_clock::now();

		const auto archTime = Unpacker(args.processes)
								  .Unpack(args.archiveName, args.outFolder);

		const auto end = std::chrono::high_resolution_clock::now();
		const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		std::cout << "Command executed in: " << duration.count() << " milliseconds" << std::endl;
		std::cout << "Sequential part executed in: " << archTime << " milliseconds" << std::endl;

		return 0;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return 1;
	}
}

Args ParsArgs(const int argc, char* argv[])
{
	constexpr int minArgsCount = 4;
	Args args;

	if (argc < minArgsCount)
	{
		throw std::runtime_error("Not enough arguments");
	}

	const std::string mode = argv[1];
	if (mode == "-S")
	{
		args.processes = 1;

		args.archiveName = argv[2];
		args.outFolder = argv[3];
	}
	else if (mode == "-P")
	{
		if (argc < minArgsCount + 1)
		{
			throw std::runtime_error("Not enough arguments for parallel mode");
		}

		args.processes = std::stoi(argv[2]);
		if (args.processes < 1)
		{
			throw std::runtime_error("Number of processes must be at least 1");
		}

		args.archiveName = argv[3];
		args.outFolder = argv[4];
	}
	else
	{
		throw std::runtime_error("Invalid mode. Use -S or -P");
	}

	return args;
}