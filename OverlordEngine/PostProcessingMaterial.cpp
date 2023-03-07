#include "stdafx.h"
#include "PostProcessingMaterial.h"
#include "RenderTarget.h"
#include "OverlordGame.h"
#include "ContentManager.h"

PostProcessingMaterial::PostProcessingMaterial(std::wstring effectFile, unsigned int renderIndex,
                                               std::wstring technique)
	: m_IsInitialized(false), 
	  m_pInputLayout(nullptr),
	  m_pInputLayoutSize(0),
	  m_effectFile(std::move(effectFile)),
	  m_InputLayoutID(0),
	  m_RenderIndex(renderIndex),
	  m_pRenderTarget(nullptr),
	  m_pVertexBuffer(nullptr),
	  m_pIndexBuffer(nullptr),
	  m_NumVertices(0),
	  m_NumIndices(0),
	  m_VertexBufferStride(0),
	  m_pEffect(nullptr),
	  m_pTechnique(nullptr),
	  m_TechniqueName(std::move(technique))
{
}

PostProcessingMaterial::~PostProcessingMaterial()
{
	//TODO: delete and/or release necessary objects and/or resources
	SafeDelete(m_pRenderTarget);
	SafeRelease(m_pInputLayout);
	SafeRelease(m_pVertexBuffer);
	SafeRelease(m_pIndexBuffer);
//	SafeRelease(m_pEffect);
//	SafeRelease(m_pTechnique);
}

void PostProcessingMaterial::Initialize(const GameContext& gameContext)
{
	PIX_PROFILE();

	UNREFERENCED_PARAMETER(gameContext);

	if (!m_IsInitialized)
	{
		//TODO: complete
		//1. LoadEffect (LoadEffect(...))
		LoadEffect(gameContext, m_effectFile);
		//2. CreateInputLaytout (CreateInputLayout(...))
		CreateIndexBuffer(gameContext);
		//   CreateVertexBuffer (CreateVertexBuffer(...)) > As a TriangleStrip (FullScreen Quad)
		CreateVertexBuffer(gameContext);
		//3. Create RenderTarget (m_pRenderTarget)
		//		Take a look at the class, figure out how to initialize/create a RenderTarget Object
		//		GameSettings > OverlordGame::GetGameSettings()
		RENDERTARGET_DESC desc{};
		desc.Height = OverlordGame::GetGameSettings().Window.Height;
		desc.Width = OverlordGame::GetGameSettings().Window.Width;
		desc.EnableColorSRV = true;
		desc.EnableDepthSRV = true;
		desc.GenerateMipMaps_Color = true;
		m_pRenderTarget = new RenderTarget{gameContext.pRenderer->GetDevice()};
		HRESULT hr = m_pRenderTarget->Create(desc);
		if (Logger::LogHResult(hr, L"PostProcessingMaterial::Initialize() - creating rendertarget")) return;

		m_IsInitialized = true;
	}
}

bool PostProcessingMaterial::LoadEffect(const GameContext& gameContext, const std::wstring& effectFile)
{
	PIX_PROFILE();

	UNREFERENCED_PARAMETER(gameContext);
	UNREFERENCED_PARAMETER(effectFile);

	//TODO: complete
	//Load Effect through ContentManager
	m_pEffect = ContentManager::Load<ID3DX11Effect>(effectFile);
	//Check if m_TechniqueName (default constructor parameter) is set

	if (m_TechniqueName != L"")
	{	
		// If SET > Use this Technique (+ check if valid)
		 std::string tName = std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(m_TechniqueName);
		 m_pTechnique = m_pEffect->GetTechniqueByName(tName.c_str());
		 if (!m_pTechnique->IsValid())
		 {
			 Logger::LogError(L"PostProcessingMaterial::LoadEffect() - technique does not exist\n");
		 }
	}
	// If !SET > Use Technique with index 0
	else
	{
		m_pTechnique = m_pEffect->GetTechniqueByIndex(0);
		if (!m_pTechnique->IsValid())
		{
			Logger::LogError(L"PostProcessingMaterial::LoadEffect() - no technique at index 0 found\n");
		}
	}
	//create input layout
	EffectHelper::BuildInputLayout(gameContext.pRenderer->GetDevice(), m_pTechnique, &m_pInputLayout,m_pInputLayoutDescriptions, m_pInputLayoutSize, m_InputLayoutID);

	//Call LoadEffectVariables
	LoadEffectVariables();

	return true;
}

void PostProcessingMaterial::Draw(const GameContext& gameContext,RenderTarget* pPrevRendertarget, RenderTarget* pOriginalRendertarget)
{
	PIX_PROFILE();

	//TODO: complete
	//1. Clear the object's RenderTarget (m_pRenderTarget) [Check RenderTarget Class]
	m_pRenderTarget->Clear(gameContext, DirectX::Colors::Black);
	//2. Call UpdateEffectVariables(...)
	UpdateEffectVariables(gameContext, pPrevRendertarget, pOriginalRendertarget); //give previous to use as srv
	//3. Set InputLayout
	gameContext.pRenderer->GetDeviceContext()->IASetInputLayout(m_pInputLayout);

	// Set the indexbuffer.
	gameContext.pRenderer->GetDeviceContext()->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//4. Set VertexBuffer
	UINT offset = 0;
	gameContext.pRenderer->GetDeviceContext()->IASetVertexBuffers(0,1,&m_pVertexBuffer, &m_VertexBufferStride, &offset);

	//5. Set PrimitiveTopology (TRIANGLELIST)
	gameContext.pRenderer->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//6. Draw 

	//DRAW
	D3DX11_TECHNIQUE_DESC techDesc;
	m_pTechnique->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pTechnique->GetPassByIndex(p)->Apply(0, gameContext.pRenderer->GetDeviceContext());
		gameContext.pRenderer->GetDeviceContext()->DrawIndexed(m_NumIndices, 0, 0);
	}

	// Generate Mips
	gameContext.pRenderer->GetDeviceContext()->GenerateMips(m_pRenderTarget->GetShaderResourceView());
}

void PostProcessingMaterial::CreateVertexBuffer(const GameContext& gameContext)
{
	PIX_PROFILE();

	m_NumVertices = 4;	

	UNREFERENCED_PARAMETER(gameContext);
	//TODO: complete
	//Create vertex ARRAY containing three elements in system memory
	VertexPosTex arr[] = {
	arr[0] = VertexPosTex{ DirectX::XMFLOAT3{-1,1,0}, DirectX::XMFLOAT2{0,0} },
	arr[1] = VertexPosTex{ DirectX::XMFLOAT3{-1,-1,0}, DirectX::XMFLOAT2{0,1} },
	arr[2] = VertexPosTex{ DirectX::XMFLOAT3{1,1,0}, DirectX::XMFLOAT2{1,0} },
	arr[3] = VertexPosTex{ DirectX::XMFLOAT3{1,-1,0}, DirectX::XMFLOAT2{1,1} },
	};
	m_VertexBufferStride = sizeof(VertexPosTex);

	//fill a buffer description to copy the vertexdata into graphics memory
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = sizeof(VertexPosTex) * 4;
	desc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	desc.MiscFlags = 0;
	desc.CPUAccessFlags = 0;

	// Define the resource data.
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = arr;

	//create a ID3D11Buffer in graphics memory containing the vertex info
	HRESULT hr = gameContext.pRenderer->GetDevice()->CreateBuffer(&desc, &InitData, &m_pVertexBuffer);
	Logger::LogHResult(hr, L"failed to create vertexbuffer in postProcessingMaterial\n");
}

void PostProcessingMaterial::CreateIndexBuffer(const GameContext& gameContext)
{
	PIX_PROFILE();

	m_NumIndices = 6;

	if (m_pIndexBuffer != nullptr)
		return;

	//TODO: complete
	// Create index buffer
	unsigned int indices[] = { 0,2,1,1,2,3 };
	// Fill in a buffer description.
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(unsigned int) * 6;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = indices;

	auto hr = gameContext.pRenderer->GetDevice()->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	Logger::LogHResult(hr, L"PostProcessingMaterial::CreateIndexBuffer()");
}
