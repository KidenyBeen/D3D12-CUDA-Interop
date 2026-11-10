#include "stdafx.h"
#include "MainScene.h"

bool CMainScene::OnProcessingMouseMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return CScene::OnProcessingMouseMessage(hWnd, message, wParam, lParam);
}

bool CMainScene::OnProcessingKeyboardMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return CScene::OnProcessingKeyboardMessage(hWnd, message, wParam, lParam);
}

bool CMainScene::ProcessInput() { return CScene::ProcessInput(); }

void CMainScene::BuildObjects(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{

	CScene::BuildObjects(device, commandList);

	m_pd3dRootSignature = CGraphicsCommon::GetInstance().m_cpd3dMainSceneRootSignature;
	m_cpd3dComputeRootSignature = CGraphicsCommon::GetInstance().m_cpd3dMainSceneComputeRootSignature;
	m_cpd3dDevice = device;
	m_cpd3dCommandList = commandList;

	if (m_pDescriptorHeap == nullptr)
	{
		m_pDescriptorHeap = new CDescriptorHeap();
	}

	CreateCbvSrvDescriptorHeaps(device, 0, 12, 6);
	InitLights();
}

void CMainScene::InitLights() { CScene::InitLights(); }
void CMainScene::CreateShaderVariables(ID3D12Device* device, ID3D12GraphicsCommandList* commandList) { CScene::CreateShaderVariables(device, commandList); }
void CMainScene::CreateSSAOKernel() { CScene::CreateSSAOKernel(); }
void CMainScene::ReleaseObjects() { CScene::ReleaseObjects(); }
void CMainScene::ReleaseUploadBuffers() { CScene::ReleaseUploadBuffers(); }
void CMainScene::ReleaseShaderVariables() { CScene::ReleaseShaderVariables(); }
void CMainScene::PreviousTransform() { CScene::PreviousTransform(); }
void CMainScene::AnimateObjects(float elapsedTime) { CScene::AnimateObjects(elapsedTime); }

void CMainScene::OnPreRender(ID3D12GraphicsCommandList* commandList, int renderTargetCount,
	D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles, D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle, CCamera* camera)
{
	CScene::OnPreRender(commandList, renderTargetCount, rtvHandles, dsvHandle, camera);
}

void CMainScene::Render(ID3D12GraphicsCommandList* commandList, CCamera* camera) 
{ 
	CScene::Render(commandList, camera);
}

void CMainScene::OnPostRender(ID3D12GraphicsCommandList* commandList, int renderTargetCount,
	D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles, D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle, CCamera* camera)
{
	CScene::OnPostRender(commandList, renderTargetCount, rtvHandles, dsvHandle, camera);
}

void CMainScene::UpdateShaderVariables(ID3D12GraphicsCommandList* commandList) 
{

CScene::UpdateShaderVariables(commandList);
}
void CMainScene::OnGUI(ImGuiIO* gui) { CScene::OnGUI(gui); }
void CMainScene::OnEnter() { std::cout << "MainScene OnEnter" << std::endl; }
