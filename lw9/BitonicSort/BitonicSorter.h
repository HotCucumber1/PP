#pragma once
#include <algorithm>
#include <chrono>
#include <CL/cl2.hpp>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>


const auto bitonicKernel = R"(
__kernel void bitonicSort(__global int* data,
                          const unsigned int n,
                          const unsigned int stage,
                          const unsigned int step) {

    unsigned int i = get_global_id(0);
    if (i >= n)
	{
		return;
	}

    unsigned int j = i ^ step;
    if (j >= n)
	{
		return;
	}

    if (i < j)
	{
        bool ascending = ((i / stage) % 2) == 0;

        if ((ascending && data[i] > data[j]) ||
            (!ascending && data[i] < data[j])
		) {
            int temp = data[i];
            data[i] = data[j];
            data[j] = temp;
        }
    }
}
)"; // TODO узбавиться от ветвлений в kernel

class BitonicSorter
{
public:
	void Sort(std::vector<int>& arr)
	{
		const auto n = arr.size();

		size_t paddedSize = 1;
		while (paddedSize < n)
		{
			paddedSize <<= 1;
		}

		std::vector padded(paddedSize, INT_MAX);
		std::ranges::copy(arr, padded.begin());

		SetupOpenCL();

		const cl::Buffer buffer(
			m_context,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			paddedSize * sizeof(int),
			padded.data());

		for (unsigned int blockSize = 2; blockSize <= paddedSize; blockSize <<= 1)
		{
			for (auto comparisonStep = blockSize >> 1; comparisonStep > 0; comparisonStep >>= 1)
			{
				m_kernel.setArg(0, buffer);
				m_kernel.setArg(1, static_cast<unsigned int>(paddedSize));
				m_kernel.setArg(2, blockSize);
				m_kernel.setArg(3, comparisonStep);

				const size_t globalSize = paddedSize;
				constexpr size_t localSize = 256;

				m_queue.enqueueNDRangeKernel(
					m_kernel, cl::NullRange,
					cl::NDRange(globalSize),
					cl::NDRange(localSize));
			}
		}
		m_queue.enqueueReadBuffer(buffer, CL_TRUE, 0, paddedSize * sizeof(int), padded.data());
		std::copy_n(padded.begin(), n, arr.begin());
	}

private:
	void SetupOpenCL()
	{
		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);
		const cl::Platform platform = platforms[0];

		std::vector<cl::Device> devices;
		platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);

		if (devices.empty())
		{
			platform.getDevices(CL_DEVICE_TYPE_CPU, &devices);
			std::cout << "Using CPU device" << std::endl;
		}
		else
		{
			std::cout << "Using GPU device" << std::endl;
		}

		m_device = devices[0];
		m_context = cl::Context(m_device);
		m_queue = cl::CommandQueue(m_context, m_device);

		const auto deviceName = m_device.getInfo<CL_DEVICE_NAME>();
		std::cout << "Device: " << deviceName << std::endl;

		cl::Program::Sources sources;
		sources.emplace_back(bitonicKernel, strlen(bitonicKernel));
		m_program = cl::Program(m_context, sources);

		const std::string buildOptions = "-cl-std=CL1.2";
		const auto err = m_program.build({ m_device }, buildOptions.c_str());

		if (err != CL_SUCCESS)
		{
			const auto buildLog = m_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(m_device);
			std::cerr << "Build error: " << buildLog << std::endl;
			throw std::runtime_error("Kernel build failed");
		}
		m_kernel = cl::Kernel(m_program, "bitonicSort");
	}

private:
	cl::Device m_device;
	cl::Context m_context;
	cl::CommandQueue m_queue;
	cl::Program m_program;
	cl::Kernel m_kernel;
};
