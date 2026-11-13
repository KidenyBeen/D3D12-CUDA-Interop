#include "DirectXCudaInterop.h"
#include "helper_cuda.h"

#include <iostream>
#include <sstream>

extern "C" cudaError_t LaunchDirectXCudaInteropKernel(
	void*, size_t, float, float, cudaStream_t);
extern "C" cudaError_t LaunchDirectXCudaInteropTextureKernel(
	cudaSurfaceObject_t, cudaSurfaceObject_t, UINT, UINT, int, float, cudaStream_t);
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

//현재 지원중인 Init 함수로
// DX12에서 공유된 Texture를 CUDA에서 읽고 쓰기를 위해 초기화하고 있다.
bool CDirectXCudaInterop::InitializeTexture(ID3D12Device* device, ID3D12Resource* sharedTexture,
	DXGI_FORMAT format, UINT width, UINT height, int cudaDeviceOrdinal)
{
	Shutdown();
	if (!device || !sharedTexture || width == 0 || height == 0)
	{
		m_lastError = "InitializeTexture requires a device, shared texture, width and height.";
		return false;
	}
	if (format != DXGI_FORMAT_R16G16B16A16_FLOAT)
	{
		m_lastError = "CUDA texture interop currently supports R16G16B16A16_FLOAT only.";
		return false;
	}

	m_sharedTexture = sharedTexture;
	m_nTextureWidth = width;
	m_nTextureHeight = height;
	m_dxgiTextureFormat = format;
	m_bTextureInterop = true;
	D3D12_RESOURCE_DESC resourceDesc = sharedTexture->GetDesc();
	m_nAllocationSize = device->GetResourceAllocationInfo(0, 1, &resourceDesc).SizeInBytes;

	if (!CreateSharedFences(device))
		return false;

	SetCudaDXDevice(cudaDeviceOrdinal);
	checkCudaErrors(cudaStreamCreateWithFlags(&m_cudaStream, cudaStreamNonBlocking));
	ImportDXTextureToCuda(device);
	ImportDXFencesToCuda(device);
	m_initialized = true;
	return true;
}

//기본 초기화 함수 현재는 사용하지 않는다.
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
	if (m_cudaTempSurface) checkCudaErrors(cudaDestroySurfaceObject(m_cudaTempSurface));
	if (m_cudaSharedSurface) checkCudaErrors(cudaDestroySurfaceObject(m_cudaSharedSurface));
	if (m_cudaTempArray) checkCudaErrors(cudaFreeArray(m_cudaTempArray));
	if (m_dxWaitsForCuda) checkCudaErrors(cudaDestroyExternalSemaphore(m_dxWaitsForCuda));
	if (m_cudaWaitsForDx) checkCudaErrors(cudaDestroyExternalSemaphore(m_cudaWaitsForDx));
	if (m_cudaExternalMemory) checkCudaErrors(cudaDestroyExternalMemory(m_cudaExternalMemory));
	if (m_cudaStream) checkCudaErrors(cudaStreamDestroy(m_cudaStream));
	m_cudaStream = nullptr; m_cudaBuffer = nullptr; m_cudaExternalMemory = nullptr;
	m_cudaSharedMipmappedArray = nullptr; m_cudaSharedArray = nullptr; m_cudaTempArray = nullptr;
	m_cudaSharedSurface = 0; m_cudaTempSurface = 0;
	m_cudaWaitsForDx = nullptr; m_dxWaitsForCuda = nullptr;
	m_sharedBuffer.Reset(); m_sharedTexture.Reset(); m_dxToCudaFence.Reset(); m_cudaToDxFence.Reset();
	m_bufferSize = 0; m_nAllocationSize = 0; m_dxToCudaValue = 0; m_cudaToDxValue = 0;
	m_nTextureWidth = 0; m_nTextureHeight = 0; m_dxgiTextureFormat = DXGI_FORMAT_UNKNOWN;
	m_bTextureInterop = false;
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
	//둘 중 하나라도 실패하면 false를 반환.
	if (!SetHRESULTError("CreateFence(DX to CUDA)", device->CreateFence(
		0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&m_dxToCudaFence))))
	{
		return false;
	}

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

void CDirectXCudaInterop::ImportDXTextureToCuda(ID3D12Device* device)
{
	// DX12 공유 Texture를 CUDA surface로 매핑한다.
	HANDLE hSharedHandle = CreateExportHandle(device, m_sharedTexture.Get());
	if (!hSharedHandle)
	{
		std::cerr << "CreateSharedHandle(shared texture) failed." << std::endl;
		std::exit(EXIT_FAILURE);
	}

	cudaExternalMemoryHandleDesc memoryDesc = {};
	memoryDesc.type = cudaExternalMemoryHandleTypeD3D12Resource;
	memoryDesc.handle.win32.handle = hSharedHandle;
	memoryDesc.size = m_nAllocationSize;
	memoryDesc.flags = cudaExternalMemoryDedicated;
	checkCudaErrors(cudaImportExternalMemory(&m_cudaExternalMemory, &memoryDesc));
	CloseHandle(hSharedHandle);

	// 현재 후처리 공유 포맷은 R16G16B16A16_FLOAT를 기준으로 한다.
	cudaChannelFormatDesc channelDesc = {};
	channelDesc.x = 16; channelDesc.y = 16; channelDesc.z = 16; channelDesc.w = 16;
	channelDesc.f = cudaChannelFormatKindFloat;

	cudaExternalMemoryMipmappedArrayDesc arrayDesc = {};
	arrayDesc.formatDesc = channelDesc;
	// make_cudaExtent 헬퍼는 CUDA 헤더 구성에 따라 IntelliSense가 찾지 못할 수 있다.
	// 2D CUDA Array이므로 cudaExtent를 직접 채우고 depth는 0으로 둔다.
	cudaExtent textureExtent = {};
	textureExtent.width = static_cast<size_t>(m_nTextureWidth);
	textureExtent.height = static_cast<size_t>(m_nTextureHeight);
	textureExtent.depth = 0;
	arrayDesc.extent = textureExtent;
	arrayDesc.numLevels = 1;
	arrayDesc.flags = cudaArraySurfaceLoadStore;
	checkCudaErrors(cudaExternalMemoryGetMappedMipmappedArray(
		&m_cudaSharedMipmappedArray, m_cudaExternalMemory, &arrayDesc));
	checkCudaErrors(cudaGetMipmappedArrayLevel(&m_cudaSharedArray, m_cudaSharedMipmappedArray, 0));

	// 블러 같은 다단계 후처리에서 사용할 CUDA 전용 중간 surface이다.
	checkCudaErrors(cudaMallocArray(&m_cudaTempArray, &channelDesc,
		m_nTextureWidth, m_nTextureHeight, cudaArraySurfaceLoadStore));

	cudaResourceDesc resourceDesc = {};
	resourceDesc.resType = cudaResourceTypeArray;
	resourceDesc.res.array.array = m_cudaSharedArray;
	checkCudaErrors(cudaCreateSurfaceObject(&m_cudaSharedSurface, &resourceDesc));
	resourceDesc.res.array.array = m_cudaTempArray;
	checkCudaErrors(cudaCreateSurfaceObject(&m_cudaTempSurface, &resourceDesc));
}

void CDirectXCudaInterop::ImportDXFencesToCuda(ID3D12Device* device)
{
	// DX->CUDA와 CUDA->DX 단방향 Fence를 각각 CUDA 세마포어로 가져온다.
	HANDLE dxHandle = CreateExportHandle(device, m_dxToCudaFence.Get());
	HANDLE cudaHandle = CreateExportHandle(device, m_cudaToDxFence.Get());
	if (!dxHandle || !cudaHandle)
	{
		if (dxHandle)
		{
			CloseHandle(dxHandle)
		}; 
		if (cudaHandle)
		{
			CloseHandle(cudaHandle);
		}
		std::cerr << "CreateSharedHandle(shared fence) failed." << std::endl;
		std::exit(EXIT_FAILURE);
	}
	cudaExternalSemaphoreHandleDesc desc = {};
	desc.type = cudaExternalSemaphoreHandleTypeD3D12Fence; //DX12 Fence를 CUDA 세마포어로 가져온다.
	desc.handle.win32.handle = dxHandle;
	checkCudaErrors(cudaImportExternalSemaphore(&m_cudaWaitsForDx, &desc));
	CloseHandle(dxHandle);
	desc = {}; 
	desc.type = cudaExternalSemaphoreHandleTypeD3D12Fence;
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
	if (m_bTextureInterop)
	{
		checkCudaErrors(LaunchDirectXCudaInteropTextureKernel(m_cudaSharedSurface,
			m_cudaTempSurface, m_nTextureWidth, m_nTextureHeight,
			parameters.m_nBlurRadius, parameters.m_fBlurSigma, m_cudaStream));
	}
	else
	{
		checkCudaErrors(LaunchDirectXCudaInteropKernel(
			m_cudaBuffer, m_bufferSize / sizeof(float), parameters.m_fElapsedTime,
			parameters.m_fValueScale, m_cudaStream));
	}
	return SignalCudaComplete(&m_pendingCudaValue);
}

// CUDA에서 작성한 데이터를 DirectX가 읽기 전에 호출하는 함수이다.
bool CDirectXCudaInterop::WaitForCuda(ID3D12CommandQueue* queue)
{
	if (m_executionMode == EExecutionMode::ComputeShaderOnly) return true;
	if (!m_pendingCudaValue) { m_lastError = "WaitForCuda called before CUDA submission."; return false; }
	if (m_pendingCudaValue <= m_lastDxWaitedCudaValue) return true;
	if (!WaitDirectXForCuda(queue, m_pendingCudaValue)) return false;
	m_lastDxWaitedCudaValue = m_pendingCudaValue;
	return true;
}

// CUDA에서 DirectX로 넘어가는 신호를 보내는 함수이다. DirectX에서 CUDA가 끝났음을 알리는 용도이다.
bool CDirectXCudaInterop::SignalDirectXComplete(ID3D12CommandQueue* queue, UINT64* outValue)
{
	if (!queue || !m_dxToCudaFence)
	{
		return false;
	}
	const UINT64 value = ++m_dxToCudaValue;
	if (!SetHRESULTError("CommandQueue::Signal(DX to CUDA)", queue->Signal(m_dxToCudaFence.Get(), value)))
	{
		return false;
	}
	if (outValue) *outValue = value; return true;
}

// CUDA 스트림에 DirectX 완료 Fence 값에 대한 대기를 추가한다.
// 같은 스트림의 후속 작업은 해당 값에 도달한 뒤 실행된다.
bool CDirectXCudaInterop::WaitCudaForDirectX(UINT64 value)
{
	cudaExternalSemaphoreWaitParams params = {}; params.params.fence.value = value;
	checkCudaErrors(cudaWaitExternalSemaphoresAsync(
		&m_cudaWaitsForDx, &params, 1, m_cudaStream));
	return true;
}

//	CUDA에서 DirectX로 넘어가는 신호를 보내는 함수이다. DirectX에서 CUDA가 끝났음을 알리는 용도이다.
bool CDirectXCudaInterop::SignalCudaComplete(UINT64* outValue)
{
	const UINT64 value = ++m_cudaToDxValue;
	cudaExternalSemaphoreSignalParams params = {}; params.params.fence.value = value;
	checkCudaErrors(cudaSignalExternalSemaphoresAsync(
		&m_dxWaitsForCuda, &params, 1, m_cudaStream));
	if (outValue) *outValue = value; return true;
}

// DirectX에서 CUDA로 넘어가는 신호를 기다리는 함수이다. DirectX에서 CUDA가 끝났음을 알리는 용도이다.
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
