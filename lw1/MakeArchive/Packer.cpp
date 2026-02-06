#include "Packer.h"

#include "GzipFileManager.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

const std::string TMP_FILE_PREFIX = "/tmp/packer_result_";
constexpr int MAX_PROCESSES = 16;

void ArchiveFiles(const std::string& outFile, const std::vector<std::string>& files);
void CollectCompressedFiles(pid_t pid, std::vector<std::string>& compressedFiles);
void CompressFiles(const std::vector<std::string>& files);

Packer::Packer(const int processes)
	: m_processes(processes)
{
	if (m_processes > MAX_PROCESSES)
	{
		throw std::runtime_error("Processes count must be " + std::to_string(MAX_PROCESSES) + " at most");
	}
}

unsigned long Packer::Pack(
	const std::string& outFile,
	const std::vector<std::string>& inFiles) const
{
	const int processesCount = std::min(
		m_processes,
		static_cast<int>(inFiles.size()));

	if (processesCount == 0)
	{
		return 0;
	}

	std::vector<pid_t> pids(processesCount);
	for (size_t i = 0; i < processesCount; i++)
	{
		const auto pid = fork();
		if (pid == 0)
		{
			std::vector<std::string> myFiles;
			for (size_t j = i; j < inFiles.size(); j += processesCount)
			{
				myFiles.push_back(inFiles[j]);
			}

			CompressFiles(myFiles);
			exit(0);
		}
		if (pid > 0)
		{
			pids[i] = pid;
		}
		else
		{
			throw std::runtime_error("Failed to fork process");
		}
	}

	std::vector<std::string> allCompressed;
	for (int i = 0; i < processesCount; i++)
	{
		int status;
		waitpid(pids[i], &status, 0);
		if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		{
			CollectCompressedFiles(pids[i], allCompressed);
		}
		else
		{
			std::cerr << "Process " << pids[i] << " failed" << std::endl;
		}
	}

	const auto start = std::chrono::high_resolution_clock::now();

	ArchiveFiles(outFile, allCompressed);
	for (const auto& file : allCompressed)
	{
		remove(file.c_str());
	}

	const auto end = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

void CompressFiles(const std::vector<std::string>& files)
{
	GzipFileManager packer(getpid());
	try
	{
		const auto compressed = packer.CompressFiles(files);

		const auto tmpFile = TMP_FILE_PREFIX + std::to_string(getpid());
		std::ofstream tmpFiles(tmpFile);
		for (const auto& f : compressed)
		{
			tmpFiles << f << "\n";
		}
		tmpFiles.close();
	}
	catch (const std::exception&)
	{
		packer.Cleanup();
		exit(1);
	}
}

void CollectCompressedFiles(const pid_t pid, std::vector<std::string>& compressedFiles)
{
	const auto resultFile = TMP_FILE_PREFIX + std::to_string(pid);
	std::ifstream input(resultFile);
	if (input)
	{
		std::string filename;
		while (std::getline(input, filename))
		{
			compressedFiles.push_back(filename);
		}
		input.close();
		remove(resultFile.c_str());
	}
}

void ArchiveFiles(const std::string& outFile, const std::vector<std::string>& files)
{
	if (files.empty())
	{
		return;
	}
	std::string cmd = "tar -cf " + outFile;
	for (const auto& file : files)
	{
		cmd += " " + file;
	}

	cmd += " 2>/dev/null";
	if (system(cmd.c_str()) != 0)
	{
		throw std::runtime_error("Error while tar files");
	}
}
