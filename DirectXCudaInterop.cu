#include <cuda_runtime.h>
#include "helper_cuda.h"
#include <cstdio>

namespace
{
	cudaError_t ValidateKernelStage(const char* stage, cudaStream_t stream)
	{
		cudaError_t result = cudaGetLastError();
		if (result != cudaSuccess)
		{
			std::fprintf(stderr, "[CUDA Interop] %s launch failed: %s\n",
				stage, cudaGetErrorString(result));
			return result;
		}
#if defined(_DEBUG)
		// CUDA 실행 오류는 비동기로 보고되므로 Debug에서는 각 단계 직후 확인한다.
		result = cudaStreamSynchronize(stream);
		if (result != cudaSuccess)
		{
			std::fprintf(stderr, "[CUDA Interop] %s execution failed: %s\n",
				stage, cudaGetErrorString(result));
		}
#endif
		return result;
	}
}

// DX12 공유 버퍼에 적용할 기본 CUDA 커널 템플릿이다.
// 실제 데이터 구조가 정해지면 이 연산부를 교체하거나 전용 커널을 추가한다.
__global__ void DirectXCudaInteropKernel(
	float* sharedValues, size_t elementCount, float elapsedTime, float valueScale)
{
	const size_t index = blockIdx.x * blockDim.x + threadIdx.x;
	if (index >= elementCount)
	{
		return;
	}
	sharedValues[index] = sharedValues[index] * valueScale + elapsedTime * 0.0f;
}

extern "C" cudaError_t LaunchDirectXCudaInteropKernel(
	void* sharedBuffer, size_t elementCount,
	float elapsedTime, float valueScale, cudaStream_t stream)
{
	if (!sharedBuffer || elementCount == 0)
	{
		return cudaErrorInvalidValue;
	}
	constexpr unsigned int threadsPerBlock = 256;
	const unsigned int blockCount = static_cast<unsigned int>(
		(elementCount + threadsPerBlock - 1) / threadsPerBlock);
	DirectXCudaInteropKernel<<<blockCount, threadsPerBlock, 0, stream>>>(
		static_cast<float*>(sharedBuffer), elementCount, elapsedTime, valueScale);
	checkCudaErrors(cudaGetLastError());
	return cudaSuccess;
}

__device__ float4 UnormPixelToFloat4(uchar4 pixel)
{
	constexpr float unormScale = 1.0f / 255.0f;
	return make_float4(pixel.x * unormScale, pixel.y * unormScale,
		pixel.z * unormScale, pixel.w * unormScale);
}

__device__ float ClampUnorm(float value)
{
	return fminf(1.0f, fmaxf(0.0f, value));
}

__device__ uchar4 Float4ToUnormPixel(float4 pixel)
{
	return make_uchar4(
		static_cast<unsigned char>(ClampUnorm(pixel.x) * 255.0f + 0.5f),
		static_cast<unsigned char>(ClampUnorm(pixel.y) * 255.0f + 0.5f),
		static_cast<unsigned char>(ClampUnorm(pixel.z) * 255.0f + 0.5f),
		static_cast<unsigned char>(ClampUnorm(pixel.w) * 255.0f + 0.5f));
}

__device__ float GaussianWeight(int offset, float sigma)
{
	return expf(-(float)(offset * offset) / (2.0f * sigma * sigma));
}

// DX12가 매 프레임 렌더링한 공유 Texture를 CUDA 전용 원본에 보존한다.
// 이후 블러 패스는 공유 Texture를 입력으로 다시 읽지 않는다.
__global__ void CopyTextureKernel(cudaSurfaceObject_t destinationSurface,
	cudaTextureObject_t sourceTexture, unsigned int nWidth, unsigned int nHeight)
{
	const unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	const unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;
	if (x >= nWidth || y >= nHeight)
	{
		return;
	}

	const float u = (static_cast<float>(x) + 0.5f) / static_cast<float>(nWidth);
	const float v = (static_cast<float>(y) + 0.5f) / static_cast<float>(nHeight);
	const float4 pixel = tex2D<float4>(sourceTexture, u, v);
	surf2Dwrite(Float4ToUnormPixel(pixel), destinationSurface,
		x * sizeof(uchar4), y);
}

__global__ void GaussianBlurHorizontalKernel(cudaSurfaceObject_t originalSurface,
	cudaSurfaceObject_t tempSurface, unsigned int nWidth, unsigned int nHeight,
	int nRadius, float fSigma)
{
	const unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	const unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;
	if (x >= nWidth || y >= nHeight)
	{
		return;
	}

	float4 color = make_float4(0.0f, 0.0f, 0.0f, 0.0f);
	float totalWeight = 0.0f;
	for (int offset = -nRadius; offset <= nRadius; ++offset)
	{
		const int sampleX = max(0, min((int)nWidth - 1, (int)x + offset));
		const float weight = GaussianWeight(offset, fSigma);
		const uchar4 pixel = surf2Dread<uchar4>(
			originalSurface, sampleX * sizeof(uchar4), y);
		const float4 sample = UnormPixelToFloat4(pixel);
		color.x += sample.x * weight; color.y += sample.y * weight;
		color.z += sample.z * weight; color.w += sample.w * weight;
		totalWeight += weight;
	}
	color.x /= totalWeight; color.y /= totalWeight;
	color.z /= totalWeight; color.w /= totalWeight;
	surf2Dwrite(Float4ToUnormPixel(color), tempSurface, x * sizeof(uchar4), y);

}

__global__ void GaussianBlurVerticalKernel(cudaSurfaceObject_t tempSurface,
	cudaSurfaceObject_t sharedSurface, unsigned int nWidth, unsigned int nHeight,
	int nRadius, float fSigma)
{
	const unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	const unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;
	if (x >= nWidth || y >= nHeight)
	{
		return;
	}

	float4 color = make_float4(0.0f, 0.0f, 0.0f, 0.0f);
	float totalWeight = 0.0f;
	for (int offset = -nRadius; offset <= nRadius; ++offset)
	{
		const int sampleY = max(0, min((int)nHeight - 1, (int)y + offset));
		const float weight = GaussianWeight(offset, fSigma);
		const uchar4 pixel = surf2Dread<uchar4>(
			tempSurface, x * sizeof(uchar4), sampleY);
		const float4 sample = UnormPixelToFloat4(pixel);
		color.x += sample.x * weight; color.y += sample.y * weight;
		color.z += sample.z * weight; color.w += sample.w * weight;
		totalWeight += weight;
	}
	color.x /= totalWeight; color.y /= totalWeight;
	color.z /= totalWeight; color.w /= totalWeight;
	surf2Dwrite(Float4ToUnormPixel(color), sharedSurface, x * sizeof(uchar4), y);
}

extern "C" cudaError_t LaunchDirectXCudaInteropTextureKernel(cudaTextureObject_t sharedTexture,
	cudaSurfaceObject_t sharedSurface, cudaSurfaceObject_t tempSurface,
	unsigned int nWidth, unsigned int nHeight,
	int nRadius, float fSigma, int nIterations, cudaStream_t stream)
{
	if (!sharedTexture || !sharedSurface || !tempSurface || !nWidth || !nHeight)
	{
		return cudaErrorInvalidValue;
	}
	nRadius = max(0, min(nRadius, 32));
	nIterations = max(1, min(nIterations, 100));
	if (fSigma <= 0.0f)
	{
		fSigma = max(1.0f, nRadius * 0.5f);
	}

	const dim3 block(16, 16);
	const dim3 grid((nWidth + block.x - 1) / block.x, (nHeight + block.y - 1) / block.y);
	CopyTextureKernel<<<grid, block, 0, stream>>>(
		sharedSurface, sharedTexture, nWidth, nHeight);
	cudaError_t result = ValidateKernelStage("CopyTextureKernel", stream);
	if (result != cudaSuccess)
	{
		return result;
	}
	if (nRadius == 0)
	{
		return cudaSuccess;
	}
	for (int iteration = 0; iteration < nIterations; ++iteration)
	{
		GaussianBlurHorizontalKernel<<<grid, block, 0, stream>>>(
			sharedSurface, tempSurface, nWidth, nHeight, nRadius, fSigma);
		result = ValidateKernelStage("GaussianBlurHorizontalKernel", stream);
		if (result != cudaSuccess)
		{
			return result;
		}
		GaussianBlurVerticalKernel<<<grid, block, 0, stream>>>(
			tempSurface, sharedSurface, nWidth, nHeight, nRadius, fSigma);
		result = ValidateKernelStage("GaussianBlurVerticalKernel", stream);
		if (result != cudaSuccess)
		{
			return result;
		}
	}
	return cudaSuccess;
}

__global__ void CudaConsoleAddKernel(const int* pLeft, const int* pRight, int* pResult)
{
	// CUDA 실행 여부만 확인하는 1개 스레드 덧셈 테스트이다.
	if (blockIdx.x == 0 && threadIdx.x == 0)
	{
		*pResult = *pLeft + *pRight;
	}
}

extern "C" cudaError_t RunDirectXCudaConsoleAdd(int nLeft, int nRight, int* pResult)
{
	if (!pResult)
	{
		return cudaErrorInvalidValue;
	}
	int* pDeviceLeft = nullptr;
	int* pDeviceRight = nullptr;
	int* pDeviceResult = nullptr;
	checkCudaErrors(cudaMalloc(&pDeviceLeft, sizeof(int)));
	checkCudaErrors(cudaMalloc(&pDeviceRight, sizeof(int)));
	checkCudaErrors(cudaMalloc(&pDeviceResult, sizeof(int)));
	checkCudaErrors(cudaMemcpy(pDeviceLeft, &nLeft, sizeof(int), cudaMemcpyHostToDevice));
	checkCudaErrors(cudaMemcpy(pDeviceRight, &nRight, sizeof(int), cudaMemcpyHostToDevice));
	CudaConsoleAddKernel<<<1, 1>>>(pDeviceLeft, pDeviceRight, pDeviceResult);
	checkCudaErrors(cudaGetLastError());
	checkCudaErrors(cudaMemcpy(pResult, pDeviceResult, sizeof(int), cudaMemcpyDeviceToHost));
	checkCudaErrors(cudaFree(pDeviceResult));
	checkCudaErrors(cudaFree(pDeviceRight));
	checkCudaErrors(cudaFree(pDeviceLeft));
	return cudaSuccess;
}
