#pragma once
#include "GA/Interface.h"
#include <d3d11.h>

class RenderTarget;

namespace GA
{
	namespace DX11
	{

		class InterfaceDX11 : public GA::Interface
		{
		public:
			InterfaceDX11();
			virtual ~InterfaceDX11();

			virtual void Destroy() override;
			virtual void Initialize() override;

			void ClearBackBuffer() override;
			void SetRenderTarget(RenderTarget* renderTarget) override;
			RenderTarget* GetRenderTarget() const override;
			void ResetViewPort() override;

			virtual void Present() override;
		public:
			// DX11
			ID3D11Device* GetDevice();
			ID3D11DeviceContext* GetDeviceContext();
		private:
			bool m_Initialized = false;

			IDXGIAdapter* m_pAdapter;
			IDXGIOutput* m_pOutput;

			ID3D11Device* m_pDevice = nullptr;
			ID3D11DeviceContext* m_pDeviceContext = nullptr;
			IDXGISwapChain* m_pSwapchain = nullptr;
			IDXGIFactory* m_pDxgiFactory = nullptr;
			D3D11_VIEWPORT m_Viewport{};

			RenderTarget* m_pDefaultRenderTarget, * m_pCurrentRenderTarget = nullptr;
		};

		GA::DX11::InterfaceDX11* QuickCast(GA::Interface* i);
		GA::DX11::InterfaceDX11* SafeCast(GA::Interface* i);
	}
}