#pragma once
#include "ContentLoader.h"
#include "TextureData.h"

class TextureDataLoader : public ContentLoader<TextureData>
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
	std::unique_ptr<DirectX::ScratchImage> GetInitialData(const std::wstring& assetFile);
	std::wstring GetExtension(const std::wstring& assetFile);

	TextureData* LoadContent(const std::wstring& assetFile) override;
	void Destroy(TextureData* objToDestroy) override;

};

