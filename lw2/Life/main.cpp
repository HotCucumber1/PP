#include "LifeGame/LifeGameIO.h"

#include <complex>
#include <cstring>
#include <thread>

enum class Mode
{
	GenerateMode,
	StepMode,
	VisualizeMode,
};

struct LifeGameArgs
{
	std::string outFile;
	std::string inFile;
	int width;
	int height;
	int threads;
	double probability;
	Mode mode;
};

LifeGameArgs ParseArgs(int argc, char* argv[]);
void PrintGame(const LifeGame& game, const std::string& outFile);

void Generate(const LifeGameArgs& args);
void Step(const LifeGameArgs& args);
void Visualize(const LifeGameArgs& args);

int main(const int argc, char* argv[])
{
	try
	{
		const auto args = ParseArgs(argc, argv);

		switch (args.mode)
		{
		case Mode::GenerateMode:
			Generate(args);
			break;
		case Mode::StepMode:
			Step(args);
			break;
		case Mode::VisualizeMode:
			Visualize(args);
			break;
		default:
			break;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
}

void Generate(const LifeGameArgs& args)
{
	LifeGame game(args.width, args.height);
	game.Generate(args.probability);

	PrintGame(game, args.outFile);
}

void Step(const LifeGameArgs& args)
{
	const auto field = GetFieldFromFile(args.inFile);
	LifeGame game(field);
	game.GenerateNextStep(args.threads);

	PrintGame(game, args.outFile);
}

void Visualize(const LifeGameArgs& args)
{
	const auto field = GetFieldFromFile(args.inFile);
	LifeGame game(field);

	while (!game.IsEnd())
	{
		constexpr int sleepTime = 200;

		system("clear");
		game.GenerateNextStep(args.threads);
		PrintField(game.GetField(), '#', std::cout);
		std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
	}
}

void PrintGame(const LifeGame& game, const std::string& outFile)
{
	const auto newField = game.GetField();
	std::ofstream out(outFile);
	if (!out.is_open())
	{
		throw std::runtime_error("Cannot open file: " + outFile);
	}
	PrintField(newField, '#', out);
}

LifeGameArgs ParseArgs(const int argc, char* argv[])
{
	if (argc < 2)
	{
		throw std::runtime_error("Wrong argument count");
	}

	if (strcmp(argv[1], "generate") == 0)
	{
		return {
			argv[2],
			"",
			std::stoi(argv[3]),
			std::stoi(argv[4]),
			1,
			std::stod(argv[5]),
			Mode::GenerateMode,
		};
	}
	if (strcmp(argv[1], "step") == 0)
	{
		const std::string outFile = (argc == 5)
			? argv[4]
			: argv[2];
		return {
			outFile,
			argv[2],
			0,
			0,
			std::stoi(argv[3]),
			0,
			Mode::StepMode,
		};
	}
	if (strcmp(argv[1], "visualize") == 0)
	{
		return {
			"",
			argv[2],
			0,
			0,
			std::stoi(argv[3]),
			0,
			Mode::VisualizeMode,
		};
	}
	throw std::runtime_error("Wrong command");
}
