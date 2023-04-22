#pragma once
#include <GA/Texture2D.h>
#include <d3d11.h>

namespace GA
{
	namespace DX11
	{
		class InterfaceDX11;

		class Texture2DDX11 : public Texture2D
		{
			ID3D11Texture2D* m_pInternalTexture;
			ID3D11ShaderResourceView* m_pSRV;
		public:
			Texture2DDX11(GA::DX11::InterfaceDX11* pGAInterface, const GA::Texture2D::Params& params);
			~Texture2DDX11();

			// Inherited via Texture2D

			ID3D11ShaderResourceView* GetSRV() const;
			virtual std::any GetInternal() override;
			virtual void* Map() override;
			virtual void Unmap() override;
		};

		const GA::DX11::Texture2DDX11* QuickCast(const GA::Texture2D* i);
		const GA::DX11::Texture2DDX11* SafeCast(const GA::Texture2D* i);
	}
}