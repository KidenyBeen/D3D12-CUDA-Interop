#include "DirectXCudaInterop.h"
#include "helper_cuda.h"

#include <iostream>
#include <sstream>

extern "C" cudaError_t LaunchDirectXCudaInteropKernel(
	void*, size_t, float, float, cudaStream_t);
extern "C" cudaError_t RunDirectXCudaConsoleAdd(int, int, int*);

namespace
{
	HANDLE CreateExportHandle(ID3D12Device* device, ID3D12DeviceChild* object)
	{
		HANDLE handle = nullptr;
		return SUCCEEDED(device->CreateSharedHandle(object, nullptr, GENERIC_ALL, nullptr, &handle))
			? handle : nullptr;
	}
}

CDirectXCudaInterop::~CDirectXCudaInterop() { Shutdown(); }

bool CDirectXCudaInterop::RunCudaConsoleTest()
{
	constexpr int nLeft = 6;
	constexpr int nRight = 7;
	int nResult = 0;
	const cudaError_t cudaResult = RunDirectXCudaConsoleAdd(nLeft, nRight, &nResult);
	if (cudaResult != cudaSuccess)
	{
		std::cout << "[CUDA Test] Failed: " << cudaGetErrorString(cudaResult) << std::endl;
		return false;
	}
	std::cout << "[CUDA Test] " << nLeft << " + " << nRight << " = " << nResult << std::endl;
	return nResult == nLeft + nRight;
}

bool CDirectXCudaInterop::Initialize(ID3D12Device* device, size_t bufferSize, int cudaDeviceOrdinal)
{
	Shutdown();
	if (!device || bufferSize == 0)
	{
		m_lastError = "Initialize requires a D3D12 device and non-zero buffer size.";
		return false;
	}
	if (!CreateSharedBuffer(device, bufferSize) || !CreateSharedFences(device))
	{
		const std::string error = m_lastError;
		Shutdown();
		m_lastError = error;
		return false;
	}

	InitCuda(device, cudaDeviceOrdinal);
	m_initialized = true;
	return true;
}

void CDirectXCudaInterop::InitCuda(ID3D12Device* device, int cudaDeviceOrdinal)
{
	// VulkanCudaInterop::InitCuda와 같은 순서로 CUDA 실행 환경을 구성한다.
	SetCudaDXDevice(cudaDeviceOrdinal);
	checkCudaErrors(cudaStreamCreateWithFlags(&m_cudaStream, cudaStreamNonBlocking));
	ImportDXBufferToCuda(device);
	ImportDXFencesToCuda(device);
}

void CDirectXCudaInterop::SetCudaDXDevice(int cudaDeviceOrdinal)
{
	// DX12 디바이스를 만든 GPU와 같은 번호의 CUDA 디바이스를 선택해야 한다.
	// 현재 단계에서는 전달받은 디바이스 번호를 직접 설정하는 단순한 구조로 둔다.
	checkCudaErrors(cudaSetDevice(cudaDeviceOrdinal));
}

void CDirectXCudaInterop::Shutdown()
{
	if (m_cudaStream) checkCudaErrors(cudaStreamSynchronize(m_cudaStream));
	if (m_dxWaitsForCuda) checkCudaErrors(cudaDestroyExternalSemaphore(m_dxWaitsForCuda));
	if (m_cudaWaitsForDx) checkCudaErrors(cudaDestroyExternalSemaphore(m_cudaWaitsForDx));
	if (m_cudaExternalMemory) checkCudaErrors(cudaDestroyExternalMemory(m_cudaExternalMemory));
	if (m_cudaStream) checkCudaErrors(cudaStreamDestroy(m_cudaStream));
	m_cudaStream = nullptr; m_cudaBuffer = nullptr; m_cudaExternalMemory = nullptr;
	m_cudaWaitsForDx = nullptr; m_dxWaitsForCuda = nullptr;
	m_sharedBuffer.Reset(); m_dxToCudaFence.Reset(); m_cudaToDxFence.Reset();
	m_bufferSize = 0; m_nAllocationSize = 0; m_dxToCudaValue = 0; m_cudaToDxValue = 0;
	m_pendingCudaValue = 0; m_lastDxWaitedCudaValue = 0; m_initialized = false;
}

bool CDirectXCudaInterop::SetExecutionMode(EExecutionMode mode, ID3D12CommandQueue* directQueue)
{
	if (mode == m_executionMode) return true;
	if (!m_initialized || !directQueue)
	{
		m_lastError = "SetExecutionMode requires initialized interop and a DirectX queue.";
		return false;
	}
	// CUDA -> Compute Shader 전환 시 마지막 CUDA 쓰기가 끝난 뒤 모드를 바꾼다.
	// 이 Wait 뒤에 같은 큐로 제출되는 Compute Shader는 공유 버퍼를 안전하게 쓸 수 있다.
	if (m_executionMode == EExecutionMode::CudaShared &&
		mode == EExecutionMode::ComputeShaderOnly &&
		m_pendingCudaValue > m_lastDxWaitedCudaValue && !WaitForCuda(directQueue))
		return false;
	m_executionMode = mode;
	return true;
}

bool CDirectXCudaInterop::CreateSharedBuffer(ID3D12Device* device, size_t bufferSize)
{
	D3D12_HEAP_PROPERTIES heap = {}; heap.Type = D3D12_HEAP_TYPE_DEFAULT;
	heap.CreationNodeMask = 1; heap.VisibleNodeMask = 1;
	D3D12_RESOURCE_DESC desc = {}; desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = bufferSize; desc.Height = 1; desc.DepthOrArraySize = 1; desc.MipLevels = 1;
	desc.SampleDesc.Count = 1; desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	m_nAllocationSize = device->GetResourceAllocationInfo(0, 1, &desc).SizeInBytes;
	if (!SetHRESULTError("CreateCommittedResource(shared buffer)", device->CreateCommittedResource(
		&heap, D3D12_HEAP_FLAG_SHARED, &desc, D3D12_RESOURCE_STATE_COMMON,
		nullptr, IID_PPV_ARGS(&m_sharedBuffer)))) return false;
	m_bufferSize = bufferSize;
	return true;
}

bool CDirectXCudaInterop::CreateSharedFences(ID3D12Device* device)
{
	if (!SetHRESULTError("CreateFence(DX to CUDA)", device->CreateFence(
		0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&m_dxToCudaFence)))) return false;
	return SetHRESULTError("CreateFence(CUDA to DX)", device->CreateFence(
		0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&m_cudaToDxFence)));
}

void CDirectXCudaInterop::ImportDXBufferToCuda(ID3D12Device* device)
{
	// DX12 공유 리소스 핸들을 CUDA 외부 메모리로 가져온다.
	HANDLE handle = CreateExportHandle(device, m_sharedBuffer.Get());
	if (!handle)
	{
		std::cerr << "CreateSharedHandle(shared buffer) failed." << std::endl;
		std::exit(EXIT_FAILURE);
	}
	cudaExternalMemoryHandleDesc memoryDesc = {};
	memoryDesc.type = cudaExternalMemoryHandleTypeD3D12Resource;
	memoryDesc.handle.win32.handle = handle; memoryDesc.size = m_nAllocationSize;
	memoryDesc.flags = cudaExternalMemoryDedicated;
	checkCudaErrors(cudaImportExternalMemory(&m_cudaExternalMemory, &memoryDesc));
	CloseHandle(handle);
	cudaExternalMemoryBufferDesc bufferDesc = {}; bufferDesc.size = m_bufferSize;
	checkCudaErrors(cudaExternalMemoryGetMappedBuffer(
		&m_cudaBuffer, m_cudaExternalMemory, &bufferDesc));
}

void CDirectXCudaInterop::ImportDXFencesToCuda(ID3D12Device* device)
{
	// DX->CUDA와 CUDA->DX 단방향 Fence를 각각 CUDA 세마포어로 가져온다.
	HANDLE dxHandle = CreateExportHandle(device, m_dxToCudaFence.Get());
	HANDLE cudaHandle = CreateExportHandle(device, m_cudaToDxFence.Get());
	if (!dxHandle || !cudaHandle)
	{
		if (dxHandle) CloseHandle(dxHandle); if (cudaHandle) CloseHandle(cudaHandle);
		std::cerr << "CreateSharedHandle(shared fence) failed." << std::endl;
		std::exit(EXIT_FAILURE);
	}
	cudaExternalSemaphoreHandleDesc desc = {};
	desc.type = cudaExternalSemaphoreHandleTypeD3D12Fence; desc.handle.win32.handle = dxHandle;
	checkCudaErrors(cudaImportExternalSemaphore(&m_cudaWaitsForDx, &desc));
	CloseHandle(dxHandle);
	desc = {}; desc.type = cudaExternalSemaphoreHandleTypeD3D12Fence;
	desc.handle.win32.handle = cudaHandle;
	checkCudaErrors(cudaImportExternalSemaphore(&m_dxWaitsForCuda, &desc));
	CloseHandle(cudaHandle);
}

bool CDirectXCudaInterop::ExecuteCuda(ID3D12CommandQueue* queue, const KernelParameters& parameters)
{
	if (m_executionMode == EExecutionMode::ComputeShaderOnly) return true;
	if (!m_initialized || !queue)
	{ m_lastError = "ExecuteCuda called before Initialize or without a queue."; return false; }
	UINT64 ready = 0;
	if (!SignalDirectXComplete(queue, &ready) || !WaitCudaForDirectX(ready)) return false;
	checkCudaErrors(LaunchDirectXCudaInteropKernel(
		m_cudaBuffer, m_bufferSize / sizeof(float), parameters.m_fElapsedTime,
		parameters.m_fValueScale, m_cudaStream));
	return SignalCudaComplete(&m_pendingCudaValue);
}

bool CDirectXCudaInterop::WaitForCuda(ID3D12CommandQueue* queue)
{
	if (m_executionMode == EExecutionMode::ComputeShaderOnly) return true;
	if (!m_pendingCudaValue) { m_lastError = "WaitForCuda called before CUDA submission."; return false; }
	if (m_pendingCudaValue <= m_lastDxWaitedCudaValue) return true;
	if (!WaitDirectXForCuda(queue, m_pendingCudaValue)) return false;
	m_lastDxWaitedCudaValue = m_pendingCudaValue;
	return true;
}

bool CDirectXCudaInterop::SignalDirectXComplete(ID3D12CommandQueue* queue, UINT64* outValue)
{
	if (!queue || !m_dxToCudaFence) return false;
	const UINT64 value = ++m_dxToCudaValue;
	if (!SetHRESULTError("CommandQueue::Signal(DX to CUDA)", queue->Signal(m_dxToCudaFence.Get(), value))) return false;
	if (outValue) *outValue = value; return true;
}

bool CDirectXCudaInterop::WaitCudaForDirectX(UINT64 value)
{
	cudaExternalSemaphoreWaitParams params = {}; params.params.fence.value = value;
	checkCudaErrors(cudaWaitExternalSemaphoresAsync(
		&m_cudaWaitsForDx, &params, 1, m_cudaStream));
	return true;
}

bool CDirectXCudaInterop::SignalCudaComplete(UINT64* outValue)
{
	const UINT64 value = ++m_cudaToDxValue;
	cudaExternalSemaphoreSignalParams params = {}; params.params.fence.value = value;
	checkCudaErrors(cudaSignalExternalSemaphoresAsync(
		&m_dxWaitsForCuda, &params, 1, m_cudaStream));
	if (outValue) *outValue = value; return true;
}

bool CDirectXCudaInterop::WaitDirectXForCuda(ID3D12CommandQueue* queue, UINT64 value)
{
	if (!queue || !m_cudaToDxFence) return false;
	return SetHRESULTError("CommandQueue::Wait(CUDA to DX)", queue->Wait(m_cudaToDxFence.Get(), value));
}

bool CDirectXCudaInterop::SetHRESULTError(const char* operation, HRESULT result)
{
	if (SUCCEEDED(result)) return true;
	std::ostringstream stream; stream << operation << " failed (HRESULT 0x"
		<< std::hex << static_cast<unsigned long>(result) << ").";
	m_lastError = stream.str(); return false;
}
