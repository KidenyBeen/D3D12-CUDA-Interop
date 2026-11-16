#pragma once

#include "stdafx.h"
#include <cuda_runtime_api.h>

// 공유 버퍼 소유권 전달 순서:
// DirectX 쓰기 완료 -> DX가 m_dxToCudaFence Signal -> CUDA가 Wait
// CUDA 쓰기 완료    -> CUDA가 m_cudaToDxFence Signal -> DX가 Wait
// GameFramework의 화면 출력용 Fence와 공유 메모리용 Fence는 역할이 달라 분리한다.
class CDirectXCudaInterop
{
public:
	enum class EExecutionMode { ComputeShaderOnly, CudaShared };

	struct KernelParameters
	{
		float m_fElapsedTime = 0.0f;
		float m_fValueScale = 1.0f;
		int m_nBlurRadius = 0;
		float m_fBlurSigma = 0.0f;
		int m_nBlurIterations = 1;
	};

	CDirectXCudaInterop() = default;
	~CDirectXCudaInterop();
	CDirectXCudaInterop(const CDirectXCudaInterop&) = delete;
	CDirectXCudaInterop& operator=(const CDirectXCudaInterop&) = delete;

	bool Initialize(ID3D12Device* device, size_t bufferSize, int cudaDeviceOrdinal = 0);
	bool InitializeTexture(ID3D12Device* device, ID3D12Resource* sharedTexture,
		DXGI_FORMAT format, UINT width, UINT height, int cudaDeviceOrdinal = 0);
	void Shutdown();
	// Standalone smoke test used by GameFramework::OnCreate. It does not touch
	// Scene or rendering resources and prints one CUDA addition result.
	static bool RunCudaConsoleTest();

	// 추후 ImGui RadioButton에서 호출할 실행 방식 전환 함수이다.
	// CUDA에서 Compute Shader로 바꿀 때 마지막 CUDA 쓰기를 먼저 기다린다.
	bool SetExecutionMode(EExecutionMode mode, ID3D12CommandQueue* directQueue);
	EExecutionMode GetExecutionMode() const { return m_executionMode; }

	// Signals DX->CUDA after earlier queue work, launches the template kernel,
	// then signals CUDA->DX. In compute-only mode this intentionally does nothing.
	bool ExecuteCuda(ID3D12CommandQueue* directQueue, const KernelParameters& parameters);
	// Queue before DirectX consumes CUDA-written data.
	bool WaitForCuda(ID3D12CommandQueue* directQueue);

	// Signal/Wait 단계를 분리해서 이후 세부 제출 순서를 조정할 수 있게 한다.
	bool SignalDirectXComplete(ID3D12CommandQueue* directQueue, UINT64* signaledValue = nullptr);
	bool WaitCudaForDirectX(UINT64 value);
	bool SignalCudaComplete(UINT64* signaledValue = nullptr);
	bool WaitDirectXForCuda(ID3D12CommandQueue* directQueue, UINT64 value);

	ID3D12Resource* GetSharedBuffer() const { return m_sharedBuffer.Get(); }
	ID3D12Resource* GetSharedTexture() const { return m_sharedTexture.Get(); }
	size_t GetSharedBufferSize() const { return m_bufferSize; }
	void* GetCudaBuffer() const { return m_cudaBuffer; }
	cudaStream_t GetCudaStream() const { return m_cudaStream; }
	const std::string& GetLastError() const { return m_lastError; }
	bool IsInitialized() const { return m_initialized; }

private:
	void InitCuda(ID3D12Device* device, int cudaDeviceOrdinal);
	void SetCudaDXDevice(int cudaDeviceOrdinal);
	bool CreateSharedBuffer(ID3D12Device* device, size_t bufferSize);
	bool CreateSharedFences(ID3D12Device* device); //Shared Fence를 만든다. 현재 단방향 2개의 Fence를 만든다.
	void ImportDXBufferToCuda(ID3D12Device* device);
	void ImportDXTextureToCuda(ID3D12Device* device);
	void ImportDXFencesToCuda(ID3D12Device* device);
	bool SetHRESULTError(const char* operation, HRESULT result);

	EExecutionMode m_executionMode = EExecutionMode::CudaShared;
	ComPtr<ID3D12Resource> m_sharedBuffer;
	ComPtr<ID3D12Resource> m_sharedTexture;
	ComPtr<ID3D12Fence> m_dxToCudaFence;
	ComPtr<ID3D12Fence> m_cudaToDxFence;
	cudaExternalMemory_t m_cudaExternalMemory = nullptr;
	cudaExternalSemaphore_t m_cudaWaitsForDx = nullptr;
	cudaExternalSemaphore_t m_dxWaitsForCuda = nullptr;
	cudaStream_t m_cudaStream = nullptr;
	void* m_cudaBuffer = nullptr;
	cudaMipmappedArray_t m_cudaSharedMipmappedArray = nullptr;
	cudaMipmappedArray_t m_cudaOriginalMipmappedArray = nullptr;
	cudaArray_t m_cudaSharedArray = nullptr;
	cudaArray_t m_cudaOriginalArray = nullptr;
	cudaArray_t m_cudaTempArray = nullptr;
	cudaSurfaceObject_t m_cudaSharedSurface = 0;
	cudaSurfaceObject_t m_cudaTempSurface = 0;
	cudaTextureObject_t m_cudaSharedTexture = 0;
	size_t m_bufferSize = 0;
	UINT m_nTextureWidth = 0;
	UINT m_nTextureHeight = 0;
	DXGI_FORMAT m_dxgiTextureFormat = DXGI_FORMAT_UNKNOWN;
	bool m_bTextureInterop = false;
	UINT64 m_nAllocationSize = 0;
	UINT64 m_dxToCudaValue = 0;
	UINT64 m_cudaToDxValue = 0;
	UINT64 m_pendingCudaValue = 0;
	UINT64 m_lastDxWaitedCudaValue = 0;
	bool m_initialized = false;
	std::string m_lastError;
};
