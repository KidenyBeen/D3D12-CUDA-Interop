#include "stdafx.h"
#include "Renderer.h"

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
