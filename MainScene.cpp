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

	// CUDA 공유 원본과 Compute Shader Temp Texture의 SRV/UAV 슬롯을 추가한다.
	CreateCbvSrvDescriptorHeaps(device, 0, 15, 8);
	InitLights();

	m_pTestObject = new CTestObject(device, commandList, m_pd3dRootSignature.Get());
	m_pPostProcessingObject = new CPostProcessingObject(device, commandList, m_pd3dRootSignature.Get());
}

void CMainScene::CreateShaderResourceViewCudaRtvs(ID3D12Device* device,
	ID3D12GraphicsCommandList* commandList, DXGI_FORMAT format,
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle)
{
	// 0번은 CUDA 공유 원본, 1번은 Compute Shader 블러용 Temp Texture로 사용한다.
	m_pCudaRenderTexture = new CTexture(2, RESOURCE_TEXTURE2D, 0, 1);
	D3D12_CLEAR_VALUE clearValue = { format, { 0.0f, 0.0f, 0.0f, 1.0f } };
	m_pCudaRenderTexture->CreateSharedTexture(device, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT,
		format, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COMMON, &clearValue, RESOURCE_TEXTURE2D, 0);
	m_pCudaRenderTexture->CreateTexture(device, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT,
		format, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COMMON, &clearValue, RESOURCE_TEXTURE2D, 1);

	DXGI_FORMAT viewFormats[2] = { format, format };
	CreateUnorderedAccessView(device, m_pCudaRenderTexture, viewFormats, 0, 4);
	CreateShaderResourceViews(device, m_pCudaRenderTexture, 0, 19);
	if (m_pPostProcessingObject)
	{
		m_pPostProcessingObject->SetPostProcessTexture(m_pCudaRenderTexture);
	}

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;
	device->CreateRenderTargetView(m_pCudaRenderTexture->GetResource(0).Get(), &rtvDesc, rtvHandle);
	m_d3dCudaRtvCPUDescriptorHandle = rtvHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE tempRtvHandle = rtvHandle;
	tempRtvHandle.ptr += ::gnRtvDescriptorIncrementSize;
	device->CreateRenderTargetView(m_pCudaRenderTexture->GetResource(1).Get(), &rtvDesc, tempRtvHandle);
	m_d3dCudaTempRtvCPUDescriptorHandle = tempRtvHandle;

	const HRESULT allocatorResult = device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(m_pd3dCudaPostCommandAllocator.ReleaseAndGetAddressOf()));
	if (FAILED(allocatorResult))
	{
		OutputDebugStringW(L"[CMainScene] Failed to create the CUDA post-process command allocator.\n");
		return;
	}

	// DX12 View 생성이 끝난 동일 리소스를 CUDA surface로 가져온다.
	m_CudaInterop.InitializeTexture(device, m_pCudaRenderTexture->GetResource(0).Get(),
		format, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
}

void CMainScene::InitLights() { CScene::InitLights(); }
void CMainScene::CreateShaderVariables(ID3D12Device* device, ID3D12GraphicsCommandList* commandList) { CScene::CreateShaderVariables(device, commandList); }
void CMainScene::CreateSSAOKernel() { CScene::CreateSSAOKernel(); }
void CMainScene::ReleaseObjects()
{
	m_CudaInterop.Shutdown();
	delete m_pCudaRenderTexture;
	m_pCudaRenderTexture = nullptr;
	delete m_pTestObject;
	m_pTestObject = nullptr;
	delete m_pPostProcessingObject;
	m_pPostProcessingObject = nullptr;
	m_pd3dCudaPostCommandAllocator.Reset();
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

	if (IsCudaPostProcessEnabled())
	{
		m_CudaInterop.SetExecutionMode(CDirectXCudaInterop::EExecutionMode::CudaShared, m_pd3dCommandQueue);

		// 같은 프레임의 원본을 CUDA 공유 Texture에 먼저 렌더링한다.
		::SynchronizeResourceTransition(commandList, m_pCudaRenderTexture->GetResource(0).Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
		const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		commandList->ClearRenderTargetView(m_d3dCudaRtvCPUDescriptorHandle, clearColor, 0, nullptr);
		commandList->OMSetRenderTargets(1, &m_d3dCudaRtvCPUDescriptorHandle, TRUE, dsvHandle);
	}
	else
	{
		if (m_CudaInterop.IsInitialized() && m_pd3dCommandQueue)
		{
			m_CudaInterop.SetExecutionMode(
				CDirectXCudaInterop::EExecutionMode::ComputeShaderOnly, m_pd3dCommandQueue);
		}
		const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		::SynchronizeResourceTransition(commandList, m_pCudaRenderTexture->GetResource(0).Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ClearRenderTargetView(m_d3dCudaRtvCPUDescriptorHandle, clearColor, 0, nullptr);
		commandList->OMSetRenderTargets(1, &m_d3dCudaRtvCPUDescriptorHandle, TRUE, dsvHandle);
		//commandList->OMSetRenderTargets(renderTargetCount, rtvHandles, TRUE, dsvHandle);
	}
}

void CMainScene::Render(ID3D12GraphicsCommandList* commandList, CCamera* camera) 
{ 
	CScene::Render(commandList, camera);

	if (!m_pTestObject)
	{
		OutputDebugStringW(L"[CMainScene] TestObject is null.\n");
		return;
	}
	commandList->SetDescriptorHeaps(1, m_pDescriptorHeap->m_pd3dCbvSrvDescriptorHeap.GetAddressOf());
	commandList->SetGraphicsRootSignature(m_pd3dRootSignature.Get());
	m_pTestObject->Render(commandList, camera);
}

void CMainScene::OnPostRender(ID3D12GraphicsCommandList* commandList,
	ID3D12CommandQueue* commandQueue, int renderTargetCount,
	D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles, D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle, CCamera* camera)
{
	if (IsCudaPostProcessEnabled())
	{
		ID3D12Resource* pSharedSource = m_pCudaRenderTexture->GetResource(0).Get();
		::SynchronizeResourceTransition(commandList, pSharedSource,
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		// CUDA가 같은 프레임의 DX12 결과를 읽을 수 있도록 첫 CommandList를 먼저 제출한다.
		HRESULT result = commandList->Close();
		if (SUCCEEDED(result))
		{
			ComPtr<ID3D12CommandList> cudaInputLists[] = { commandList };
			commandQueue->ExecuteCommandLists(_countof(cudaInputLists), cudaInputLists->GetAddressOf());
		}

		bool cudaResultReady = SUCCEEDED(result);

		CDirectXCudaInterop::KernelParameters kernelParameters = {};

		kernelParameters.m_nBlurRadius = m_nBlurSet;
		kernelParameters.m_fBlurSigma = max(1.0f, static_cast<float>(m_nBlurSet) * 0.5f);

		if (cudaResultReady && !m_CudaInterop.ExecuteCuda(commandQueue, kernelParameters))
		{
			OutputDebugStringA(m_CudaInterop.GetLastError().c_str());
			OutputDebugStringA("\n");
			cudaResultReady = false;
		}
		if (cudaResultReady && !m_CudaInterop.WaitForCuda(commandQueue))
		{
			OutputDebugStringA(m_CudaInterop.GetLastError().c_str());
			OutputDebugStringA("\n");
			cudaResultReady = false;
		}

		// 첫 제출과 독립된 Allocator로 최종 출력과 GUI/Present 명령 기록을 계속한다.
		result = m_pd3dCudaPostCommandAllocator->Reset();
		if (SUCCEEDED(result))
		{
			result = commandList->Reset(m_pd3dCudaPostCommandAllocator.Get(), nullptr);
		}
		if (FAILED(result))
		{
			OutputDebugStringW(L"[CMainScene] Failed to reset the CUDA output command list.\n");
			return;
		}

		commandList->OMSetRenderTargets(renderTargetCount, rtvHandles, TRUE, dsvHandle);
		if (cudaResultReady && m_pPostProcessingObject)
		{
		/*	::SynchronizeResourceTransition(commandList, pSharedSource,
				D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);*/
			commandList->SetDescriptorHeaps(1, m_pDescriptorHeap->m_pd3dCbvSrvDescriptorHeap.GetAddressOf());
			commandList->SetGraphicsRootSignature(m_pd3dRootSignature.Get());
			m_pPostProcessingObject->Render(commandList, camera);
			//::SynchronizeResourceTransition(commandList, pSharedSource,
			//	D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON);
		}
	}

	else
	{
		::SynchronizeResourceTransition(commandList, m_pCudaRenderTexture->GetResource(0).Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
		commandList->OMSetRenderTargets(renderTargetCount, rtvHandles, TRUE, dsvHandle);
		m_pPostProcessingObject->Render(commandList, camera);
	}
	CScene::OnPostRender(commandList, commandQueue, renderTargetCount,
		rtvHandles, dsvHandle, camera);



}

bool CMainScene::IsCudaPostProcessEnabled() const
{
	return m_eRenderingMethod == ERenderingMethod::Cuda && m_pCudaRenderTexture &&
		m_CudaInterop.IsInitialized() && m_pd3dCommandQueue &&
		m_pd3dCudaPostCommandAllocator;
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
			// 단일 enum 상태를 사용하여 세 가지 렌더링 방식 중 하나만 활성화한다.
			if (ImGui::RadioButton("Default", m_eRenderingMethod == ERenderingMethod::Default))
			{
				m_eRenderingMethod = ERenderingMethod::Default;
			}

			if (ImGui::RadioButton("Compute Shader", m_eRenderingMethod == ERenderingMethod::ComputeShader))
			{
				m_eRenderingMethod = ERenderingMethod::ComputeShader;
			}

			if (ImGui::RadioButton("CUDA", m_eRenderingMethod == ERenderingMethod::Cuda))
			{
				m_eRenderingMethod = ERenderingMethod::Cuda;
			}
		}

		if (ImGui::CollapsingHeader("Blur Settings", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderInt("Blur", &m_nBlurSet, 0, 20);
		}
	}
	ImGui::End();
}
void CMainScene::OnEnter() { std::cout << "MainScene OnEnter" << std::endl; }
