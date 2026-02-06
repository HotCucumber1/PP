#pragma once

#include <string>
#include <vector>

class Packer
{
public:
	explicit Packer(int processes = 1);

	unsigned long Pack(const std::string& outFile, const std::vector<std::string>& inFiles) const;

private:
	int m_processes;
};
