#pragma once
#include "ContentLoader.h"
#include "SpriteFont.h"

class SpriteFontLoader : public ContentLoader<SpriteFont>
{
public:
	SpriteFontLoader() = default;
	virtual ~SpriteFontLoader() = default;

	SpriteFontLoader(const SpriteFontLoader& other) = delete;
	SpriteFontLoader(SpriteFontLoader&& other) noexcept = delete;
	SpriteFontLoader& operator=(const SpriteFontLoader& other) = delete;
	SpriteFontLoader& operator=(SpriteFontLoader&& other) noexcept = delete;

protected:
	std::shared_ptr<SpriteFont> LoadContent(const std::wstring& assetFile) override;
};

