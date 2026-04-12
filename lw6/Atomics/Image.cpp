#include "Image.h"

#include <random>

Image::Image(
	const int width,
	const int height)
	: m_width(width)
	, m_height(height)
	, m_data(3 * width * height, 0)
{
}

void Image::GenerateRandom()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution distribution(0, RGB_MAX);

	for (auto& pixel : m_data)
	{
		pixel = static_cast<uint8_t>(distribution(gen));
	}
}

const uint8_t* Image::GetData() const
{
	return m_data.data();
}

int Image::GetWidth() const
{
	return m_width;
}

int Image::GetHeight() const
{
	return m_height;
}
