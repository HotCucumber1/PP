#pragma once
#include <string>
#include <vector>

class GzipFileManager
{
public:
	explicit GzipFileManager(pid_t key);

	std::vector<std::string> CompressFiles(const std::vector<std::string>& files);

	void Cleanup();

private:
	std::vector<std::string> m_files;
	pid_t m_key;
};
