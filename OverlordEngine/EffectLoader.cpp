#include "stdafx.h"
#include "EffectLoader.h"
//
#include <GA/DX11/InterfaceDX11.h>

std::shared_ptr<ID3DX11Effect> EffectLoader::LoadContent(const std::wstring& assetFile)
{
	PIX_PROFILE();

	HRESULT hr;
	ID3D10Blob* pErrorBlob = nullptr;
	ID3DX11Effect* pEffect;
	
	DWORD shaderFlags = 0;
#if defined( _DEBUG )
    shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	hr = D3DX11CompileEffectFromFile(assetFile.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		shaderFlags,
		0,
		GA::DX11::SafeCast(m_pGAInterface)->GetDevice(),
		&pEffect,
		&pErrorBlob);

	if(FAILED(hr))
	{
		if(pErrorBlob!=nullptr)
		{
			char *errors = (char*)pErrorBlob->GetBufferPointer();
 
		 std::wstringstream ss;
			for(unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++)
				ss<<errors[i];
 
			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;

			Logger::LogError(ss.str());
		}
		else
		{
		 std::wstringstream ss;
			ss<<"EffectLoader: Failed to CreateEffectFromFile!\nPath: ";
			ss<<assetFile;
			Logger::LogHResult(hr, ss.str());
			return nullptr;
		}
	}

	return std::shared_ptr<ID3DX11Effect>{pEffect, [](ID3DX11Effect* pEffect) {SafeRelease(pEffect); }};
}
