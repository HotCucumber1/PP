#pragma once
#include <iostream>
#include <vector>

class LifeGame
{
public:
	using Field = std::vector<std::vector<bool>>;

	LifeGame(
		int width,
		int height,
		std::ostream& logOut = std::cout);

	explicit LifeGame(
		const Field& field,
		std::ostream& logOut = std::cout);

	void Generate(double probability = 0.5);

	void GenerateNextStep(int threads = 1);

	Field GetField() const;

private:
	size_t GetCellAliveNeighboursCount(int i, int j, const Field& field) const;

private:
	int m_width;
	int m_height;
	Field m_field;
	std::ostream& m_logOut;
};
