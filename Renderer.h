#pragma once
#include "GameObject.h"
#include "Shader.h"

class CRenderer
{
public:

	CRenderer() {};
	virtual ~CRenderer() {};


	//m_pComputeShader->OnPreparePSO(pd3dCommandList, 0); // PostPorcessing : 0, BlurX : 1, BlurY :2, SSAO : 3
	void SSAOProcess(ID3D12GraphicsCommandList* pd3dCommandList , CTexture* pRenderTexture, CTexture* pComputeRenderTexture , ID3D12Resource* pDepthStancilResource , CShader* pShader , D3D12_GPU_DESCRIPTOR_HANDLE DepthMapSrvHandle) ;
	void BloomProcess(ID3D12GraphicsCommandList* pd3dCommandList , CTexture* pRenderTexture, CTexture* pComputeRenderTexture , ID3D12Resource* pDepthStancilResource , CShader* pShader) ;
	void CSCombine(ID3D12GraphicsCommandList* pd3dCommandList , CTexture* pRenderTexture, CTexture* pComputeRenderTexture , ID3D12Resource* pDepthStancilResource , CShader* pShader) ;
};

