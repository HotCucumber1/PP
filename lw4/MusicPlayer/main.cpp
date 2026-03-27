#define MINIAUDIO_IMPLEMENTATION

#include "Audio/AudioEngine.h"
#include "Score.h"
#include "Synthesizer.h"

#include <iostream>

constexpr int CONSOLE_WIDTH = 80;

std::string GetInputFileName(int argc, char* argv[]);
void Draw(float amplitude);

int main(int argc, char* argv[])
{
	const auto filename = GetInputFileName(argc, argv);

	const auto score = Parse(filename);
	if (score.lines.empty())
	{
		std::cout << "Failed to read file or file is empty. File: " << filename << std::endl;
		return 1;
	}

	try
	{
		audio::Player player(ma_format_f32, 1);
		Synthesizer synth(player.GetSampleRate(), score);

		player.SetDataCallback([&synth](void* output, const ma_uint32 frameCount) {
			synth.ProcessAudio(static_cast<float*>(output), frameCount);
		});

		// FIXME
		// player.SetDataCallback([&synth](void* output, const ma_uint32 frameCount) {
		// 	const auto outBuffer = static_cast<float*>(output);
		// 	synth.ProcessAudio(outBuffer, frameCount);
		//
		// 	if (frameCount > 0)
		// 	{
		// 		Draw(outBuffer[0]);
		// 	}
		// });

		player.Start();

		std::cout << "Playing audio... Click `Enter` to stop." << std::endl;
		std::string s;
		std::getline(std::cin, s);
		player.Stop();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}

std::string GetInputFileName(const int argc, char* argv[])
{
	if (argc < 2)
	{
		throw std::runtime_error("Need to specify input file");
	}
	return argv[1];
}

void Draw(const float amplitude)
{
	auto pos = static_cast<int>((amplitude + 1) * 0.5 * CONSOLE_WIDTH);
	if (pos < 0)
	{
		pos = 0;
	}
	if (pos >= CONSOLE_WIDTH)
	{
		pos = CONSOLE_WIDTH - 1;
	}

	std::string line(CONSOLE_WIDTH, ' ');
	line[pos] = '*';
	std::cout << line << '\n';
}