#pragma once
#include "Audio/AudioEngine.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

constexpr int MAX_CHANNELS = 10;
constexpr int DEFAULT_TEMPO = 120;

struct Note
{
	std::string name;
	bool fade = false;
	audio::Waveform waveType = audio::Waveform::Sine;
};

struct ChannelData
{
	std::vector<Note> notes;
	bool globalFade = false;
};

struct ScoreLine
{
	std::array<ChannelData, MAX_CHANNELS> channels;
	bool isEnd = false;
};

struct Score
{
	int tempo = DEFAULT_TEMPO;
	std::vector<ScoreLine> lines;
};

inline Score Parse(const std::string& filename)
{
	Score score;

	std::ifstream file(filename);
	if (!file.is_open())
	{
		return score;
	}

	std::string lineStr;
	if (std::getline(file, lineStr))
	{
		score.tempo = std::stoi(lineStr);
	}

	while (std::getline(file, lineStr))
	{
		ScoreLine scoreLine;

		if (lineStr.find("END") != std::string::npos)
		{
			scoreLine.isEnd = true;
			score.lines.push_back(scoreLine);
			break;
		}

		std::istringstream iss(lineStr);
		std::string channelStr;
		size_t channelIndex = 0;

		while (std::getline(iss, channelStr, '|') && channelIndex < MAX_CHANNELS)
		{
			std::istringstream channelIss(channelStr);
			std::string token;

			while (channelIss >> token)
			{
				if (token == "-")
				{
					scoreLine.channels[channelIndex].globalFade = true;
					continue;
				}
				Note note;
				std::string rawToken = token;

				if (!rawToken.empty() && rawToken.back() == '-')
				{
					note.fade = true;
					rawToken.pop_back();
				}

				if (!rawToken.empty())
				{
					const auto lastChar = rawToken.back();
					if (lastChar == 'P')
					{
						note.waveType = audio::Waveform::Pulse;
						rawToken.pop_back();
					}
					else if (lastChar == '\\')
					{
						note.waveType = audio::Waveform::Sawtooth;
						rawToken.pop_back();
					}
					else if (lastChar == 'W')
					{
						note.waveType = audio::Waveform::Triangle;
						rawToken.pop_back();
					}
				}

				note.name = rawToken;
				scoreLine.channels[channelIndex].notes.push_back(note);
			}
			channelIndex++;
		}
		score.lines.push_back(scoreLine);
	}
	return score;
}
