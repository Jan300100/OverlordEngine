#include "stdafx.h"

#include "GA/DX11/InterfaceDX11.h"
#include "GA/DX11/BufferDX11.h"

#include "OverlordGame.h"
#include "RenderTarget.h"

namespace GA
{
	namespace DX11
	{
		void InterfaceDX11::Destroy()
		{
			if (m_Initialized)
			{
				m_Initialized = false;
				//DirectX Cleanup
				SafeDelete(m_pDefaultRenderTarget);
				SafeRelease(m_pDxgiFactory);
				SafeRelease(m_pSwapchain);


				if (m_pDeviceContext)
				{
					m_pDeviceContext->ClearState();
					m_pDeviceContext->Flush();
					SafeRelease(m_pDeviceContext);
				}

				SafeRelease(m_pDevice);

				SafeRelease(m_pAdapter);
				SafeRelease(m_pOutput);
			}
		}

		void InterfaceDX11::Initialize()
		{
			if (!m_Initialized)
			{
				m_Initialized = true;

				HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&m_pDxgiFactory));
				if (FAILED(hr)) return;

				hr = m_pDxgiFactory->EnumAdapters(0, &m_pAdapter);
				if (FAILED(hr)) return;

				IDXGIOutput* tempOutput;
				hr = m_pAdapter->EnumOutputs(0, &tempOutput);
				if (FAILED(hr)) return;

				hr = tempOutput->QueryInterface(__uuidof(IDXGIOutput), reinterpret_cast<void**>(&m_pOutput));
				if (FAILED(hr)) return;
				SafeRelease(tempOutput);

				//Create DX11 Device & Context
				UINT createDeviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
				createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

#pragma warning(push)
#pragma warning(disable: 26812)
				D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
				hr = D3D11CreateDevice(m_pAdapter,
					D3D_DRIVER_TYPE_UNKNOWN,
					nullptr,
					createDeviceFlags,
					nullptr, 0,
					D3D11_SDK_VERSION,
					&m_pDevice,
					&featureLevel,
					&m_pDeviceContext);
#pragma warning(pop)

				if (SUCCEEDED(hr))
				{
					if (featureLevel < D3D_FEATURE_LEVEL_10_0)
					{
						Logger::LogHResult(-1, L"Feature level 10.0+ not supported on this device!");
						exit(-1);
					}
					if (featureLevel < D3D_FEATURE_LEVEL_11_0)
					{
						Logger::LogWarning(L"Feature level 10.1, some DirectX11 specific features won't be available on this device!");
					}

					//Create Swapchain descriptor
					DXGI_SWAP_CHAIN_DESC swapChainDesc;
					ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
					swapChainDesc.BufferDesc.Width = OverlordGame::GetGameSettings().Window.Width;
					swapChainDesc.BufferDesc.Height = OverlordGame::GetGameSettings().Window.Height;
					swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
					swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
					swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
					swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
					swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
					swapChainDesc.SampleDesc.Count = 1;
					swapChainDesc.SampleDesc.Quality = 0;
					// Update PP
					swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
					swapChainDesc.BufferCount = 1;
					swapChainDesc.OutputWindow = OverlordGame::GetGameSettings().Window.WindowHandle;
					swapChainDesc.Windowed = true;
					swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
					swapChainDesc.Flags = 0;

					hr = m_pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapchain);
					if (SUCCEEDED(hr))
					{

						//Create the default rendertarget.
						m_pDefaultRenderTarget = new RenderTarget(m_pDevice);

						ID3D11Texture2D* pBackbuffer = nullptr;
						hr = m_pSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackbuffer));
						if (FAILED(hr))
						{
							return;
						}

						RENDERTARGET_DESC rtDesc;
						rtDesc.pColor = pBackbuffer;
						rtDesc.EnableDepthSRV = true;
						hr = m_pDefaultRenderTarget->Create(rtDesc);
						if (FAILED(hr))
						{
							return;
						}

						//Set Default Rendertarget 
						SetRenderTarget(nullptr);

						Logger::LogFixMe(L"Viewport ownership, overlordgame");
						m_Viewport.Width = static_cast<FLOAT>(OverlordGame::GetGameSettings().Window.Width);
						m_Viewport.Height = static_cast<FLOAT>(OverlordGame::GetGameSettings().Window.Height);
						m_Viewport.TopLeftX = 0;
						m_Viewport.TopLeftY = 0;
						m_Viewport.MinDepth = 0.0f;
						m_Viewport.MaxDepth = 1.0f;
						m_pDeviceContext->RSSetViewports(1, &m_Viewport);
					}
				}
			}
		}

		std::unique_ptr<GA::Buffer> InterfaceDX11::CreateBuffer(const GA::Buffer::Params& params)
		{
			return std::make_unique<GA::DX11::BufferDX11>(this, params);
		}

		void InterfaceDX11::ClearBackBuffer()
		{
			//Clear Backbuffer
			m_pDeviceContext->ClearRenderTargetView(m_pCurrentRenderTarget->GetRenderTargetView(), reinterpret_cast<const float*>(&DirectX::Colors::CornflowerBlue));
			m_pDeviceContext->ClearDepthStencilView(m_pCurrentRenderTarget->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		}

		void InterfaceDX11::SetRenderTarget(RenderTarget* renderTarget)
		{
			PIX_PROFILE();

			if (renderTarget == nullptr)
				renderTarget = m_pDefaultRenderTarget;

			auto rtView = renderTarget->GetRenderTargetView();
			m_pDeviceContext->OMSetRenderTargets(1, &rtView, renderTarget->GetDepthStencilView());

			m_pCurrentRenderTarget = renderTarget;
		}

		RenderTarget* InterfaceDX11::GetRenderTarget() const
		{
			return m_pCurrentRenderTarget;
		}

		void InterfaceDX11::ResetViewPort()
		{
			m_pDeviceContext->RSSetViewports(1, &m_Viewport);
		}

		void InterfaceDX11::Present()
		{
			PIX_PROFILE();
			//Present Backbuffer
			m_pSwapchain->Present(0, 0);
		}

		ID3D11Device* InterfaceDX11::GetDevice()
		{
			return m_pDevice;
		}

		ID3D11DeviceContext* InterfaceDX11::GetDeviceContext()
		{
			return m_pDeviceContext;
		}

		InterfaceDX11::InterfaceDX11()
			: GA::Interface{}
		{
		}

		InterfaceDX11::~InterfaceDX11()
		{
			Destroy();
		}

		GA::DX11::InterfaceDX11* QuickCast(GA::Interface* i)
		{
			return static_cast<GA::DX11::InterfaceDX11*>(i);
		}

		GA::DX11::InterfaceDX11* SafeCast(GA::Interface* i)
		{
			return dynamic_cast<GA::DX11::InterfaceDX11*>(i);
		}
}
}