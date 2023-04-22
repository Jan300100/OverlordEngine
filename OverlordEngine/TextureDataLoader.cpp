#include "stdafx.h"
#include "TextureDataLoader.h"
#include <GA/Resource.h>
#include <GA/Texture2D.h>
#include <GA/Helpers.h>

using namespace DirectX;

std::unique_ptr<DirectX::ScratchImage> TextureDataLoader::GenerateMips(DirectX::ScratchImage* imageDataWithoutMips)
{
	std::unique_ptr<DirectX::ScratchImage> imageMips = std::make_unique<DirectX::ScratchImage>();

	// probably need to revisit these flags
	int flags = TEX_FILTER_POINT | TEX_FILTER_MIRROR | TEX_FILTER_FORCE_NON_WIC | TEX_FILTER_SEPARATE_ALPHA;

	size_t numMipsToGenerate = 5;
#if defined(_M_IX86)
	HRESULT hr = GenerateMipMaps(image->GetImages(), image->GetImageCount(), image->GetMetadata(), flags, numMipsToGenerate, *imageMips);
#elif defined(_M_X64)
	HRESULT hr = GenerateMipMaps(imageDataWithoutMips->GetImages(), imageDataWithoutMips->GetImageCount(), imageDataWithoutMips->GetMetadata(), static_cast<TEX_FILTER_FLAGS>(flags), numMipsToGenerate, *imageMips);
#endif
	if (Logger::LogHResult(hr, L"TextureDataLoader::LoadContent() > GenerateMipMaps Failed!"))
	{
		return nullptr;
	}

	return imageMips;
}

std::unique_ptr<DirectX::ScratchImage> TextureDataLoader::GetInitialData(const std::wstring& assetFile, bool generateMips)
{
	// even though it uses DirectXTex Library, seems pretty cross platform to me.

	DirectX::TexMetadata info;
	std::unique_ptr<DirectX::ScratchImage> image = std::make_unique<DirectX::ScratchImage>();

	std::wstring extension = GetExtension(assetFile);
	if (extension == L"dds") //DDS Loader
	{
		HRESULT hr = LoadFromDDSFile(assetFile.c_str(), DirectX::DDS_FLAGS_NONE, &info, *image);
		if (Logger::LogHResult(hr, L"TextureDataLoader::LoadContent() > LoadFromDDsFile Failed!"))
			return nullptr;
	}
	else if (extension == L"tga") //TGA Loader
	{
		HRESULT hr = LoadFromTGAFile(assetFile.c_str(), &info, *image);
		if (Logger::LogHResult(hr, L"TextureDataLoader::LoadContent() > LoadFromTGAFile Failed!"))
			return nullptr;
	}
	else //WIC Loader
	{
		HRESULT hr = LoadFromWICFile(assetFile.c_str(), DirectX::WIC_FLAGS_NONE, &info, *image);
		if (Logger::LogHResult(hr, L"TextureDataLoader::LoadContent() > LoadFromWICFile Failed!"))
			return nullptr;
	}

	if (generateMips)
	{
		image = std::move(GenerateMips(image.get()));
	}

	return image;
}

std::wstring TextureDataLoader::GetExtension(const std::wstring& assetFile)
{
	if (assetFile.find_last_of(L".") != std::wstring::npos)
	{
		return assetFile.substr(assetFile.find_last_of(L".") + 1);
	}
	else
	{
		Logger::LogFormat(LogLevel::Error, L"TextureDataLoader::LoadContent() > Invalid File Extensions!\nPath: %s", assetFile.c_str());
		return {};
	}
}

std::shared_ptr<GA::Texture2D> TextureDataLoader::LoadContent(const std::wstring& assetFile)
{
	PIX_PROFILE();

	std::unique_ptr<DirectX::ScratchImage> initialData = GetInitialData(assetFile, true);

	GA::Texture2D::Params params;

	params.lifeTime = GA::Resource::LifeTime::Permanent;
	params.cpuUpdateFreq = GA::Resource::CPUUpdateFrequency::Never;

	params.format = GA::HELP::DXGIFormatToGAFormat(initialData->GetMetadata().format);

	params.height = static_cast<uint32_t>(initialData->GetMetadata().height);
	params.width = static_cast<uint32_t>(initialData->GetMetadata().width);
	
	//subresourceData
	params.numSubresources = static_cast<uint32_t>(initialData->GetImageCount());
	std::vector<void*> subResourceData{ params.numSubresources };
	for (uint32_t i = 0; i < params.numSubresources; i++)
	{
		subResourceData[i] = initialData->GetImages()[i].pixels;
	}
	params.subresourceData = subResourceData.data();
	return m_pGAInterface->CreateTexture2D(params);
}