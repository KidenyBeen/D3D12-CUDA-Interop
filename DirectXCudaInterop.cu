#include <cuda_runtime.h>
#include <cuda_fp16.h>
#include "helper_cuda.h"

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
	if (!sharedBuffer || elementCount == 0) return cudaErrorInvalidValue;
	constexpr unsigned int threadsPerBlock = 256;
	const unsigned int blockCount = static_cast<unsigned int>(
		(elementCount + threadsPerBlock - 1) / threadsPerBlock);
	DirectXCudaInteropKernel<<<blockCount, threadsPerBlock, 0, stream>>>(
		static_cast<float*>(sharedBuffer), elementCount, elapsedTime, valueScale);
	checkCudaErrors(cudaGetLastError());
	return cudaSuccess;
}

__device__ float4 HalfPixelToFloat4(ushort4 pixel)
{
	return make_float4(__half2float(__ushort_as_half(pixel.x)),
		__half2float(__ushort_as_half(pixel.y)),
		__half2float(__ushort_as_half(pixel.z)),
		__half2float(__ushort_as_half(pixel.w)));
}

__device__ ushort4 Float4ToHalfPixel(float4 pixel)
{
	return make_ushort4(__half_as_ushort(__float2half_rn(pixel.x)),
		__half_as_ushort(__float2half_rn(pixel.y)),
		__half_as_ushort(__float2half_rn(pixel.z)),
		__half_as_ushort(__float2half_rn(pixel.w)));
}

__device__ float GaussianWeight(int offset, float sigma)
{
	return expf(-(float)(offset * offset) / (2.0f * sigma * sigma));
}

__global__ void GaussianBlurHorizontalKernel(cudaSurfaceObject_t sharedSurface,
	cudaSurfaceObject_t tempSurface, unsigned int nWidth, unsigned int nHeight,
	int nRadius, float fSigma)
{
	const unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	const unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;
	if (x >= nWidth || y >= nHeight) return;

	float4 color = make_float4(0.0f, 0.0f, 0.0f, 0.0f);
	float totalWeight = 0.0f;
	for (int offset = -nRadius; offset <= nRadius; ++offset)
	{
		const int sampleX = max(0, min((int)nWidth - 1, (int)x + offset));
		const float weight = GaussianWeight(offset, fSigma);
		const ushort4 pixel = surf2Dread<ushort4>(
			sharedSurface, sampleX * sizeof(ushort4), y);
		const float4 sample = HalfPixelToFloat4(pixel);
		color.x += sample.x * weight; color.y += sample.y * weight;
		color.z += sample.z * weight; color.w += sample.w * weight;
		totalWeight += weight;
	}
	color.x /= totalWeight; color.y /= totalWeight;
	color.z /= totalWeight; color.w /= totalWeight;
	surf2Dwrite(Float4ToHalfPixel(color), tempSurface, x * sizeof(ushort4), y);

}

__global__ void GaussianBlurVerticalKernel(cudaSurfaceObject_t tempSurface,
	cudaSurfaceObject_t sharedSurface, unsigned int nWidth, unsigned int nHeight,
	int nRadius, float fSigma)
{
	const unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	const unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;
	if (x >= nWidth || y >= nHeight) return;

	float4 color = make_float4(0.0f, 0.0f, 0.0f, 0.0f);
	float totalWeight = 0.0f;
	for (int offset = -nRadius; offset <= nRadius; ++offset)
	{
		const int sampleY = max(0, min((int)nHeight - 1, (int)y + offset));
		const float weight = GaussianWeight(offset, fSigma);
		const ushort4 pixel = surf2Dread<ushort4>(
			tempSurface, x * sizeof(ushort4), sampleY);
		const float4 sample = HalfPixelToFloat4(pixel);
		color.x += sample.x * weight; color.y += sample.y * weight;
		color.z += sample.z * weight; color.w += sample.w * weight;
		totalWeight += weight;
	}
	color.x /= totalWeight; color.y /= totalWeight;
	color.z /= totalWeight; color.w /= totalWeight;
	surf2Dwrite(Float4ToHalfPixel(color), sharedSurface, x * sizeof(ushort4), y);
}

extern "C" cudaError_t LaunchDirectXCudaInteropTextureKernel(cudaSurfaceObject_t sharedSurface,
	cudaSurfaceObject_t tempSurface, unsigned int nWidth, unsigned int nHeight,
	int nRadius, float fSigma, cudaStream_t stream)
{
	if (!sharedSurface || !tempSurface || !nWidth || !nHeight) return cudaErrorInvalidValue;
	nRadius = max(0, min(nRadius, 32));
	if (nRadius == 0) return cudaSuccess;
	if (fSigma <= 0.0f) fSigma = max(1.0f, nRadius * 0.5f);

	const dim3 block(16, 16);
	const dim3 grid((nWidth + block.x - 1) / block.x, (nHeight + block.y - 1) / block.y);
	GaussianBlurHorizontalKernel<<<grid, block, 0, stream>>>(
		sharedSurface, tempSurface, nWidth, nHeight, nRadius, fSigma);
	checkCudaErrors(cudaGetLastError());
	GaussianBlurVerticalKernel<<<grid, block, 0, stream>>>(
		tempSurface, sharedSurface, nWidth, nHeight, nRadius, fSigma);
	checkCudaErrors(cudaGetLastError());
	return cudaSuccess;
}

__global__ void CudaConsoleAddKernel(const int* pLeft, const int* pRight, int* pResult)
{
	// CUDA 실행 여부만 확인하는 1개 스레드 덧셈 테스트이다.
	if (blockIdx.x == 0 && threadIdx.x == 0)
		*pResult = *pLeft + *pRight;
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
