#pragma once
#include "Audio/AudioEngine.h"
#include "Score.h"
#include <map>
#include <string>

class Synthesizer
{
public:
	Synthesizer(ma_uint32 sampleRate, Score score);

	void ProcessAudio(float* output, ma_uint32 frameCount);

private:
	void InsertNextLine();

	void InsertActiveNote(size_t channel, const std::vector<Note>& notes);

private:
	struct ActiveNote
	{
		audio::WaveGenerator generator;
		float currentAmplitude;
		float targetAmplitude;
		float amplitudeStep;
		bool isChanging;
		bool isFading;

		ActiveNote(
			audio::WaveGenerator&& gen,
			const float startAmp,
			const float targetAmp,
			const float step,
			const bool changing,
			const bool fading)
			: generator(gen)
			, currentAmplitude(startAmp)
			, targetAmplitude(targetAmp)
			, amplitudeStep(step)
			, isChanging(changing)
			, isFading(fading)
		{}
	};

	ma_uint32 m_sampleRate;
	Score m_score;
	size_t m_currentLineIndex = 0;
	ma_uint32 m_samplesPerLine;
	ma_uint32 m_sampleCounter = 0;
	std::array<std::map<std::string, ActiveNote>, MAX_CHANNELS> m_activeNotes;
	bool m_isPlaying;
};