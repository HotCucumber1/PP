#pragma once
#include <cstdint>
#include <vector>

class Image
{
public:
	static constexpr int RGB_MAX = 255;

	Image(int width, int height);

	void GenerateRandom();

	const uint8_t* GetData() const;

	int GetWidth() const;

	int GetHeight() const;

private:
	int m_width;
	int m_height;
	std::vector<uint8_t> m_data;
};
