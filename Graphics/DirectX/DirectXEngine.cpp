#include "glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "glfw3native.h"
#include "DirectXEngine.h"
#include "DirectXTex.h"

#include <glm.hpp>

#include "../Camera.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dx11effect.h>

#include "DirectXShaderReader.h"

leap::graphics::DirectXEngine::DirectXEngine(GLFWwindow* pWindow) : m_pWindow(pWindow)
{

}

leap::graphics::DirectXEngine::~DirectXEngine()
{
	if (m_pRenderTargetView)
	{
		m_pRenderTargetView->Release();
	}
	if (m_pRenderTargetBuffer)
	{
		m_pRenderTargetBuffer->Release();
	}
	if (m_pDepthStencilView)
	{
		m_pDepthStencilView->Release();
	}
	if (m_pDepthStencilBuffer)
	{
		m_pDepthStencilBuffer->Release();
	}
	if (m_pSwapChain)
	{
		m_pSwapChain->Release();
	}
	if (m_pDeviceContext)
	{
		m_pDeviceContext->ClearState();
		m_pDeviceContext->Flush();
		m_pDeviceContext->Release();
	}
	if (m_pDevice)
	{
		m_pDevice->Release();
	}
}

void leap::graphics::DirectXEngine::Initialize()
{
	/// Create device & Device context
	UINT createDeviceFlags = 0;
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;

#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	IDXGIFactory1* pFactory = nullptr;
	HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pFactory));
	HRESULT result{};

	if (SUCCEEDED(hr))
	{
		IDXGIAdapter1* pAdapter = nullptr;

		hr = pFactory->EnumAdapters1(0, &pAdapter);
		if (SUCCEEDED(hr))
		{
			result = D3D11CreateDevice(pAdapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, createDeviceFlags, &featureLevel, 1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);

			if (FAILED(result)) return;

			pAdapter->Release();
		}

		pFactory->Release();
	}

	IDXGIFactory1* pDxgiFactory{ nullptr };
	result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));
	if (FAILED(result)) return;

	/// Create swapchain

	// Get the window size to use for the swap chain
	int width, height;
	glfwGetWindowSize(m_pWindow, &width, &height);

	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;
	swapChainDesc.OutputWindow = glfwGetWin32Window(m_pWindow);

	result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
	if (FAILED(result)) return;

	/// Create Depthstencil & Depthstencil view
	//Depth
	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	//View
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
	if (FAILED(result)) return;

	result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
	if (FAILED(result)) return;

	/// Create RenderTarget and RenderTargetView
	//Render target
	result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
	if (FAILED(result)) return;

	//View
	result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
	if (FAILED(result)) return;

	/// Bind views
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	/// Set viewport
	D3D11_VIEWPORT viewport{};
	viewport.Width = static_cast<FLOAT>(width);
	viewport.Height = static_cast<FLOAT>(height);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
	m_pDeviceContext->RSSetViewports(1, &viewport);

	m_IsInitialized = true;
}

void leap::graphics::DirectXEngine::Draw()
{
	if (!m_IsInitialized) return;
	if (!m_pCamera) return;

	const glm::vec4& clearColor = m_pCamera->GetColor();
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	DirectXMaterial::SetViewProjectionMatrix(m_pCamera->GetProjectionMatrix() * m_pCamera->GetViewMatrix());

	for (const auto& pMeshRenderer : m_pRenderers)
	{
		pMeshRenderer->Draw();
	}

	m_pSwapChain->Present(0, 0);
}

leap::graphics::IMeshRenderer* leap::graphics::DirectXEngine::CreateMeshRenderer()
{
	m_pRenderers.push_back(std::make_unique<DirectXMeshRenderer>(m_pDevice, m_pDeviceContext));
	return m_pRenderers[m_pRenderers.size() - 1].get();
}

void leap::graphics::DirectXEngine::RemoveMeshRenderer(IMeshRenderer* pMeshRenderer)
{
	m_pRenderers.erase(std::remove_if(begin(m_pRenderers), end(m_pRenderers), [pMeshRenderer](const auto& pRenderer) { return pMeshRenderer == pRenderer.get(); }));
}

leap::graphics::IMaterial* leap::graphics::DirectXEngine::CreateMaterial(std::unique_ptr<Shader, ShaderDelete> pShader)
{
	const DirectXShader shader{ DirectXShaderReader::GetShaderData(std::move(pShader)) };
	m_pMaterials.push_back(std::make_unique<DirectXMaterial>(m_pDevice, shader.path, shader.vertexDataFunction));
	return m_pMaterials[m_pMaterials.size() - 1].get();
}