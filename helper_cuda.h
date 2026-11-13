#pragma once

#include <cuda_runtime_api.h>
#include <cstdio>
#include <cstdlib>

// Minimal CUDA Samples-compatible error checker used by this project.
inline void CheckCudaError(cudaError_t error, const char* functionName,
	const char* fileName, int lineNumber)
{
	if (error == cudaSuccess)
		return;

	std::fprintf(stderr, "CUDA error at %s:%d code=%d(%s) in %s\n",
		fileName, lineNumber, static_cast<int>(error),
		cudaGetErrorString(error), functionName);
	std::exit(EXIT_FAILURE);
}

#define checkCudaErrors(value) CheckCudaError((value), #value, __FILE__, __LINE__)
