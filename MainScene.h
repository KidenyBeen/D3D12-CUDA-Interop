#pragma once

#include "Scene.h"
#include "DirectXCudaInterop.h"

// Client-only gameplay scene. Scene owns the reusable rendering/resource
// infrastructure; concrete game objects are intentionally not carried over.
class CMainScene final : public CScene
{
private:
	enum class ERenderingMethod
	{
		Default = 0,
		ComputeShader,
		Cuda
	};

	ERenderingMethod m_eRenderingMethod = ERenderingMethod::Default;
	int m_nBlurSet = 0;
	CTexture* m_pCudaRenderTexture = nullptr;
	CPostProcessingObject* m_pPostProcessingObject = nullptr;
	CDirectXCudaInterop m_CudaInterop;
	ComPtr<ID3D12CommandAllocator> m_pd3dCudaPostCommandAllocator;
	D3D12_CPU_DESCRIPTOR_HANDLE m_d3dCudaRtvCPUDescriptorHandle = {};
	D3D12_CPU_DESCRIPTOR_HANDLE m_d3dCudaTempRtvCPUDescriptorHandle = {};

public:
	CMainScene(HWND hWnd, CGameTimer* timer) : CScene(hWnd, timer) {}
	~CMainScene() override = default;

	//GameFramwwork에 있는 카메라와는 별개에 카메라로 Scene에 Render에 넘어온 pCamera는 현재 사용중이 아니다
	shared_ptr<CCamera> m_pCamera = nullptr;
	shared_ptr<CPlayer> m_pPlayer = nullptr;
	
	shared_ptr<CPlanObject> m_pPlanObject = nullptr;

	bool OnProcessingMouseMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;
	bool ProcessInput() override;
	void BuildObjects(ID3D12Device* device, ID3D12GraphicsCommandList* commandList) override;
	void CreateShaderResourceViewCudaRtvs(ID3D12Device* device,
		ID3D12GraphicsCommandList* commandList, DXGI_FORMAT format,
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle) override;
	void InitLights() override;
	void CreateShaderVariables(ID3D12Device* device, ID3D12GraphicsCommandList* commandList) override;
	void CreateSSAOKernel() override;
	void ReleaseObjects() override;
	void ReleaseUploadBuffers() override;
	void ReleaseShaderVariables() override;
	void PreviousTransform() override;
	void AnimateObjects(float elapsedTime) override;
	void OnPreRender(ID3D12GraphicsCommandList* commandList, int renderTargetCount,
		D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles, D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle,
		CCamera* camera) override;
	void Render(ID3D12GraphicsCommandList* commandList, CCamera* camera) override;
	void OnPostRender(ID3D12GraphicsCommandList* commandList,
		ID3D12CommandQueue* commandQueue, int renderTargetCount,
		D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles, D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle,
		CCamera* camera) override;
	bool IsCudaPostProcessEnabled() const override;
	void UpdateShaderVariables(ID3D12GraphicsCommandList* commandList) override;
	void OnGUI(ImGuiIO* gui) override;
	void OnEnter() override;
};
