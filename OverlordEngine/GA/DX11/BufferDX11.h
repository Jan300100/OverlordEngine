#pragma once
#include <GA/Buffer.h>
#include <d3d11.h>

namespace GA
{
	namespace DX11
	{
		class InterfaceDX11;

		class BufferDX11 : public Buffer
		{
			ID3D11Buffer* m_pInternalBuffer;
		public:
			BufferDX11(GA::DX11::InterfaceDX11* i, const GA::Buffer::Params& params);
			~BufferDX11();

			virtual std::any GetInternal() override;
			virtual void* Map() override;
			virtual void Unmap() override;
		};
	}
}