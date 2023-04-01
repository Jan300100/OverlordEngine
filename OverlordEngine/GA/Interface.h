#pragma once
#include <memory>

#include "GA/Buffer.h"

namespace GA
{
	class Interface
	{
	public:
		Interface() = default;
		virtual ~Interface() = default;
	public:
		virtual void Initialize() = 0;
		virtual void Destroy() = 0;

		// creation interface
		virtual std::unique_ptr<GA::Buffer> CreateBuffer(const GA::Buffer::Params& params) = 0;

		// potential temporary interface
		virtual void ClearBackBuffer() = 0;
		virtual void SetRenderTarget(RenderTarget* renderTarget) = 0;
		virtual RenderTarget* GetRenderTarget() const = 0;
		virtual void ResetViewPort() = 0;
		virtual void Present() = 0;
	};
}