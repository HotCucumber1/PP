#pragma once
#include <string>
#include <vector>

class GaussBlur
{
public:
	GaussBlur();

	void LoadImageFile(const std::string& path);

	void SaveImageFile(const std::string& path) const;

	void ApplyBlur(int radius, int numThreads = 1);

private:
	double GetPixelClamped(const std::vector<double>& data, int x, int y, int c) const;

	void ProcessConvolution(
		int startY,
		int endY,
		int radius,
		const std::vector<double>& kernel,
		const std::vector<double>& source,
		std::vector<double>& destination,
		bool isHorizontal) const;

	std::vector<double> m_linearData;
	std::vector<double> m_tempData;
	int m_width;
	int m_height;
	int m_channels;
};
