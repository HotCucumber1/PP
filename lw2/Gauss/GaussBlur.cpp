#include "GaussBlur.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <algorithm>
#include <stdexcept>
#include <thread>

constexpr double COLORS = 255;
constexpr double GAMMA = 2.2;

double ToLinear(unsigned char c);
unsigned char ToSrgb(double c);
std::vector<double> GenerateKernel(int radius);

GaussBlur::GaussBlur()
	: m_width(0)
	, m_height(0)
	, m_channels(0)
{
}

void GaussBlur::LoadImageFile(const std::string& path)
{
	const auto imgData = stbi_load(
		path.c_str(),
		&m_width,
		&m_height,
		&m_channels, 0);
	if (!imgData)
	{
		throw std::runtime_error("Failed to load image: " + path);
	}

	const auto totalPixels = m_width * m_height * m_channels;
	m_linearData.resize(totalPixels);
	m_tempData.resize(totalPixels);

	for (int i = 0; i < totalPixels; i++)
	{
		m_linearData[i] = ToLinear(imgData[i]);
	}

	stbi_image_free(imgData);
}

void GaussBlur::SaveImageFile(const std::string& path) const
{
	const auto totalPixels = m_width * m_height * m_channels;
	std::vector<unsigned char> outData(totalPixels);

	for (int i = 0; i < totalPixels; i++)
	{
		outData[i] = ToSrgb(m_linearData[i]);
	}

	const int stride = m_width * m_channels;
	if (stbi_write_png(
			path.c_str(),
			m_width,
			m_height,
			m_channels,
			outData.data(),
			stride)
		== 0)
	{
		throw std::runtime_error("Failed to save image: " + path);
	}
}

void GaussBlur::ApplyBlur(int radius, const int numThreads)
{
	if (radius <= 0)
	{
		return;
	}

	const auto kernel = GenerateKernel(radius);
	std::vector<std::jthread> threads;

	const int rowsPerThread = m_height / numThreads;
	for (int i = 0; i < numThreads; i++)
	{
		int startY = i * rowsPerThread;
		int endY = (i == numThreads - 1)
			? m_height
			: startY + rowsPerThread;
		threads.emplace_back(&GaussBlur::ProcessConvolution, this,
			startY,
			endY,
			radius,
			std::cref(kernel),
			std::cref(m_linearData),
			std::ref(m_tempData), true);
	}
	for (auto& t : threads)
	{
		t.join();
	}
	threads.clear();

	for (int i = 0; i < numThreads; ++i)
	{
		int startY = i * rowsPerThread;
		int endY = (i == numThreads - 1)
			? m_height
			: startY + rowsPerThread;
		threads.emplace_back(
			&GaussBlur::ProcessConvolution, this,
			startY,
			endY,
			radius,
			std::cref(kernel),
			std::cref(m_tempData),
			std::ref(m_linearData), false);
	}
}

double GaussBlur::GetPixelClamped(
	const std::vector<double>& data,
	const int x,
	const int y,
	const int c) const
{
	const int clampedX = std::clamp(x, 0, m_width - 1);
	const int clampedY = std::clamp(y, 0, m_height - 1);
	return data[(clampedY * m_width + clampedX) * m_channels + c];
}

void GaussBlur::ProcessConvolution(
	const int startY,
	const int endY,
	const int radius,
	const std::vector<double>& kernel,
	const std::vector<double>& source,
	std::vector<double>& destination,
	const bool isHorizontal) const
{
	for (int y = startY; y < endY; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			for (int c = 0; c < m_channels; c++)
			{
				double sum = 0;
				for (int k = -radius; k <= radius; k++)
				{
					double pixelValue;
					if (isHorizontal)
					{
						pixelValue = GetPixelClamped(source, x + k, y, c);
					}
					else
					{
						pixelValue = GetPixelClamped(source, x, y + k, c);
					}
					sum += pixelValue * kernel[k + radius];
				}
				destination[(y * m_width + x) * m_channels + c] = sum;
			}
		}
	}
}

double ToLinear(const unsigned char c)
{
	return std::pow(c / COLORS, GAMMA);
}

unsigned char ToSrgb(const double c)
{
	const auto clamped = std::clamp(c, 0.0, 1.0);
	return static_cast<unsigned char>(std::round(std::pow(clamped, 1.0 / GAMMA) * COLORS));
}

std::vector<double> GenerateKernel(const int radius)
{
	const int size = 2 * radius + 1;
	std::vector<double> kernel(size);
	double sum = 0;
	const double sigma = std::max(radius / 2.0, 1.0);

	for (int i = -radius; i <= radius; i++)
	{
		const auto val = std::exp(-(i * i) / (2 * sigma * sigma));
		kernel[i + radius] = val;
		sum += val;
	}

	for (auto& val : kernel)
	{
		val /= sum;
	}
	return kernel;
}
