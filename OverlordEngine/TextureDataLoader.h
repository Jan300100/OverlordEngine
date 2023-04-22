#pragma once
#include "ContentLoader.h"

namespace GA
{
	class Texture2D;
}

class TextureDataLoader : public ContentLoader<GA::Texture2D>
{
public:
	TextureDataLoader() = default;
	virtual ~TextureDataLoader() = default;

	TextureDataLoader(const TextureDataLoader& other) = delete;
	TextureDataLoader(TextureDataLoader&& other) noexcept = delete;
	TextureDataLoader& operator=(const TextureDataLoader& other) = delete;
	TextureDataLoader& operator=(TextureDataLoader&& other) noexcept = delete;

protected:
	std::unique_ptr<DirectX::ScratchImage> GenerateMips(DirectX::ScratchImage* imageDataWithoutMips);
	std::unique_ptr<DirectX::ScratchImage> GetInitialData(const std::wstring& assetFile, bool generateMips);
	std::wstring GetExtension(const std::wstring& assetFile);

	std::shared_ptr<GA::Texture2D> LoadContent(const std::wstring& assetFile) override;

};

