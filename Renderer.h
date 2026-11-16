#pragma once
#include "GameObject.h"
#include "Shader.h"

class CRenderer
{
private:
	struct CB_GAUSSIAN_BLUR_INFO
	{
		int m_nBlurRadius = 0;
		float m_fBlurSigma = 1.0f;
		UINT m_nTextureWidth = 0;
		UINT m_nTextureHeight = 0;
	};

	ComPtr<ID3D12Resource> m_cpd3dGaussianBlurInfo;
	CB_GAUSSIAN_BLUR_INFO* m_pGaussianBlurInfo = nullptr;

public:

	CRenderer() {};
	virtual ~CRenderer();


	//m_pComputeShader->OnPreparePSO(pd3dCommandList, 0); // PostPorcessing : 0, BlurX : 1, BlurY :2, SSAO : 3
	void SSAOProcess(ID3D12GraphicsCommandList* pd3dCommandList , CTexture* pRenderTexture, CTexture* pComputeRenderTexture , ID3D12Resource* pDepthStancilResource , CShader* pShader , D3D12_GPU_DESCRIPTOR_HANDLE DepthMapSrvHandle) ;
	void BloomProcess(ID3D12GraphicsCommandList* pd3dCommandList , CTexture* pRenderTexture, CTexture* pComputeRenderTexture , ID3D12Resource* pDepthStancilResource , CShader* pShader) ;
	void CSCombine(ID3D12GraphicsCommandList* pd3dCommandList , CTexture* pRenderTexture, CTexture* pComputeRenderTexture , ID3D12Resource* pDepthStancilResource , CShader* pShader) ;
	void CreateGaussianBlurResources(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList);
	void GaussianBlur(ID3D12GraphicsCommandList* pd3dCommandList,
		CTexture* pRenderTexture, CShader* pShader, int nBlurRadius, float fBlurSigma);
	void PixelShaderGaussianBlur(ID3D12GraphicsCommandList* pd3dCommandList,
		CTexture* pRenderTexture, CPostProcessingObject* pPostProcessingObject,
		D3D12_CPU_DESCRIPTOR_HANDLE d3dSourceRtvHandle,
		D3D12_CPU_DESCRIPTOR_HANDLE d3dTemporaryRtvHandle,
		int nBlurRadius, float fBlurSigma);
};

