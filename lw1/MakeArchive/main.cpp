#include "Packer.h"

#include <chrono>
#include <iostream>

constexpr double MS_IN_S = 1000.0;

struct Args
{
	int processes = 1;
	std::string outFile;
	std::vector<std::string> inFiles;
};

Args ParsArgs(int argc, char* argv[]);

int main(const int argc, char* argv[])
{
	try
	{
		const auto args = ParsArgs(argc, argv);
		const auto start = std::chrono::high_resolution_clock::now();

		const auto archTime = Packer(args.processes)
								  .Pack(args.outFile, args.inFiles);

		const auto end = std::chrono::high_resolution_clock::now();
		const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		std::cout << "Command executed in: " << duration.count() / MS_IN_S << " seconds" << std::endl;
		std::cout << "Sequential part executed in: " << archTime / MS_IN_S << " seconds" << std::endl;

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
		args.outFile = argv[2];

		for (int i = 3; i < argc; i++)
		{
			args.inFiles.emplace_back(argv[i]);
		}
	}
	else if (mode == "-P")
	{
		if (argc < minArgsCount + 1)
		{
			throw std::runtime_error("Not enough arguments for parallel mode");
		}

		try
		{
			args.processes = std::stoi(argv[2]);
			if (args.processes < 1)
			{
				throw std::runtime_error("Number of processes must be at least 1");
			}
		}
		catch (const std::exception&)
		{
			throw std::runtime_error("Invalid number of processes");
		}

		args.outFile = argv[3];
		for (int i = minArgsCount; i < argc; i++)
		{
			args.inFiles.emplace_back(argv[i]);
		}
	}
	else
	{
		throw std::runtime_error("Invalid mode. Use -S or -P");
	}
	if (args.inFiles.empty())
	{
		throw std::runtime_error("No input files specified");
	}

	return args;
}