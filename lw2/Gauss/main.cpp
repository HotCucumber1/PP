#include "GaussBlur.h"

#include <chrono>
#include <iostream>

int main(const int argc, char* argv[])
{
	try
	{
		if (argc != 5)
		{
			std::cerr << "Usage: gauss INPUT_FILE OUTPUT_FILE RADIUS NUM_THREADS" << std::endl;
			return 1;
		}

		const std::string inputFile = argv[1];
		const std::string outputFile = argv[2];
		const int radius = std::stoi(argv[3]);
		const int numThreads = std::stoi(argv[4]);

		if (numThreads <= 0)
		{
			std::cerr << "Error: Wrong threads num." << std::endl;
			return 1;
		}

		const auto start = std::chrono::high_resolution_clock::now();

		GaussBlur app;

		// std::cout << "Loading image..." << std::endl;
		app.LoadImageFile(inputFile);
		// std::cout << "Gauss usage (R = " << radius << ", Threads = " << numThreads << ")..." << std::endl;
		app.ApplyBlur(radius, numThreads);
		// std::cout << "Save image..." << std::endl;
		app.SaveImageFile(outputFile);

		const auto end = std::chrono::high_resolution_clock::now();
		const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		std::cout << duration << std::endl;

		// std::cout << "Ready!" << std::endl;
		return 0;
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
}