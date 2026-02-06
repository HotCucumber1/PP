#include "GzipFileManager.h"

#include <filesystem>
#include <stdexcept>

GzipFileManager::GzipFileManager(const pid_t key)
	: m_key(key)
{
}

std::vector<std::string> GzipFileManager::CompressFiles(const std::vector<std::string>& files)
{
	std::vector<std::string> result;

	for (const auto& file : files)
	{
		std::filesystem::path filePath(file);
		std::string compressedFile = "/tmp/" + filePath.filename().string() + ".gz";
		std::string cmd = "gzip -c \"" + file + "\" > \"" + compressedFile + "\"";

		if (system(cmd.c_str()) != 0)
		{
			Cleanup();
			throw std::runtime_error("Error compressing: " + file);
		}

		result.push_back(compressedFile);
		m_files.push_back(compressedFile);
	}
	return result;
}

void GzipFileManager::Cleanup()
{
	for (const auto& file : m_files)
	{
		if (std::filesystem::exists(file))
		{
			remove(file.c_str());
		}
	}
	m_files.clear();
}