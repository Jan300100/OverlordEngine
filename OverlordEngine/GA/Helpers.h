#pragma once
#include <GA/Texture2D.h>
#include <dxgiformat.h>

namespace GA
{
	namespace HELP
	{
		GA::Texture2D::Format DXGIFormatToGAFormat(DXGI_FORMAT format);
		DXGI_FORMAT GAFormatToDXGIFormat(GA::Texture2D::Format format);
		size_t GetFormatSize(GA::Texture2D::Format format);
	}
}