#include "stdafx.h"
#include "PostProcessingMaterial.h"
#include "RenderTarget.h"
#include "OverlordGame.h"
#include "ContentManager.h"
#include <GA/DX11/InterfaceDX11.h>

#include <StringHelper.h>

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
		m_pRenderTarget = new RenderTarget{GA::DX11::SafeCast(gameContext.pRenderer)->GetDevice()};
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
		 m_pTechnique = m_pEffect->GetTechniqueByName(StringHelpers::WStringToString(m_TechniqueName).c_str());
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
	EffectHelper::BuildInputLayout(GA::DX11::SafeCast(gameContext.pRenderer)->GetDevice(), m_pTechnique, &m_pInputLayout,m_pInputLayoutDescriptions, m_pInputLayoutSize, m_InputLayoutID);

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
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetInputLayout(m_pInputLayout);

	// Set the indexbuffer.
	ID3D11Buffer* internalBuf = std::any_cast<ID3D11Buffer*>(m_pIndexBuffer->GetInternal());
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetIndexBuffer(internalBuf, DXGI_FORMAT_R32_UINT, 0);

	//4. Set VertexBuffer
	UINT offset = 0;
	internalBuf = std::any_cast<ID3D11Buffer*>(m_pVertexBuffer->GetInternal());
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetVertexBuffers(0,1,&internalBuf, &m_VertexBufferStride, &offset);

	//5. Set PrimitiveTopology (TRIANGLELIST)
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//6. Draw 

	//DRAW
	D3DX11_TECHNIQUE_DESC techDesc;
	m_pTechnique->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pTechnique->GetPassByIndex(p)->Apply(0, GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext());
		GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->DrawIndexed(m_NumIndices, 0, 0);
	}

	// Generate Mips
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->GenerateMips(m_pRenderTarget->GetShaderResourceView());
}

void PostProcessingMaterial::CreateVertexBuffer(const GameContext& gameContext)
{
	PIX_PROFILE();

	m_NumVertices = 4;	

	//TODO: complete
	//Create vertex ARRAY containing three elements in system memory
	VertexPosTex initialData[] = {
	initialData[0] = VertexPosTex{ DirectX::XMFLOAT3{-1,1,0}, DirectX::XMFLOAT2{0,0} },
	initialData[1] = VertexPosTex{ DirectX::XMFLOAT3{-1,-1,0}, DirectX::XMFLOAT2{0,1} },
	initialData[2] = VertexPosTex{ DirectX::XMFLOAT3{1,1,0}, DirectX::XMFLOAT2{1,0} },
	initialData[3] = VertexPosTex{ DirectX::XMFLOAT3{1,-1,0}, DirectX::XMFLOAT2{1,1} },
	};

	m_VertexBufferStride = sizeof(VertexPosTex);

	GA::Buffer::Params params;
	params.type = GA::Buffer::Type::Vertex;
	params.lifeTime = GA::Resource::LifeTime::Permanent;
	params.sizeInBytes = sizeof(initialData);
	params.initialData = initialData;
	params.cpuUpdateFreq = GA::Resource::CPUUpdateFrequency::Never;

	m_pVertexBuffer = gameContext.pRenderer->CreateBuffer(params);
}

void PostProcessingMaterial::CreateIndexBuffer(const GameContext& gameContext)
{
	PIX_PROFILE();

	m_NumIndices = 6;

	if (m_pIndexBuffer != nullptr)
		return;

	// Create index buffer
	unsigned int indices[] = { 0,2,1,1,2,3 };

	GA::Buffer::Params params;
	params.lifeTime = GA::Resource::LifeTime::Permanent;
	params.cpuUpdateFreq = GA::Resource::CPUUpdateFrequency::Never;
	params.sizeInBytes = sizeof(unsigned int) * 6;
	params.initialData = indices;
	params.type = GA::Buffer::Type::Index;
	m_pIndexBuffer = gameContext.pRenderer->CreateBuffer(params);
}
