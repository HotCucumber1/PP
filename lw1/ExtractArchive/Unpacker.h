#pragma once
#include <string>

class Unpacker
{
public:
	explicit Unpacker(int processes = 1);

	unsigned long Unpack(
		const std::string& inArchive,
		const std::string& outDir) const;

private:
	int m_processes;
};
