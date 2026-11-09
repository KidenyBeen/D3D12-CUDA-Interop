#include "stdafx.h"
#include "Scene.h"

CDescriptorHeap* CScene::m_pDescriptorHeap = NULL;

CDescriptorHeap::CDescriptorHeap()
{
	m_d3dSrvCPUDescriptorStartHandle.ptr = NULL;
	m_d3dSrvGPUDescriptorStartHandle.ptr = NULL;
	m_d3dUavCPUDescriptorStartHandle.ptr = NULL;
	m_d3dUavGPUDescriptorStartHandle.ptr = NULL;
}

CDescriptorHeap::~CDescriptorHeap()
{
	if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCbvSrvDescriptorHeap->Release();
}


CScene::CScene(HWND hWnd, CGameTimer* timer)
{
	m_hWnd = hWnd;
	m_pGameTimer = timer;
	
}

CScene::~CScene()
{

}

void CScene::CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews , int nUnorderedAccessViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews + nUnorderedAccessViews; //CBVs + SRVs + UAVs
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	
	HRESULT hr = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)m_pDescriptorHeap-> m_pd3dCbvSrvDescriptorHeap.GetAddressOf());

	if (FAILED(hr)) {
		_com_error err(hr);
		OutputDebugStringW(L"[CreateDescriptorHeap] FAILED: ");
		OutputDebugStringW(err.ErrorMessage());   // ← 여기서 사람이 읽을 수 있는 메시지 출력
		OutputDebugStringW(L"\n");
		return;
	}

	m_pDescriptorHeap->m_d3dCbvCPUDescriptorStartHandle = m_pDescriptorHeap->m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pDescriptorHeap->m_d3dCbvGPUDescriptorStartHandle = m_pDescriptorHeap->m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_pDescriptorHeap->m_d3dSrvCPUDescriptorStartHandle.ptr = m_pDescriptorHeap->m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_pDescriptorHeap->m_d3dSrvGPUDescriptorStartHandle.ptr = m_pDescriptorHeap->m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_pDescriptorHeap->m_d3dUavCPUDescriptorStartHandle.ptr = m_pDescriptorHeap->m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * (nConstantBufferViews +nShaderResourceViews));
	m_pDescriptorHeap->m_d3dUavGPUDescriptorStartHandle.ptr = m_pDescriptorHeap->m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * (nConstantBufferViews + nShaderResourceViews));

	m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle = m_pDescriptorHeap->m_d3dCbvCPUDescriptorStartHandle;
	m_pDescriptorHeap->m_d3dCbvGPUDescriptorNextHandle = m_pDescriptorHeap->m_d3dCbvGPUDescriptorStartHandle;
	m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle = m_pDescriptorHeap->m_d3dSrvCPUDescriptorStartHandle;
	m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle = m_pDescriptorHeap->m_d3dSrvGPUDescriptorStartHandle;
	m_pDescriptorHeap->m_d3dUavCPUDescriptorNextHandle = m_pDescriptorHeap->m_d3dUavCPUDescriptorStartHandle;
	m_pDescriptorHeap->m_d3dUavGPUDescriptorNextHandle = m_pDescriptorHeap->m_d3dUavGPUDescriptorStartHandle;
}

void CScene::CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride)
{
	/*
	* 아 왜 이렇게 작성했지;;; CPU 핸들 값이 변하지 않는 이상한걸로 계속 하나만 채우고있었네;;
	*/
	/*D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j = 0; j < nConstantBufferViews; j++)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		D3D12_CPU_DESCRIPTOR_HANDLE d3dCbvCPUDescriptorHandle;
		d3dCbvCPUDescriptorHandle.ptr = m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle);
	}*/

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVdesc = {};
	d3dCBVdesc.SizeInBytes = nStride;

	for (int j = 0; j < nConstantBufferViews; ++j)
	{
		d3dCBVdesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		pd3dDevice->CreateConstantBufferView(&d3dCBVdesc,
			m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle);

		m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		m_pDescriptorHeap->m_d3dCbvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateConstantBufferView(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dConstantBuffer, UINT nStride)
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	d3dCBVDesc.BufferLocation = pd3dConstantBuffer->GetGPUVirtualAddress();
	pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle);
	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = m_pDescriptorHeap->m_d3dCbvGPUDescriptorNextHandle;
	m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle.ptr = m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
	m_pDescriptorHeap->m_d3dCbvGPUDescriptorNextHandle.ptr = m_pDescriptorHeap->m_d3dCbvGPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;

	return(d3dCbvGPUDescriptorHandle);
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateConstantBufferView(ID3D12Device* pd3dDevice, D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress, UINT nStride)
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress;
	pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle);
	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = m_pDescriptorHeap->m_d3dCbvGPUDescriptorNextHandle;
	m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle.ptr = m_pDescriptorHeap->m_d3dCbvCPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
	m_pDescriptorHeap->m_d3dCbvGPUDescriptorNextHandle.ptr = m_pDescriptorHeap->m_d3dCbvGPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;

	return(d3dCbvGPUDescriptorHandle);
}

/*
* TextureResourceView를 DescriptorHeap에 생성과 함께 저장해주는 함수로
* Texture 정보를 읽어온 CTexture를 받아오며
* 디스크립터 힙의 몇 번째 부터 시작할껀지 지정 3 번쨰 파라미터 값이다
* 현재 루트 파라미터의 몇번째에 등록해서 사용할지가 4번째 파라미터에 있다.
*/
void CScene::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{
	m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);
	m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

	int nTextures = pTexture->GetTextures();
	for (int i = 0; i < nTextures; i++)
	{
		ComPtr<ID3D12Resource> pShaderResource = pTexture->GetResource(i);
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(i);
		pd3dDevice->CreateShaderResourceView(pShaderResource.Get(), &d3dShaderResourceViewDesc, m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle);
		m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		pTexture->SetGpuDescriptorHandle(i, m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle);
		m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	}
	int nRootParameters = pTexture->GetRootParameters();
	for (int i = 0; i < nRootParameters; i++)
	{
		pTexture->SetRootParameterIndex(i, nRootParameterStartIndex + i);
	}
}

void CScene::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, DXGI_FORMAT* pdxgiFormat,  UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{
	m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);
	m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

	int nTextures = pTexture->GetTextures();
	for (int i = 0; i < nTextures; i++)
	{
		ComPtr<ID3D12Resource> pShaderResource = pTexture->GetResource(i);
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(i);
		d3dShaderResourceViewDesc.Format = pdxgiFormat[i];
		pd3dDevice->CreateShaderResourceView(pShaderResource.Get(), &d3dShaderResourceViewDesc, m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle);
		m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		pTexture->SetGpuDescriptorHandle(i, m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle);
		m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	}
	int nRootParameters = pTexture->GetRootParameters();
	for (int i = 0; i < nRootParameters; i++)
	{
		pTexture->SetRootParameterIndex(i, nRootParameterStartIndex + i);
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateStructureBufferSRV(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dResource,UINT nElementCount, UINT nStructureStride)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN; //StructuredBuffer 필수
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = nElementCount;
	srvDesc.Buffer.StructureByteStride = nStructureStride;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle =
		m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle;

	pd3dDevice->CreateShaderResourceView(
		pd3dResource,
		&srvDesc,
		m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle
	);

	m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle.ptr +=
		::gnCbvSrvDescriptorIncrementSize;

	m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle.ptr +=
		::gnCbvSrvDescriptorIncrementSize;

	return gpuHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateShaderResourceView(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dResource, DXGI_FORMAT dxgiSrvFormat)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	d3dShaderResourceViewDesc.Format = dxgiSrvFormat;
	d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	d3dShaderResourceViewDesc.Texture2D.MipLevels = 1;
	d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
	d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle;
	pd3dDevice->CreateShaderResourceView(pd3dResource, &d3dShaderResourceViewDesc, m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle);
	m_pDescriptorHeap->m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	m_pDescriptorHeap->m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

	return(d3dSrvGPUDescriptorHandle);
}

void CScene::CreateUnorderedAccessView(ID3D12Device* pd3dDevice, CTexture* pTexture, DXGI_FORMAT* pdxgiFormat , UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{
	// UAV Heap 시작 위치 이동
	m_pDescriptorHeap->m_d3dUavCPUDescriptorNextHandle.ptr +=
		(::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

	m_pDescriptorHeap->m_d3dUavGPUDescriptorNextHandle.ptr +=
		(::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

	int nTextures = pTexture->GetTextures();

	for (int i = 0; i < nTextures; i++)
	{
		ComPtr<ID3D12Resource> pUavResource = pTexture->GetResource(i);

		// UAV Desc (Texture2D 기준)
		D3D12_UNORDERED_ACCESS_VIEW_DESC d3dUnorderedAccessViewDesc = {};
		d3dUnorderedAccessViewDesc.Format = pdxgiFormat[i];
		d3dUnorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		d3dUnorderedAccessViewDesc.Texture2D.MipSlice = 0;
		d3dUnorderedAccessViewDesc.Texture2D.PlaneSlice = 0;

		pd3dDevice->CreateUnorderedAccessView(
			pUavResource.Get(),
			nullptr, // Texture UAV는 Counter 없음
			&d3dUnorderedAccessViewDesc,
			m_pDescriptorHeap->m_d3dUavCPUDescriptorNextHandle
		);

		// GPU Handle 저장
		pTexture->SetUavGpuDescriptorHandle(i,
			m_pDescriptorHeap->m_d3dUavGPUDescriptorNextHandle);

		// 다음 UAV 슬롯으로 이동
		m_pDescriptorHeap->m_d3dUavCPUDescriptorNextHandle.ptr +=
			::gnCbvSrvDescriptorIncrementSize;

		m_pDescriptorHeap->m_d3dUavGPUDescriptorNextHandle.ptr +=
			::gnCbvSrvDescriptorIncrementSize;
	}

	// Root Parameter 인덱스 설정 (u0부터)
	int nRootParameters = pTexture->GetRootParameters();
	for (int i = 0; i < nRootParameters; i++)
	{
		pTexture->SetComputeRootParameterIndex(i, nRootParameterStartIndex + i);
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateStructureBufferUAV(ID3D12Device* pd3dDevice, ID3D12Resource* pd3dResource, UINT nElementCount, UINT nStructureStride)
{
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;           // StructuredBuffer 필수
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = nElementCount;
	uavDesc.Buffer.StructureByteStride = nStructureStride;
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle =
		m_pDescriptorHeap->m_d3dUavGPUDescriptorNextHandle;

	pd3dDevice->CreateUnorderedAccessView(
		pd3dResource,
		nullptr,
		&uavDesc,
		m_pDescriptorHeap->m_d3dUavCPUDescriptorNextHandle
	);

	m_pDescriptorHeap->m_d3dUavCPUDescriptorNextHandle.ptr +=
		::gnCbvSrvDescriptorIncrementSize;

	m_pDescriptorHeap->m_d3dUavGPUDescriptorNextHandle.ptr +=
		::gnCbvSrvDescriptorIncrementSize;

	return gpuHandle;
}




void CScene::CreateShaderResourceViewRtvs(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nRenderTargets, DXGI_FORMAT* pdxgiFormats, D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle)
{
	//Normal 값을 저장할 G-buffer 하나를 더 만든다
	//PostPorcessing 과 HDRI 결과를 저장할 RTV와  Normal 값을 저장할 G-buffer를 하나 더 만든다

	/*
	* TODO: 리팩토링할떄 저 nRenderTargets 값이랑 Compute를 위한것과 Graphics를 위한거 2개로 쪼개는 과정 필요
	*/

	//Texture를 선언 및 초기화
	m_pRenderTexture = new CTexture(nRenderTargets, RESOURCE_TEXTURE2D, 0, 1);
	m_pComputeRenderTexture = new CTexture( 4, RESOURCE_TEXTURE2D, 0, 1);

	//HDR 파이프라인으로 바꾸기 위해서
	D3D12_CLEAR_VALUE d3dClearValue = { DXGI_FORMAT_R16G16B16A16_FLOAT, { 0.0f, 0.0f, 0.0f, 1.0f } };
	//RenderTarget 갯수만큼 만들고
	for (UINT i = 0; i < nRenderTargets; i++)
	{
		d3dClearValue.Format = pdxgiFormats[i];
		m_pRenderTexture->CreateTexture(pd3dDevice, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, pdxgiFormats[i], D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS , D3D12_RESOURCE_STATE_COMMON, &d3dClearValue, RESOURCE_TEXTURE2D, i);
	}

	DXGI_FORMAT dxgiFormats[4] = { DXGI_FORMAT_R16G16B16A16_FLOAT  , DXGI_FORMAT_R16G16B16A16_FLOAT  ,
		DXGI_FORMAT_R16G16B16A16_FLOAT , DXGI_FORMAT_R16G16B16A16_FLOAT };

	for (UINT i = 0; i < 4; i++)
	{
		d3dClearValue.Format = dxgiFormats[i];
		m_pComputeRenderTexture->CreateTexture(pd3dDevice, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, dxgiFormats[i], D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON, &d3dClearValue, RESOURCE_TEXTURE2D, i);
	}


	//다른 Texture리소스도 만든다.
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	
	DXGI_FORMAT uavFormats[1] =
	{
		DXGI_FORMAT_R16G16B16A16_FLOAT, // HDR Color
	};

	DXGI_FORMAT uavFormats2[4] =
	{
		DXGI_FORMAT_R16G16B16A16_FLOAT, // HDR Color
		DXGI_FORMAT_R16G16B16A16_FLOAT, // HDR Color
		DXGI_FORMAT_R16G16B16A16_FLOAT, // HDR Color
		DXGI_FORMAT_R16G16B16A16_FLOAT, // HDR Color
		
	};

	//UAV 만들어주고
	CreateUnorderedAccessView( //Render에서 결과를 담아올 RTV를 UAV로 만드는 경우
		pd3dDevice,
		m_pRenderTexture,   
		uavFormats,
		0,                  // UAV Heap index
		4                   // Compute RootParameter 시작 인덱스 (u0)
	);

	CreateUnorderedAccessView( //CS에서 사용할 목적으로 생성된 G-Buffer들과 블러결과를 swap 해줄 2개의 Texture UAV
		pd3dDevice,
		m_pComputeRenderTexture,
		uavFormats2,
		0,                  // UAV Heap index
		4                   // Compute RootParameter 시작 인덱스 (u0)
	);
	//SRV도 만들고
	CScene::CreateShaderResourceViews(pd3dDevice, m_pRenderTexture, 0, 19);
	CScene::CreateShaderResourceViews(pd3dDevice, m_pComputeRenderTexture, 0, 3); //Compute에 연결할꺼니까

	D3D12_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc;
	d3dRenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3dRenderTargetViewDesc.Texture2D.PlaneSlice = 0;


	m_pd3dRtvCPUDescriptorHandles = new D3D12_CPU_DESCRIPTOR_HANDLE[nRenderTargets];
	m_pd3dG_bufferRtvCPUDescriptorHandles = new D3D12_CPU_DESCRIPTOR_HANDLE[2];
	//RTV 만들고
	for (UINT i = 0; i < nRenderTargets; i++)
	{
		//PostProcessing에 전반적인 기본 Texture
		d3dRenderTargetViewDesc.Format = pdxgiFormats[i];
		ComPtr<ID3D12Resource> pd3dTextureResource = m_pRenderTexture->GetResource(i);
		pd3dDevice->CreateRenderTargetView(pd3dTextureResource.Get(), &d3dRenderTargetViewDesc, d3dRtvCPUDescriptorHandle);
		m_pd3dRtvCPUDescriptorHandles[i] = d3dRtvCPUDescriptorHandle; //RTV랑 DSV는 CPU Address가 필요하다
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}

	for (UINT i = 0; i < 2; i++)
	{
		//PostProcessing에 전반적인 기본 Texture
		d3dRenderTargetViewDesc.Format = dxgiFormats[i];

		//SSAO에 사용할 G_Buffer를 저장할 RenderTexture
		ComPtr<ID3D12Resource> pd3dTextureResource = m_pComputeRenderTexture->GetResource(i);
		pd3dDevice->CreateRenderTargetView(pd3dTextureResource.Get(), &d3dRenderTargetViewDesc, d3dRtvCPUDescriptorHandle);
		m_pd3dG_bufferRtvCPUDescriptorHandles[i] = d3dRtvCPUDescriptorHandle; //RTV랑 DSV는 CPU Address가 필요하다
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}
}

void CScene::CreateShaderResourceViewDsvs(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nDepthTextures, D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle, ID3D12Resource* DefaultDepthStncilBuffer)
{
	//Texture를 선언 및 초기화
	m_pShadowDepthTexture = new CTexture(nDepthTextures, RESOURCE_TEXTURE2D, 0, 1);

	D3D12_CLEAR_VALUE d3dClearValue = {};
	d3dClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;
	
	DXGI_FORMAT dxgiFormatResource = DXGI_FORMAT_R32_TYPELESS; // Resource를 위한것
	DXGI_FORMAT* pdxgiFormatSRV = new DXGI_FORMAT[nDepthTextures]; // SRV를 위한것

	//nDepthTextures 갯수만큼 만들고
	for (UINT i = 0; i < nDepthTextures; i++)
	{
		pdxgiFormatSRV[i] = DXGI_FORMAT_R32_FLOAT;
		if (i == 0)
		{
			m_pShadowDepthTexture->CreateTexture(pd3dDevice, _DEPTH_BUFFER_WIDTH, _DEPTH_BUFFER_HEIGHT, dxgiFormatResource, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, &d3dClearValue, RESOURCE_TEXTURE2D, i);
		}
		else
		{
			m_pShadowDepthTexture->CreateTexture(pd3dDevice, _SPOT_DEPTH_BUFFER_WIDTH, _SPOT_DEPTH_BUFFER_HEIGHT, dxgiFormatResource, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, &d3dClearValue, RESOURCE_TEXTURE2D, i);
		}
	}

	//다른 Texture리소스도 만든다.
	//CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//SRV도 만들고
	CScene::CreateShaderResourceViews(pd3dDevice, m_pShadowDepthTexture, pdxgiFormatSRV,  0, 20);

	//기본으로 쓰이는 깊이맵
	DXGI_FORMAT dxgiFormatSRV = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	m_pd3dDefaultDepthMapSrvDescrptorHandles = CScene::CreateShaderResourceView(pd3dDevice, DefaultDepthStncilBuffer,  dxgiFormatSRV);
	m_pDefaultDepthStancilResource = DefaultDepthStncilBuffer;

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;
	d3dDepthStencilViewDesc.Texture2D.MipSlice = 0;

	m_pd3dSahdowMapDsvCPUDescriptorHandles = new D3D12_CPU_DESCRIPTOR_HANDLE[nDepthTextures];

	//DSV 만들고

	ComPtr<ID3D12Resource> pd3dTextureResource;

	for (UINT i = 0; i < nDepthTextures; i++)
	{
		pd3dTextureResource = m_pShadowDepthTexture->GetResource(i);
		pd3dDevice->CreateDepthStencilView(pd3dTextureResource.Get(), &d3dDepthStencilViewDesc, d3dDsvCPUDescriptorHandle);
		m_pd3dSahdowMapDsvCPUDescriptorHandles[i] = d3dDsvCPUDescriptorHandle; //RTV랑 DSV는 CPU Address가 필요하다
		d3dDsvCPUDescriptorHandle.ptr += ::gnDsvDescriptorIncrementSize;
	}
	if (pdxgiFormatSRV)
	{
		delete[] pdxgiFormatSRV;
	}
}

void CScene::CreateShaderResourceViewAndUAVStructuredBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	//
	UINT ncbElementBytes = ((sizeof(ClusterCB) + 255) & ~255); //256의 배수
	m_cpcbCluster = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_cpcbCluster->Map(0, NULL, (void**)&m_pcbCluster);

	ncbElementBytes = ((sizeof(LightingData) + 255) & ~255); //256의 배수
	m_cpcbLightData = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_cpcbLightData->Map(0, NULL, (void**)&m_pcbLightData);

	//UINT clusterCount =
	//	m_ClusterDim.x * m_ClusterDim.y * m_ClusterDim.z;
	UINT clusterCount =
		m_ClusterDim.x * m_ClusterDim .y* m_ClusterDim.z;


	UINT bufferSize =
		sizeof(ClusterLightList) * clusterCount;

	m_cpsbClusterLightList =
		::CreateBufferResource(
			pd3dDevice,
			pd3dCommandList,
			nullptr,
			bufferSize,
			D3D12_HEAP_TYPE_DEFAULT,
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
			nullptr
		);

	// ClusterLightList UAV (CS에서 쓰기)
	m_hClusterLightListUav =
		CreateStructureBufferUAV(
			pd3dDevice,
			m_cpsbClusterLightList.Get(),
			clusterCount,
			sizeof(ClusterLightList)
		);

	// ClusterLightList SRV (PS에서 읽기)
	m_hClusterLightListSrv =
		CreateStructureBufferSRV(
			pd3dDevice,
			m_cpsbClusterLightList.Get(),
			clusterCount,
			sizeof(ClusterLightList)
		);
	

}


void CScene::BuildObjects(ID3D12Device* pd3dDevice)
{
	//CreateGraphicsRootSignature(pd3dDevice);	//루트 시그니쳐와 PSO를 여기서 초기화해주고 있다. 
	//CreateGraphicsPipelineState(pd3dDevice);	//이건 구지 BuildObjects에 넣어줄 필요까진 없고 설계를 이렇게 했을 뿐
	//초기화 함수를 따로 만들든해서 제어 가능하다.

	//다만 GameFramework에서 BuildObjects를 초기화 개념을 적용해서 사용하는 구조로 로직이 짜여있어서 여기다가 넣는게
	//편리하게 보인다.

}

void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dRootSignature = CGraphicsCommon::GetInstance().m_cpd3dMainSceneRootSignature;

	m_cpd3dDevice = pd3dDevice;
	m_cpd3dCommandList = pd3dCommandList;
}

void CScene::InitLights()
{
	
}

void CScene::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{

}

void CScene::ReleaseShaderVariables()
{
	if (m_cpd3dcbLights)
	{
		m_cpd3dcbLights->Unmap(0, NULL);
		m_cpd3dcbLights->Release();
	}

	if (m_cpd3dcbImGui_Info)
	{
		m_cpd3dcbImGui_Info->Unmap(0, NULL);
		m_cpd3dcbImGui_Info->Release();
	}
}

void CScene::ReleaseObjects()
{
	if (m_pd3dRootSignature) m_pd3dRootSignature.Reset();	//ComPtr을 썻기에 구지 필요하진 않은데

	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) delete m_ppObjects[j];
		delete[] m_ppObjects;
	}
	for (int i = 0; i < m_nShaders; i++)
	{
		m_pShaders[i].ReleaseShaderVariables();
		m_pShaders[i].ReleaseObjects();
	}
	if (m_pShaders)
	{
		delete[] m_pShaders;
	}
	if (m_pd3dPipelineState) m_pd3dPipelineState.Reset();	//항상 Release로 메모리 관리가 필요하단걸 잊지말자.


}

void CScene::ReleaseUploadBuffers()
{

	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j])
		{
			m_ppObjects[j]->ReleaseUploadBuffers();
		}
	}

	for (int i = 0; i < m_nShaders; i++)
	{
		m_pShaders[i].ReleaseUploadBuffers();
	}
	//if (m_ppShader)
	//{
	//	for (int j = 0; j < m_nShaders; j++) if (m_ppShader[j])
	//		m_ppShader[j]->ReleaseUploadBuffers();
	//}
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}

void CScene::PreviousTransform()
{

}

bool CScene::ProcessInput()
{
	return false;
}

//오브젝트의 애니메이션
void CScene::AnimateObjects(float fTimeElapsed)
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++)
		{
			m_ppObjects[j]->Animate(fTimeElapsed);
		}
	}

	if (m_pShaders)
	{
		for (int i = 0; i < m_nShaders; i++)
		{
			m_pShaders[i].AnimateObjects(fTimeElapsed);
		}
	}

}

//Render 하기전 우선 이건 안쓰고 CShader에서 설정하고 있다
void CScene::PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//루트 시그니쳐 연결
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dRootSignature.Get());

	//PSO 연결 ==> PSO에 루트 시그니쳐를 연결했으며 서로 다른 게 연결되어 있다면 런타임 
	pd3dCommandList->SetPipelineState(m_pd3dPipelineState.Get());

	//IA의 토폴로지를 트라이앵글리스트로 설정
	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}

//본 Render를 시작하기 전에 사전작업
void CScene::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, int nRenderTargets, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dRtvCPUHandles, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dDsvCPUHandle, CCamera* pCamera)
{
	//pCamera->SetViewportsAndScissorRects(pd3dCommandList); //뷰포트와 시저렉트를 실제 GPU 명령어로 Set해주는 거

	pd3dCommandList->SetGraphicsRootSignature(m_pd3dRootSignature.Get()); //루트 시그니쳐를 GPU에게 넘기는거
	pd3dCommandList->SetComputeRootSignature(m_cpd3dComputeRootSignature.Get());
	//if (pCamera)
	//{
	//	for (int i = 0; i < MAX_LIGHTS; i++)
	//	{
	//		//TODO 여기다가 조명 Shader 처리를 한번 에 쫙 해야한다 
	//	}
	//}
}

//Scene내에 오브젝트들을 그리는 동작 ==> Camara가 하나이고 움직이지 않는다는 전제하에서
void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//PrepareRender(pd3dCommandList);

	//pd3dCommandList->DrawInstanced(6,1,0,0);	//Draw 명령 여긴 임의로 그냥 그리는거라고 보면된다.

	 //그래픽 루트 시그너쳐를 파이프라인에 연결(설정)한다.
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dRootSignature.Get());

	//씬을 렌더링하는 것은 씬을 구성하는 셰이더(셰이더가 포함하는 객체)들을 렌더링하는 것이다.
	//for (int i = 0; i < m_nShaders; i++)
	//{
	//	m_ppShader[i]->Render(pd3dCommandList);
	//}
}

void CScene::OnlyShadowRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
}

void CScene::PrePareShadowMap(ID3D12GraphicsCommandList* pd3dCommandList)
{

}

void CScene::OnPostRender(ID3D12GraphicsCommandList* pd3dCommandList, int nRenderTargets, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dRtvCPUHandles
	, D3D12_CPU_DESCRIPTOR_HANDLE* pd3dDsvCPUHandle, CCamera* pCamera)
{

}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_nWeather == 0)
	{
		if (m_pIBLBRDFTexture) //환경맵이 존재를 한다면
		{
			m_pIBLBRDFTexture->UpdateShaderVariables(pd3dCommandList);
		}

		if (m_pIBLSpecluarTexture) //환경맵이 존재를 한다면
		{
			m_pIBLSpecluarTexture->UpdateShaderVariables(pd3dCommandList);
		}

		if (m_pIBLIrradianceTexture) //환경맵이 존재를 한다면
		{
			m_pIBLIrradianceTexture->UpdateShaderVariables(pd3dCommandList);
		}
	}
	else
	{
		if (m_pIBLBRDFTextureCloud) //환경맵이 존재를 한다면
		{
			m_pIBLBRDFTextureCloud->UpdateShaderVariables(pd3dCommandList);
		}

		if (m_pIBLSpecluarTextureCloud) //환경맵이 존재를 한다면
		{
			m_pIBLSpecluarTextureCloud->UpdateShaderVariables(pd3dCommandList);
		}

		if (m_pIBLIrradianceTextureCloud) //환경맵이 존재를 한다면
		{
			m_pIBLIrradianceTextureCloud->UpdateShaderVariables(pd3dCommandList);
		}
	}
}


void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	//pCamera->SetViewportsAndScissorRects(pd3dCommandList); //뷰포트와 시저렉트를 실제 GPU 명령어로 Set해주는 거

	//pd3dCommandList->SetGraphicsRootSignature(m_pd3dRootSignature.Get()); //루트 시그니쳐를 GPU에게 넘기는거
	
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	
	//ImGui에 설정된 플래그를 전달하는 역할 중
	UpdateShaderVariables(pd3dCommandList);

	if (pCamera)
	{
		pCamera->UpdateShaderVariables(pd3dCommandList); //카메라의 상수 버퍼나 여러 GPU나 Shader에 넘겨야할 명령어등을 Update해주는 부분
	}



	for (int i = 0; i < m_nShaders; i++)
	{
		if (m_pShaders)
		{
			m_pShaders[i].Render(pd3dCommandList, pCamera);
		}
	}
}

void CScene::OnGUI(ImGuiIO* pGuiIO)
{
	(void)pGuiIO;
}

//파일로 부터 몇개의 Texture를 사용하는지 읽어오는 함수
int CScene::CountTexturesFromFiles(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const std::vector<const char*>& fileList)
{
	int total = 0;
	for (auto file : fileList)
	{
		if (file && strlen(file) > 0)
			total += CGameObject::LoadCountTexturesFromFile(pd3dDevice, pd3dCommandList,pd3dGraphicsRootSignature, (char*)file, NULL);
	}
	return total;
}

XMVECTOR CScene::ClosetPointOnOBB(BoundingOrientedBox& obb, XMVECTOR point)
{
	
	// OBB 정보
	XMVECTOR vCenter = XMLoadFloat3(&obb.Center);
	XMVECTOR vExtents = XMLoadFloat3(&obb.Extents);
	XMVECTOR vOrientation = XMLoadFloat4(&obb.Orientation);

	// point를 OBB 로컬 공간으로 변환
	XMVECTOR vLocal = XMVectorSubtract(point, vCenter);

	// 회전 역변환 (world -> obb local)
	XMVECTOR vInvRot = XMQuaternionInverse(vOrientation);
	vLocal = XMVector3Rotate(vLocal, vInvRot);

	// 로컬 AABB에서 clamp
	XMVECTOR vClamped = XMVectorClamp(
		vLocal,
		XMVectorNegate(vExtents),
		vExtents
	);

	// 다시 월드 공간으로
	XMVECTOR vWorld = XMVector3Rotate(vClamped, vOrientation);
	vWorld = XMVectorAdd(vWorld, vCenter);

	return vWorld;
}

void CScene::CreateSSAOKernel()
{
}




LightControl::LightControl()
{
	// Light 기본 초기화
	m_Lights.radiance = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_Lights.direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_Lights.position = XMFLOAT3(0.0f, 0.0f, 0.0f);

	m_Lights.fallOffStart = 0.0f;
	m_Lights.fallOffEnd = 0.0f;
	m_Lights.spotPower = 1.0f;

	m_Lights.radius = 0;
	m_Lights.bIsShadowSet = 0;
	m_Lights.bIsUse = 0;
	m_Lights.type = 0; // NONE or INVALID

	m_Lights.viewProj = Matrix4x4::Identity();
	m_Lights.invProj = Matrix4x4::Identity();

	// Spot Light BoundingSphere 초기화
	spotSphere.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	spotSphere.Radius = 0.0f;
}

LightControl::~LightControl()
{
}

void LightControl::SetLight(UINT nType, XMFLOAT3 radiance, XMFLOAT3 direction, XMFLOAT3 position, UINT nbIsUse)
{
	m_Lights.type = nType;
	m_Lights.bIsUse = nbIsUse;

	m_Lights.radiance = radiance;
	m_Lights.direction = direction;
	m_Lights.position = position;

	m_Lights.bIsShadowSet = 1;

	m_Lights.radius = 0;
}

void LightControl::SetLight(UINT nType, XMFLOAT3 radiance, XMFLOAT3 direction, XMFLOAT3 position, float fallOffStart, float fallOffEnd, float spotPower, UINT nbIsUse, UINT nbIsShadowUse)
{
	m_Lights.type = nType;
	m_Lights.bIsUse = nbIsUse;
	m_Lights.bIsShadowSet = nbIsShadowUse;

	m_Lights.radiance = radiance;
	m_Lights.direction = direction;
	m_Lights.position = position;

	m_Lights.fallOffStart = fallOffStart;
	m_Lights.fallOffEnd = fallOffEnd;
	m_Lights.spotPower = spotPower;

	m_Lights.radius = 0;

	// Spot Light용 BoundingSphere
	spotSphere.Center = position;
	spotSphere.Radius = fallOffEnd;
}

void LightControl::SetPosition(XMFLOAT3 xmf3Position)
{
	m_Lights.position = xmf3Position;
	spotSphere.Center = xmf3Position;
}

void LightControl::SetDirection(XMFLOAT3 xmf3Direction)
{
	m_Lights.direction = xmf3Direction;
}

void LightControl::SetShadowUse(UINT nbIsUse)
{
	m_Lights.bIsShadowSet = nbIsUse;
}

Light LightControl::GetLight()
{
	return m_Lights;
}

BoundingSphere LightControl::GetBoundingSphere()
{
	return spotSphere;
}


