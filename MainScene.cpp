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

	m_pTestObject = new CTestObject(device, commandList, m_pd3dRootSignature.Get());
}

void CMainScene::InitLights() { CScene::InitLights(); }
void CMainScene::CreateShaderVariables(ID3D12Device* device, ID3D12GraphicsCommandList* commandList) { CScene::CreateShaderVariables(device, commandList); }
void CMainScene::CreateSSAOKernel() { CScene::CreateSSAOKernel(); }
void CMainScene::ReleaseObjects()
{
	delete m_pTestObject;
	m_pTestObject = nullptr;
	CScene::ReleaseObjects();
}
void CMainScene::ReleaseUploadBuffers()
{
	if (m_pTestObject)
	{
		m_pTestObject->ReleaseUploadBuffers();
	}
	CScene::ReleaseUploadBuffers();
}
void CMainScene::ReleaseShaderVariables() { CScene::ReleaseShaderVariables(); }
void CMainScene::PreviousTransform() { CScene::PreviousTransform(); }
void CMainScene::AnimateObjects(float elapsedTime) { CScene::AnimateObjects(elapsedTime); }

void CMainScene::OnPreRender(ID3D12GraphicsCommandList* commandList, int renderTargetCount,
	D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles, D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle, CCamera* camera)
{
	CScene::OnPreRender(commandList, renderTargetCount, rtvHandles, dsvHandle, camera);
	commandList->OMSetRenderTargets(renderTargetCount, rtvHandles, TRUE, dsvHandle);
}

void CMainScene::Render(ID3D12GraphicsCommandList* commandList, CCamera* camera) 
{ 
	CScene::Render(commandList, camera);

	if (!m_pTestObject)
	{
		OutputDebugStringW(L"[CMainScene] TestObject is null.\n");
		return;
	}

	commandList->SetGraphicsRootSignature(m_pd3dRootSignature.Get());
	m_pTestObject->Render(commandList, camera);
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
void CMainScene::OnGUI(ImGuiIO* gui)
{
	//CScene::OnGUI(gui);

	if (!gui)
	{
		return;
	}

	// Keep the settings window anchored to the upper-left and sized to one
	// quarter of the current client area's width and height.
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(
		ImVec2(gui->DisplaySize.x * 0.25f, gui->DisplaySize.y * 0.25f),
		ImGuiCond_Always);

	if (ImGui::Begin("Render Settings"))
	{
		if (ImGui::CollapsingHeader("Rendering Method", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::Checkbox("Compute Shader", &ComputeShaderSet) && ComputeShaderSet)
			{
				CUDASet = false;
			}

			if (ImGui::Checkbox("CUDA", &CUDASet) && CUDASet)
			{
				ComputeShaderSet = false;
			}
		}

		if (ImGui::CollapsingHeader("Blur Settings", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderInt("Blur", &BlurSet, 0, 20);
		}
	}
	ImGui::End();
}
void CMainScene::OnEnter() { std::cout << "MainScene OnEnter" << std::endl; }
