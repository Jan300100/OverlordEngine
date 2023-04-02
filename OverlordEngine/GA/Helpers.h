#pragma once
#include <GA/Texture2D.h>
#include <dxgiformat.h>

namespace GA
{
	namespace HELP
	{
		GA::Texture2D::Format DXGIFormatToGAFormat(DXGI_FORMAT format);
	}
}