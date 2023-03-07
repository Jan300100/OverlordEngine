#pragma once

#include "RenderTarget.h"
#include "Renderer/IRenderer.h"
#include <d3d11.h>

class DX11Renderer : public IRenderer
{
public:
	DX11Renderer();
	virtual ~DX11Renderer();

	virtual void Destroy() override;
	virtual void Initialize() override;

	void ClearBackBuffer() override;
	void SetRenderTarget(RenderTarget* renderTarget) override;
	RenderTarget* GetRenderTarget() const override;
	void ResetViewPort() override;

	virtual void Present() override;
public:
	// DX11
	ID3D11Device* GetDevice() override;
	ID3D11DeviceContext* GetDeviceContext() override;
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