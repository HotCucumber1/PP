#include "./LifeGame.h"

#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>
#include <random>

bool GenerateLife(double probability);

LifeGame::LifeGame(
	const int width,
	const int height,
	std::ostream& logOut)
	: m_width(width)
	, m_height(height)
	, m_logOut(logOut)
{
	m_field = std::vector(m_height, std::vector<bool>(m_width));
}

LifeGame::LifeGame(const Field& field, std::ostream& logOut)
	: m_field(field)
	, m_logOut(logOut)
{
	m_height = field.size();
	m_width = (m_height > 0)
		? field[0].size()
		: 0;
}

LifeGame::Field LifeGame::GetField() const
{
	return m_field;
}

void LifeGame::Generate(const double probability)
{
	for (int i = 0; i < m_height; i++)
	{
		for (int j = 0; j < m_width; j++)
		{
			m_field[i][j] = GenerateLife(probability);
		}
	}
}

void LifeGame::GenerateNextStep(const int threads)
{
	const Field srcField = m_field;

	boost::asio::thread_pool threadPool(threads);

	const auto start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < m_height; i++)
	{
		boost::asio::post(threadPool, [this, i, &srcField] {
			for (int j = 0; j < m_width; j++)
			{
				const auto neighboursAlive = GetCellAliveNeighboursCount(i, j, srcField);
				if (srcField[i][j])
				{
					if (neighboursAlive != 2 && neighboursAlive != 3)
					{
						m_field[i][j] = false;
					}
				}
				else if (neighboursAlive == 3)
				{
					m_field[i][j] = true;
				}
			}
		});
	}
	threadPool.join();

	const auto end = std::chrono::high_resolution_clock::now();
	const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	m_logOut << "Generation time: " << duration.count() << "mcs" << std::endl;
}

size_t LifeGame::GetCellAliveNeighboursCount(const int i, const int j, const Field& field) const
{
	size_t count = 0;

	for (int di = -1; di <= 1; di++)
	{
		for (int dj = -1; dj <= 1; dj++)
		{
			if (di == 0 && dj == 0)
			{
				continue;
			}

			const int ni = (i + di + m_height) % m_height;
			const int nj = (j + dj + m_width) % m_width;

			if (field[ni][nj])
			{
				count++;
			}
		}
	}
	return count;
}

bool GenerateLife(const double probability)
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::bernoulli_distribution dist(probability);
	return dist(gen);
}
