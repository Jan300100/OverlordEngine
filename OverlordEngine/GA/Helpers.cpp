#include "stdafx.h"
#include "Helpers.h"

namespace GA
{
	namespace HELP
	{
		GA::Texture2D::Format DXGIFormatToGAFormat(DXGI_FORMAT format)
		{
			GA::Texture2D::Format result;

			switch (format)
			{
			case DXGI_FORMAT_B8G8R8A8_UNORM:
				result = GA::Texture2D::Format::B8G8R8A8_UNORM;
				break;
			case DXGI_FORMAT_R8G8B8A8_UNORM:
				result = GA::Texture2D::Format::R8G8B8A8_UNORM;
				break;
			default:
				Logger::LogError(L"DXGI format not supported by GA");
				result = GA::Texture2D::Format::R8G8B8A8_UNORM;
				break;
			}

			return result;
		}
	}
}
