#include "Synthesizer.h"

#include <execution>
#include <numeric>
#include <ranges>
#include <utility>

constexpr float SEC_IN_MIN = 60;
constexpr float AMPLITUDE = 0.15;
constexpr float START_FREQ = 440;
constexpr float MIN_FREQ = 0.0001;
constexpr int SEMITONES_PER_OCTAVE = 12;
constexpr int TOTAL_SEMITONES = 57;
constexpr int COMMON_OCTAVE = 4;

float CalculateFrequency(const std::string& noteName);

Synthesizer::Synthesizer(const ma_uint32 sampleRate, Score score)
	: m_sampleRate(sampleRate)
	, m_score(std::move(score))
	, m_isPlaying(false)
{
	const auto linesPerSecond = static_cast<float>(m_score.tempo) / SEC_IN_MIN;
	m_samplesPerLine = static_cast<ma_uint32>(static_cast<float>(m_sampleRate) / linesPerSecond);

	if (!m_score.lines.empty())
	{
		m_isPlaying = true;
		ProcessNextLine();
	}
}

void Synthesizer::ProcessNextLine()
{
	if (m_currentLineIndex >= m_score.lines.size())
	{
		m_isPlaying = false;
		return;
	}

	const auto [channels, isEnd] = m_score.lines[m_currentLineIndex];
	if (isEnd)
	{
		m_isPlaying = false;
		return;
	}

	for (size_t ch = 0; ch < MAX_CHANNELS; ++ch)
	{
		const auto channelData = channels[ch];

		if (channelData.globalFade)
		{
			for (auto& pair : m_activeNotes[ch])
			{
				pair.second.isFading = true;
				pair.second.fadeRate = pair.second.currentAmplitude / static_cast<float>(m_samplesPerLine);
			}
		}

		for (const auto& note : channelData.notes)
		{
			const auto frequency = CalculateFrequency(note.name);

			ActiveNote newNote{
				audio::WaveGenerator(m_sampleRate, frequency, note.waveType, 1, 0),
				0.1,
				0,
				note.fade
			};
			if (newNote.isFading)
			{
				newNote.fadeRate = newNote.currentAmplitude / static_cast<float>(m_samplesPerLine);
			}
			m_activeNotes[ch].insert_or_assign(note.name, newNote);
		}
	}
}

void Synthesizer::ProcessAudio(float* output, const ma_uint32 frameCount)
{
	std::array<size_t, MAX_CHANNELS> channelsIndices{};
	std::iota(channelsIndices.begin(), channelsIndices.end(), 0);

	for (ma_uint32 i = 0; i < frameCount; ++i)
	{
		if (!m_isPlaying)
		{
			output[i] = 0;
			continue;
		}
		if (m_sampleCounter >= m_samplesPerLine)
		{
			for (size_t ch = 0; ch < MAX_CHANNELS; ++ch)
			{
				for (auto it = m_activeNotes[ch].begin(); it != m_activeNotes[ch].end();)
				{
					if (it->second.isFading && it->second.currentAmplitude <= MIN_FREQ)
					{
						it = m_activeNotes[ch].erase(it);
						continue;
					}
					++it;
				}
			}

			m_currentLineIndex++;
			m_sampleCounter = 0;
			ProcessNextLine();
		}

		const float mixedSample = std::transform_reduce(
			std::execution::par_unseq,
			channelsIndices.begin(),
			channelsIndices.end(),
			0.f,
			std::plus<float>{},
			[this](const size_t ch) -> float {
				float chSample = 0;
				for (auto& activeNote : m_activeNotes[ch] | std::views::values)
				{
					const float rawSample = activeNote.generator.GetNextSample();
					chSample += rawSample * activeNote.currentAmplitude;

					if (activeNote.isFading)
					{
						activeNote.currentAmplitude -= activeNote.fadeRate;
						if (activeNote.currentAmplitude < 0)
						{
							activeNote.currentAmplitude = 0;
						}
					}
				}
				return chSample;
			});

		output[i] = mixedSample;
		m_sampleCounter++;
	}
}

float CalculateFrequency(const std::string& noteName)
{
	if (noteName.empty())
	{
		return 0;
	}

	const auto letter = noteName[0];
	int noteOffset = 0;

	switch (letter)
	{
	case 'C': {
		noteOffset = 0;
		break;
	}
	case 'D': {
		noteOffset = 2;
		break;
	}
	case 'E': {
		noteOffset = 4;
		break;
	}
	case 'F': {
		noteOffset = 5;
		break;
	}
	case 'G': {
		noteOffset = 7;
		break;
	}
	case 'A': {
		noteOffset = 9;
		break;
	}
	case 'B': {
		noteOffset = 11;
		break;
	}
	default: {
		break;
	}
	}

	size_t octaveIndex = 1;
	if (noteName.size() > 1 && noteName[1] == '#')
	{
		noteOffset += 1;
		octaveIndex = 2;
	}

	int octave = COMMON_OCTAVE;
	if (octaveIndex < noteName.size())
	{
		octave = noteName[octaveIndex] - '0';
	}
	const auto totalSemitones = octave * SEMITONES_PER_OCTAVE + noteOffset;
	const auto diffFromA4 = totalSemitones - TOTAL_SEMITONES;

	return START_FREQ * std::pow(2.f, static_cast<float>(diffFromA4) / SEMITONES_PER_OCTAVE);
}
