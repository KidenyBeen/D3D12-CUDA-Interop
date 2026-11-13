#include <cuda_runtime.h>
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
