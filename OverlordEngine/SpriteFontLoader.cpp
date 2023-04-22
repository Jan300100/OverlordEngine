#include "stdafx.h"
#include "SpriteFontLoader.h"
#include "BinaryReader.h"
#include "ContentManager.h"

std::shared_ptr<SpriteFont> SpriteFontLoader::LoadContent(const std::wstring& assetFile)
{
	PIX_PROFILE();

	auto pBinReader = new BinaryReader();
	pBinReader->Open(assetFile);

	if (!pBinReader->Exists())
	{
		Logger::LogFormat(LogLevel::Error, L"SpriteFontLoader::LoadContent > Failed to read the assetFile!\nPath: \'%s\'", assetFile.c_str());
		return nullptr;
	}

	//See BMFont Documentation for Binary Layout

	//Parse the Identification bytes (B,M,F)
	//If Identification bytes doesn't match B|M|F,
	//Log Error (SpriteFontLoader::LoadContent > Not a valid .fnt font) &
	//return nullptr
	//...
	std::string bmf{};
	for (int i = 0; i < 3; i++) bmf += pBinReader->Read<char>();
	if (bmf != "BMF")
	{
		Logger::LogError(L"SpriteFontLoader::LoadContent > Not a valid .fnt font");
		return nullptr;
	}

	//Parse the version (version 3 required)
	//If version is < 3,
	//Log Error (SpriteFontLoader::LoadContent > Only .fnt version 3 is supported)
	//return nullptr
	//...
	char version{ pBinReader->Read<char>() };
	if (version < 3)
	{
		Logger::LogError(L"SpriteFontLoader::LoadContent > Only .fnt version 3 is supported");
		return nullptr;
	}

	//Valid .fnt file
	auto pSpriteFont = new SpriteFont();
	//SpriteFontLoader is a friend class of SpriteFont
	//That means you have access to its privates (pSpriteFont->m_FontName = ... is valid)

	//**********
	// BLOCK 0 *
	//**********
	//Retrieve the blockId and blockSize
	//Retrieve the FontSize (will be -25, BMF bug) [SpriteFont::m_FontSize]
	//Move the binreader to the start of the FontName [BinaryReader::MoveBufferPosition(...) or you can set its position using BinaryReader::SetBufferPosition(...))
	//Retrieve the FontName [SpriteFont::m_FontName]
	//...

	//char blockId
	pBinReader->Read<char>();
	int blockSize = pBinReader->Read<int>();
	//fontsize and name
	pSpriteFont->m_FontSize = pBinReader->Read<short>();
	pBinReader->MoveBufferPosition(12); //14 - 2 (from reading the fontSize)
	pSpriteFont->m_FontName = pBinReader->ReadNullString();

	// go to next block


	//**********
	// BLOCK 1 *
	//**********
	//Retrieve the blockId and blockSize
	//Retrieve Texture Width & Height [SpriteFont::m_TextureWidth/m_TextureHeight]
	//Retrieve PageCount
	//> if pagecount > 1
	//> Log Error (SpriteFontLoader::LoadContent > SpriteFont (.fnt): Only one texture per font allowed)
	//Advance to Block2 (Move Reader)
	//...
	
	//char blockId
	pBinReader->Read<char>();
	blockSize = pBinReader->Read<int>();
	//texture width and height
	pBinReader->MoveBufferPosition(4);
	pSpriteFont->m_TextureWidth = pBinReader->Read<short>();
	pSpriteFont->m_TextureHeight = pBinReader->Read<short>();
	int pageCount = pBinReader->Read<short>();
	if (pageCount > 1)
	{
		Logger::LogError(L"SpriteFontLoader::LoadContent > SpriteFont(.fnt) : Only one texture per font allowed");
		return nullptr;
	}

	pBinReader->MoveBufferPosition(5);

	//**********
	// BLOCK 2 *
	//**********
	//Retrieve the blockId and blockSize
	//Retrieve the PageName (store Local)
	//	> If PageName is empty
	//	> Log Error (SpriteFontLoader::LoadContent > SpriteFont (.fnt): Invalid Font Sprite [Empty])
	//>Retrieve texture filepath from the assetFile path
	//> (ex. c:/Example/somefont.fnt => c:/Example/) [Have a look at: wstring::rfind()]
	//>Use path and PageName to load the texture using the ContentManager [SpriteFont::m_pTexture]
	//> (ex. c:/Example/ + 'PageName' => c:/Example/somefont_0.png)
	//...

	pBinReader->Read<char>();	//char blockId
	blockSize = pBinReader->Read<int>();

	std::wstring pageName = pBinReader->ReadNullString();
	if (pageName.empty())
	{
		Logger::LogError(L"SpriteFontLoader::LoadContent > SpriteFont (.fnt): Invalid Font Sprite [Empty]");
		return nullptr;
	}
	std::wstring texturePath = assetFile;
	while (texturePath.back() != '/' && texturePath.back() != '\\')
	{
		texturePath.pop_back();
	}
	texturePath += pageName;
	pSpriteFont->m_pTexture = ContentManager::Load<GA::Texture2D>(texturePath).get();


	//**********
	// BLOCK 3 *
	//**********
	//Retrieve the blockId and blockSize
	//Retrieve Character Count (see documentation)
	//Retrieve Every Character, For every Character:
	//> Retrieve CharacterId (store Local) and cast to a 'wchar_t'
	//> Check if CharacterId is valid (SpriteFont::IsCharValid), Log Warning and advance to next character if not valid
	//> Retrieve the corresponding FontMetric (SpriteFont::GetMetric) [REFERENCE!!!]
	//> Set IsValid to true [FontMetric::IsValid]
	//> Set Character (CharacterId) [FontMetric::Character]
	//> Retrieve Xposition (store Local)
	//> Retrieve Yposition (store Local)
	//> Retrieve & Set Width [FontMetric::Width]
	//> Retrieve & Set Height [FontMetric::Height]
	//> Retrieve & Set OffsetX [FontMetric::OffsetX]
	//> Retrieve & Set OffsetY [FontMetric::OffsetY]
	//> Retrieve & Set AdvanceX [FontMetric::AdvanceX]
	//> Retrieve & Set Page [FontMetric::Page]
	//> Retrieve Channel (BITFIELD!!!) 
	//	> See documentation for BitField meaning [FontMetrix::Channel]
	//> Calculate Texture Coordinates using Xposition, Yposition, TextureWidth & TextureHeight [FontMetric::TexCoord]
	//...

	pBinReader->Read<char>();	//char blockId
	blockSize = pBinReader->Read<int>();

	//
	int characterCount = blockSize / 20;
	for (int i = 0; i < characterCount; i++)
	{
		wchar_t charId = wchar_t(pBinReader->Read<unsigned int>());
		
		if (!pSpriteFont->IsCharValid(charId))
		{
			Logger::LogWarning(L"SpriteFontLoader::LoadContent > SpriteFont (.fnt): contains invalid characters");
			pBinReader->MoveBufferPosition(16);
			continue;
		}
		FontMetric& metric = pSpriteFont->GetMetric(charId);
		metric.IsValid = true;
		metric.Character = charId;

		short xPos = pBinReader->Read<short>();
		short yPos = pBinReader->Read<short>();
		metric.Width = pBinReader->Read<short>();
		metric.Height = pBinReader->Read<short>();
		metric.OffsetX = pBinReader->Read<short>();
		metric.OffsetY = pBinReader->Read<short>();
		metric.AdvanceX = pBinReader->Read<short>();
		metric.Page = pBinReader->Read<char>();

		metric.Channel = pBinReader->Read<unsigned char>();

		switch (metric.Channel)
		{
		case 1:
		case 2:
		case 4:
			//convert BGR bitfield to our RGB:
			metric.Channel = unsigned char(-(int(metric.Channel >> 1) - 2));
			//1 => 2 //blue --> 1 -> 0 (shift) -> -2 (subtracting 2)-> 2 (flip sign) == BLUE
			//2 => 1 //green --> 2 -> 1 (shift) -> -1 (subtracting 2)-> 1 (flip sign) == GREEN
			//4 => 0 //red --> 4 -> 2 (shift) -> 0 (subtracting 2)-> 0 (flip sign) == RED
			break;
		case 8: //a real alfa-channel doesnt follow the rules
			metric.Channel = 3;
			break;
		}

		DirectX::XMFLOAT2 texCoord{};
		texCoord.x = xPos / (float)pSpriteFont->m_TextureWidth;
		texCoord.y = yPos / (float)pSpriteFont->m_TextureHeight;
		metric.TexCoord = texCoord;
	}


	//DONE :)

	delete pBinReader;
	return std::shared_ptr<SpriteFont>(pSpriteFont);
}
