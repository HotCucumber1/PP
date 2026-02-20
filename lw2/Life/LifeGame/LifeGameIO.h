#pragma once
#include "LifeGame.h"

#include <fstream>
#include <limits>

inline void PrintField(
	const LifeGame::Field& field,
	const char ch = '#',
	std::ostream& out = std::cout)
{
	if (field.empty())
	{
		return;
	}
	out << field[0].size() << ' ' << field.size() << std::endl;

	for (const auto& row : field)
	{
		for (const auto& isAlive : row)
		{
			if (isAlive)
			{
				out << ch;
			}
			else
			{
				out << '-';
			}
		}
		out << std::endl;
	}
}

inline LifeGame::Field GetFieldFromFile(const std::string& inFile)
{
	std::ifstream file(inFile);
	if (!file.is_open())
	{
		throw std::runtime_error("Cannot open file: " + inFile);
	}

	int width, height;
	if (!(file >> width >> height))
	{
		throw std::runtime_error("Failed to read field dimensions");
	}
	file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	std::vector field(height, std::vector(width, false));
	std::string line;
	for (int row = 0; row < height; ++row)
	{
		if (!std::getline(file, line))
		{
			throw std::runtime_error("Unexpected end of file");
		}

		for (int col = 0; col < width && col < line.length(); ++col)
		{
			field[row][col] = (line[col] == '#');
		}
	}
	return field;
}
