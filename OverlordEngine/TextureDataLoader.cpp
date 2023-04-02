#include "stdafx.h"
#include "TextureDataLoader.h"
#include <GA/Resource.h>
#include <GA/Texture2D.h>

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

std::unique_ptr<DirectX::ScratchImage> TextureDataLoader::GetInitialData(const std::wstring& assetFile)
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

	image = std::move(GenerateMips(image.get()));

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

TextureData* TextureDataLoader::LoadContent(const std::wstring& assetFile)
{
	PIX_PROFILE();

	ID3D11Resource* pTexture;
	ID3D11ShaderResourceView* pShaderResourceView;

	std::unique_ptr<DirectX::ScratchImage> initialData = GetInitialData(assetFile);
	Logger::LogFormat(LogLevel::Info, L"Format: %d", initialData->GetMetadata().format);

	GA::Texture2D::Params params;
	params.lifeTime = GA::Resource::LifeTime::Permanent;
	params.cpuUpdateFreq = GA::Resource::CPUUpdateFrequency::Never;
	
	/*HRESULT hr = CreateTexture(m_pDevice, initialData->GetImages(), initialData->GetImageCount(), initialData->GetMetadata(), &pTexture);
	if (Logger::LogHResult(hr, L"TextureDataLoader::LoadContent() > CreateTexture Failed!"))
	{
		return nullptr;
	}*/

	HRESULT hr = CreateShaderResourceView(m_pDevice, initialData->GetImages(), initialData->GetImageCount(), initialData->GetMetadata(), &pShaderResourceView);
	if(Logger::LogHResult(hr, L"TextureDataLoader::LoadContent() > CreateShaderResourceView Failed!"))
	{
		return nullptr;
	}

	pShaderResourceView->GetResource(&pTexture);
	return new TextureData(pTexture, pShaderResourceView);
}

void TextureDataLoader::Destroy(TextureData* objToDestroy)
{
	SafeDelete(objToDestroy);
}
