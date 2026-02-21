#include "Unpacker.h"

#include <csignal>
#include <filesystem>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

void ExtractArchive(const std::string& inArchive, const std::string& outDir);
std::vector<std::string> GetAllZippedFiles(const std::string& dir);
bool CloseProcesses(const std::vector<pid_t>& pids);
int UnzipFile(const std::string& gzFile);

Unpacker::Unpacker(const int processes)
	: m_processes(processes)
{
}

unsigned long Unpacker::Unpack(
	const std::string& inArchive,
	const std::string& outDir) const
{
	const auto start = std::chrono::high_resolution_clock::now();
	if (m_processes == 0)
	{
		return 0;
	}
	ExtractArchive(inArchive, outDir);

	const auto gzFiles = GetAllZippedFiles(outDir);
	if (gzFiles.empty())
	{
		return 0;
	}
	const auto filesPerProcess = (gzFiles.size() + m_processes - 1) / m_processes;
	const auto end = std::chrono::high_resolution_clock::now();

	std::vector<pid_t> pids(m_processes);
	for (size_t i = 0; i < m_processes; i++)
	{
		const auto pid = fork();
		if (pid == 0)
		{
			const auto startIdx = i * filesPerProcess;
			const auto endIdx = std::min(startIdx + filesPerProcess, gzFiles.size());

			if (startIdx >= gzFiles.size())
			{
				_exit(0);
			}
			for (size_t j = startIdx; j < endIdx; j++)
			{
				const auto res = UnzipFile(gzFiles[j]);
				if (res != 0)
				{
					_exit(1);
				}
			}
			_exit(0);
		}
		if (pid > 0)
		{
			pids.push_back(pid);
		}
		else
		{
			for (const pid_t childPid : pids)
			{
				kill(childPid, SIGTERM);
			}
			throw std::runtime_error("Failed to fork process");
		}
	}
	const auto allSuccess = CloseProcesses(pids);
	if (!allSuccess)
	{
		throw std::runtime_error("Some processes failed to unpack files");
	}
	return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

std::vector<std::string> GetAllZippedFiles(const std::string& dir)
{
	std::vector<std::string> gzFiles;
	for (const auto& entry : std::filesystem::recursive_directory_iterator(dir))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".gz")
		{
			gzFiles.push_back(entry.path().string());
		}
	}
	return gzFiles;
}

int UnzipFile(const std::string& gzFile)
{
	const std::string cmd = "gzip -d -f \"" + gzFile + "\" > \"" + gzFile.substr(0, gzFile.length() - 3) + "\"";
	return system(cmd.c_str());
}

bool CloseProcesses(const std::vector<pid_t>& pids)
{
	int status;
	for (const pid_t pid : pids)
	{
		waitpid(pid, &status, 0);
		if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
		{
			return false;
		}
	}
	return true;
}

void ExtractArchive(
	const std::string& inArchive,
	const std::string& outDir)
{
	if (!std::filesystem::exists(inArchive))
	{
		throw std::runtime_error("File " + inArchive + " does not exist");
	}
	if (!std::filesystem::exists(outDir))
	{
		std::filesystem::create_directories(outDir);
	}

	const std::string cmd = "tar -xf " + inArchive + " -C " + outDir;
	if (system(cmd.c_str()) != 0)
	{
		throw std::runtime_error("Error while tar files");
	}
}