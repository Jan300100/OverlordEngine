#pragma once

namespace GA
{
	class Renderer
	{
	public:
		Renderer() = default;
		virtual ~Renderer() = default;
	public:
		virtual void Initialize() = 0;
		virtual void Destroy() = 0;

		virtual void ClearBackBuffer() = 0;
		virtual void SetRenderTarget(RenderTarget* renderTarget) = 0;
		virtual RenderTarget* GetRenderTarget() const = 0;
		virtual void ResetViewPort() = 0;
		virtual void Present() = 0;

		// TODO: remove TEMPORARY DX11, to avoid cast everywhere
		virtual ID3D11Device* GetDevice() = 0;
		virtual ID3D11DeviceContext* GetDeviceContext() = 0;

	};
}