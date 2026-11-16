#include "stdafx.h"
#include "Renderer.h"

CRenderer::~CRenderer()
{
	if (m_cpd3dGaussianBlurInfo && m_pGaussianBlurInfo)
	{
		m_cpd3dGaussianBlurInfo->Unmap(0, nullptr);
	}
	m_pGaussianBlurInfo = nullptr;
}

void CRenderer::CreateGaussianBlurResources(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList)
{
	const UINT nBufferSize = (sizeof(CB_GAUSSIAN_BLUR_INFO) + 255) & ~255;
	m_cpd3dGaussianBlurInfo = ::CreateBufferResource(pd3dDevice, pd3dCommandList,
		nullptr, nBufferSize, D3D12_HEAP_TYPE_UPLOAD,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr);
	if (m_cpd3dGaussianBlurInfo)
	{
		m_cpd3dGaussianBlurInfo->Map(0, nullptr,
			reinterpret_cast<void**>(&m_pGaussianBlurInfo));
	}
}

void CRenderer::GaussianBlur(ID3D12GraphicsCommandList* pd3dCommandList,
	CTexture* pRenderTexture, CShader* pShader, int nBlurRadius, float fBlurSigma)
{
	CComputeShader* pComputeShader = dynamic_cast<CComputeShader*>(pShader);
	if (!pd3dCommandList || !pRenderTexture || !pComputeShader ||
		!m_cpd3dGaussianBlurInfo || !m_pGaussianBlurInfo)
	{
		return;
	}

	if (nBlurRadius < 0)
	{
		nBlurRadius = 0;
	}
	if (fBlurSigma <= 0.0f)
	{
		fBlurSigma = max(1.0f, static_cast<float>(nBlurRadius) * 0.5f);
	}

	m_pGaussianBlurInfo->m_nBlurRadius = nBlurRadius;
	m_pGaussianBlurInfo->m_fBlurSigma = fBlurSigma;
	m_pGaussianBlurInfo->m_nTextureWidth = FRAME_BUFFER_WIDTH;
	m_pGaussianBlurInfo->m_nTextureHeight = FRAME_BUFFER_HEIGHT;
	pd3dCommandList->SetComputeRootConstantBufferView(0,
		m_cpd3dGaussianBlurInfo->GetGPUVirtualAddress());

	ID3D12Resource* pSource = pRenderTexture->GetResource(0).Get();
	ID3D12Resource* pTemporary = pRenderTexture->GetResource(1).Get();
	::SynchronizeResourceTransition(pd3dCommandList, pSource,
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	::SynchronizeResourceTransition(pd3dCommandList, pTemporary,
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	pComputeShader->OnPreparePSO(pd3dCommandList, 1);
	pd3dCommandList->SetComputeRootDescriptorTable(2,
		pRenderTexture->GetGpuDescriptorHandle(0));
	pd3dCommandList->SetComputeRootDescriptorTable(5,
		pRenderTexture->GetUavGpuDescriptorHandle(1));
	pd3dCommandList->Dispatch(
		UINT(ceil(FRAME_BUFFER_WIDTH / 32.0f)),
		UINT(ceil(FRAME_BUFFER_HEIGHT / 32.0f)), 1);
	::SynchronizeUAVBarrier(pd3dCommandList, pTemporary);

	::SynchronizeResourceTransition(pd3dCommandList, pSource,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	::SynchronizeResourceTransition(pd3dCommandList, pTemporary,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	pComputeShader->OnPreparePSO(pd3dCommandList, 2);
	pd3dCommandList->SetComputeRootDescriptorTable(2,
		pRenderTexture->GetGpuDescriptorHandle(1));
	pd3dCommandList->SetComputeRootDescriptorTable(5,
		pRenderTexture->GetUavGpuDescriptorHandle(0));
	pd3dCommandList->Dispatch(
		UINT(ceil(FRAME_BUFFER_WIDTH / 32.0f)),
		UINT(ceil(FRAME_BUFFER_HEIGHT / 32.0f)), 1);
	::SynchronizeUAVBarrier(pd3dCommandList, pSource);

	::SynchronizeResourceTransition(pd3dCommandList, pSource,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	::SynchronizeResourceTransition(pd3dCommandList, pTemporary,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON);
}

void CRenderer::PixelShaderGaussianBlur(ID3D12GraphicsCommandList* pd3dCommandList,
	CTexture* pRenderTexture, CPostProcessingObject* pPostProcessingObject,
	D3D12_CPU_DESCRIPTOR_HANDLE d3dSourceRtvHandle,
	D3D12_CPU_DESCRIPTOR_HANDLE d3dTemporaryRtvHandle,
	int nBlurRadius, float fBlurSigma)
{
	if (!pd3dCommandList || !pRenderTexture || !pPostProcessingObject)
	{
		return;
	}

	if (nBlurRadius < 0)
	{
		nBlurRadius = 0;
	}
	if (fBlurSigma <= 0.0f)
	{
		fBlurSigma = max(1.0f, static_cast<float>(nBlurRadius) * 0.5f);
	}

	CB_GAUSSIAN_BLUR_INFO blurInfo = {};
	blurInfo.m_nBlurRadius = nBlurRadius;
	blurInfo.m_fBlurSigma = fBlurSigma;
	blurInfo.m_nTextureWidth = FRAME_BUFFER_WIDTH;
	blurInfo.m_nTextureHeight = FRAME_BUFFER_HEIGHT;
	pd3dCommandList->SetGraphicsRoot32BitConstants(0, 4, &blurInfo, 0);

	ID3D12Resource* pSource = pRenderTexture->GetResource(0).Get();
	ID3D12Resource* pTemporary = pRenderTexture->GetResource(1).Get();
	::SynchronizeResourceTransition(pd3dCommandList, pSource,
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	::SynchronizeResourceTransition(pd3dCommandList, pTemporary,
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

	pd3dCommandList->OMSetRenderTargets(1, &d3dTemporaryRtvHandle, TRUE, nullptr);
	pPostProcessingObject->SetPostProcessTexture(pRenderTexture, 0);
	pPostProcessingObject->SetPostProcessingPass(1);
	pPostProcessingObject->Render(pd3dCommandList, nullptr);

	::SynchronizeResourceTransition(pd3dCommandList, pSource,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	::SynchronizeResourceTransition(pd3dCommandList, pTemporary,
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	pd3dCommandList->OMSetRenderTargets(1, &d3dSourceRtvHandle, TRUE, nullptr);
	pPostProcessingObject->SetPostProcessTexture(pRenderTexture, 1);
	pPostProcessingObject->SetPostProcessingPass(2);
	pPostProcessingObject->Render(pd3dCommandList, nullptr);

	::SynchronizeResourceTransition(pd3dCommandList, pSource,
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	::SynchronizeResourceTransition(pd3dCommandList, pTemporary,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON);
	pPostProcessingObject->SetPostProcessTexture(pRenderTexture, 0);
	pPostProcessingObject->SetPostProcessingPass(0);
}

void CRenderer::SSAOProcess(ID3D12GraphicsCommandList* pd3dCommandList, CTexture* pRenderTexture, CTexture* pComputeRenderTexture, ID3D12Resource* pDepthStancilResource , CShader* pShader , D3D12_GPU_DESCRIPTOR_HANDLE DepthMapSrvHandle)
{
	CComputeShader* pTmpShader = dynamic_cast<CComputeShader*>(pShader);
	SynchronizeResourceTransition(pd3dCommandList, pRenderTexture->GetResource(0).Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	::SynchronizeResourceTransition(pd3dCommandList, pComputeRenderTexture->GetResource(1).Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	::SynchronizeResourceTransition(pd3dCommandList, pComputeRenderTexture->GetResource(2).Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	::SynchronizeResourceTransition(pd3dCommandList, pRenderTexture->GetResource(0).Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	::SynchronizeResourceTransition(pd3dCommandList, pDepthStancilResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	::SynchronizeResourceTransition(pd3dCommandList, pComputeRenderTexture->GetResource(3).Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	//SSAO 생성
	pTmpShader->OnPreparePSO(pd3dCommandList, 3);
	pd3dCommandList->SetComputeRootDescriptorTable(2, DepthMapSrvHandle);//깊이맵
	pd3dCommandList->SetComputeRootDescriptorTable(3, pComputeRenderTexture->GetGpuDescriptorHandle(0)); //노멀맵
	pd3dCommandList->SetComputeRootDescriptorTable(4, pComputeRenderTexture->GetGpuDescriptorHandle(1)); //IBL
	pd3dCommandList->SetComputeRootDescriptorTable(5, pComputeRenderTexture->GetUavGpuDescriptorHandle(2)); //SSAO결과를 저장한다
	pd3dCommandList->Dispatch(UINT(ceil(FRAME_BUFFER_WIDTH / 32.0f)),
		UINT(ceil(FRAME_BUFFER_HEIGHT / 32.0f)), 1);
	::SynchronizeUAVBarrier(pd3dCommandList, pComputeRenderTexture->GetResource(2).Get());
	//::SynchronizeResourceTransition(pd3dCommandList, pComputeRenderTexture->GetResource(2).Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	//SSAO Blur
	for (int i = 0; i < 5; i++)
	{

		::SynchronizeResourceTransition(pd3dCommandList, pComputeRenderTexture->GetResource(2).Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		::SynchronizeResourceTransition(pd3dCommandList, pComputeRenderTexture->GetResource(3).Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		pTmpShader->OnPreparePSO(pd3dCommandList, 1);
		pd3dCommandList->SetComputeRootDescriptorTable(2, pComputeRenderTexture->GetGpuDescriptorHandle(2));
		pd3dCommandList->SetComputeRootDescriptorTable(5, pComputeRenderTexture->GetUavGpuDescriptorHandle(3));
		pd3dCommandList->Dispatch(UINT(ceil(FRAME_BUFFER_WIDTH / 32.0f)),
			UINT(ceil(FRAME_BUFFER_HEIGHT / 32.0f)), 1);
		::SynchronizeUAVBarrier(pd3dCommandList, pComputeRenderTexture->GetResource(3).Get());

		::SynchronizeResourceTransition(pd3dCommandList, pComputeRenderTexture->GetResource(2).Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		::SynchronizeResourceTransition(pd3dCommandList, pComputeRenderTexture->GetResource(3).Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);


		pTmpShader->OnPreparePSO(pd3dCommandList, 2);
		pd3dCommandList->SetComputeRootDescriptorTable(2, pComputeRenderTexture->GetGpuDescriptorHandle(3));
		pd3dCommandList->SetComputeRootDescriptorTable(5, pComputeRenderTexture->GetUavGpuDescriptorHandle(2));
		pd3dCommandList->Dispatch(UINT(ceil(FRAME_BUFFER_WIDTH / 32.0f)),
			UINT(ceil(FRAME_BUFFER_HEIGHT / 32.0f)), 1);
		::SynchronizeUAVBarrier(pd3dCommandList, pComputeRenderTexture->GetResource(2).Get());

	}

	::SynchronizeResourceTransition(pd3dCommandList, pComputeRenderTexture->GetResource(2).Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	::SynchronizeResourceTransition(pd3dCommandList, pComputeRenderTexture->GetResource(3).Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON);
	::SynchronizeResourceTransition(pd3dCommandList, pComputeRenderTexture->GetResource(0).Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	//SSAO 적용중
	pTmpShader->OnPreparePSO(pd3dCommandList, 0);
	pd3dCommandList->SetComputeRootDescriptorTable(2, pComputeRenderTexture->GetGpuDescriptorHandle(2));
	pd3dCommandList->SetComputeRootDescriptorTable(5, pRenderTexture->GetUavGpuDescriptorHandle(0)); //SSAO 적용한 HDRI
	pd3dCommandList->SetComputeRootDescriptorTable(6, pComputeRenderTexture->GetUavGpuDescriptorHandle(0)); //Bloom넘길거
	pd3dCommandList->Dispatch(UINT(ceil(FRAME_BUFFER_WIDTH / 32.0f)),
		UINT(ceil(FRAME_BUFFER_HEIGHT / 32.0f)), 1);

	::SynchronizeUAVBarrier(pd3dCommandList, pRenderTexture->GetResource(0).Get());
	::SynchronizeUAVBarrier(pd3dCommandList, pComputeRenderTexture->GetResource(1).Get());

}

void CRenderer::BloomProcess(ID3D12GraphicsCommandList* pd3dCommandList, CTexture* pRenderTexture, CTexture* pComputeRenderTexture, ID3D12Resource* pDepthStancilResource, CShader* pShader)
{
	CComputeShader* pTmpShader = dynamic_cast<CComputeShader*>(pShader);

	::SynchronizeResourceTransition(pd3dCommandList, pComputeRenderTexture->GetResource(2).Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	//Bloom 
	pTmpShader->OnPreparePSO(pd3dCommandList, 4);
	pd3dCommandList->SetComputeRootDescriptorTable(2, pComputeRenderTexture->GetUavGpuDescriptorHandle(0));
	pd3dCommandList->SetComputeRootDescriptorTable(5, pComputeRenderTexture->GetUavGpuDescriptorHandle(2));
	pd3dCommandList->Dispatch(UINT(ceil(FRAME_BUFFER_WIDTH / 32.0f)),
		UINT(ceil(FRAME_BUFFER_HEIGHT / 32.0f)), 1);
	::SynchronizeUAVBarrier(pd3dCommandList, pComputeRenderTexture->GetResource(2).Get());

	//Blur
	for (int i = 0; i < 10; i++)
	{

		::SynchronizeResourceTransition(pd3dCommandList, pComputeRenderTexture->GetResource(2).Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		::SynchronizeResourceTransition(pd3dCommandList, pComputeRenderTexture->GetResource(3).Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		pTmpShader->OnPreparePSO(pd3dCommandList, 1);
		pd3dCommandList->SetComputeRootDescriptorTable(2, pComputeRenderTexture->GetGpuDescriptorHandle(2));
		pd3dCommandList->SetComputeRootDescriptorTable(5, pComputeRenderTexture->GetUavGpuDescriptorHandle(3));
		pd3dCommandList->Dispatch(UINT(ceil(FRAME_BUFFER_WIDTH / 32.0f)),
			UINT(ceil(FRAME_BUFFER_HEIGHT / 32.0f)), 1);

		::SynchronizeUAVBarrier(pd3dCommandList, pComputeRenderTexture->GetResource(3).Get());
		::SynchronizeUAVBarrier(pd3dCommandList, pComputeRenderTexture->GetResource(2).Get());
		::SynchronizeResourceTransition(pd3dCommandList, pComputeRenderTexture->GetResource(2).Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		::SynchronizeResourceTransition(pd3dCommandList, pComputeRenderTexture->GetResource(3).Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		pTmpShader->OnPreparePSO(pd3dCommandList, 2);
		pd3dCommandList->SetComputeRootDescriptorTable(2, pComputeRenderTexture->GetGpuDescriptorHandle(3));
		pd3dCommandList->SetComputeRootDescriptorTable(5, pComputeRenderTexture->GetUavGpuDescriptorHandle(2));
		pd3dCommandList->Dispatch(UINT(ceil(FRAME_BUFFER_WIDTH / 32.0f)),
			UINT(ceil(FRAME_BUFFER_HEIGHT / 32.0f)), 1);
		::SynchronizeUAVBarrier(pd3dCommandList, pComputeRenderTexture->GetResource(2).Get());
		::SynchronizeUAVBarrier(pd3dCommandList, pComputeRenderTexture->GetResource(3).Get());
	}
	
	::SynchronizeResourceTransition(pd3dCommandList, pComputeRenderTexture->GetResource(2).Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	::SynchronizeResourceTransition(pd3dCommandList, pComputeRenderTexture->GetResource(3).Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON);
}

void CRenderer::CSCombine(ID3D12GraphicsCommandList* pd3dCommandList, CTexture* pRenderTexture, CTexture* pComputeRenderTexture, ID3D12Resource* pDepthStancilResource , CShader* pShader)
{
	CComputeShader* pTmpShader = dynamic_cast<CComputeShader*>(pShader);

	pTmpShader->OnPreparePSO(pd3dCommandList, 5);
	pd3dCommandList->SetComputeRootDescriptorTable(2, pComputeRenderTexture->GetGpuDescriptorHandle(2));
	pd3dCommandList->SetComputeRootDescriptorTable(5, pRenderTexture->GetUavGpuDescriptorHandle(0));
	pd3dCommandList->Dispatch(UINT(ceil(FRAME_BUFFER_WIDTH / 32.0f)),
		UINT(ceil(FRAME_BUFFER_HEIGHT / 32.0f)), 1);
	::SynchronizeUAVBarrier(pd3dCommandList, pRenderTexture->GetResource(0).Get());
	// UAV -> SRV
	::SynchronizeResourceTransition(pd3dCommandList, pRenderTexture->GetResource(0).Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET);


	//Compute SRV-> SRV
	::SynchronizeResourceTransition(pd3dCommandList, pComputeRenderTexture->GetResource(0).Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
	::SynchronizeResourceTransition(pd3dCommandList, pComputeRenderTexture->GetResource(1).Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON);
	::SynchronizeResourceTransition(pd3dCommandList, pComputeRenderTexture->GetResource(2).Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON);

	::SynchronizeResourceTransition(pd3dCommandList, pDepthStancilResource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
}
