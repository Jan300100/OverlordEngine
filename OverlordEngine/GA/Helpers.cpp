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

		DXGI_FORMAT GAFormatToDXGIFormat(GA::Texture2D::Format format)
		{
			DXGI_FORMAT result = DXGI_FORMAT_R8G8B8A8_UNORM;

			switch (format)
			{
			case Texture2D::Format::B8G8R8A8_UNORM:
				result = DXGI_FORMAT_B8G8R8A8_UNORM;
				break;
			case Texture2D::Format::R8G8B8A8_UNORM:
				result = DXGI_FORMAT_R8G8B8A8_UNORM;
				break;
			default:
				Logger::LogError(L"not implemented");
				break;
			}

			return result;
		}

		size_t GetFormatSize(GA::Texture2D::Format format)
		{
			size_t size = 0;
			switch (format)
			{
			case GA::Texture2D::Format::R8G8B8A8_UNORM:
			case GA::Texture2D::Format::B8G8R8A8_UNORM:
				size = 4;
				break;
			default:
				Logger::LogError(L"not implemented");
				break;
			}
			return size;
		}
	}
}
