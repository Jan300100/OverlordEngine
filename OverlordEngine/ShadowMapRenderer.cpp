#include "stdafx.h"
#include "ShadowMapRenderer.h"
#include "ContentManager.h"
#include "ShadowMapMaterial.h"
#include "RenderTarget.h"
#include "MeshFilter.h"
#include "SceneManager.h"
#include "OverlordGame.h"
#include "ModelComponent.h"
#include "TransformComponent.h"

#include <GA/Interface.h>
#include <GA/DX11/InterfaceDX11.h>

using namespace DirectX;


ShadowMapRenderer::~ShadowMapRenderer()
{
	delete m_pShadowRT;
	delete m_pShadowMat;
}

void ShadowMapRenderer::Initialize(const GameContext& gameContext)
{
	PIX_PROFILE();

	if (m_IsInitialized)
		return;

	//create shadow generator material + initialize it
	m_pShadowMat = new ShadowMapMaterial{};
	m_pShadowMat->Initialize(gameContext);

	//TODO: create a rendertarget with the correct settings (hint: depth only) for the shadow generator using a RENDERTARGET_DESC
	RENDERTARGET_DESC desc{};
	desc.EnableColorBuffer = false;
	desc.Width = OverlordGame::GetGameSettings().Window.Width * 2;
	desc.Height = OverlordGame::GetGameSettings().Window.Height * 2;
	desc.EnableDepthSRV = true;

	//viewport
	m_Viewport.Width = float(desc.Width);
	m_Viewport.Height = float(desc.Height);
	m_Viewport.MinDepth = 0;
	m_Viewport.MaxDepth = 1;
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;

	m_pShadowRT =  new RenderTarget {GA::DX11::SafeCast(gameContext.pRenderer)->GetDevice()};
	m_pShadowRT->Create(desc);

	m_IsInitialized = true;
}

void ShadowMapRenderer::SetLightPosition(DirectX::XMFLOAT3 position)
{
	SetLight(position, m_LightDirection);
}

void ShadowMapRenderer::SetLightDirection(DirectX::XMFLOAT3 direction)
{
	SetLight(m_LightPosition, direction);
}

void ShadowMapRenderer::SetSize(float size)
{
	m_Size = size;
}

void ShadowMapRenderer::SetLight(XMFLOAT3 position, XMFLOAT3 direction)
{
	PIX_PROFILE();

	//store the input parameters in the appropriate datamembers
	m_LightDirection = direction;
	m_LightPosition = position;
	//calculate the Light VP matrix (Directional Light only ;)) and store it in the appropriate datamember
	XMVECTOR pos = XMLoadFloat3(&position);
	XMVECTOR dir = XMLoadFloat3(&direction);

	const auto windowSettings = OverlordGame::GetGameSettings().Window;
	XMMATRIX projection;
	
	const float viewWidth = (m_Size > 0) ? m_Size * windowSettings.AspectRatio : windowSettings.Width;
	const float viewHeight = (m_Size > 0) ? m_Size : windowSettings.Height;
	projection = XMMatrixOrthographicLH(viewWidth, viewHeight, m_Near, m_Far);
	
	const XMMATRIX view = XMMatrixLookAtLH(pos, pos + dir, XMVECTOR{0,1,0});

	XMStoreFloat4x4(&m_LightVP, view * projection);
}

void ShadowMapRenderer::SetLightBasedOnCamera(const GameContext& gameContext)
{
	PIX_PROFILE();

	auto viewProjInverse = XMLoadFloat4x4(&gameContext.pCamera->GetViewProjectionInverse());

	
	XMVECTOR vecFrustum[8] =
	{
		{-1, -1, 0, 1},
		{-1,  1, 0, 1},
		{ 1,  1, 0, 1},
		{ 1, -1, 0, 1},
		{-1, -1, 1, 1},
		{-1,  1, 1, 1},
		{ 1,  1, 1, 1},
		{ 1, -1, 1, 1},
	};

	XMFLOAT4 frustum[8] =
	{};

	for (int i = 0; i < 8; i++)
	{
		vecFrustum[i] = XMVector4Transform(vecFrustum[i], viewProjInverse);
		XMStoreFloat4(&frustum[i], vecFrustum[i]);
		frustum[i].x /= frustum[i].w;
		frustum[i].y /= frustum[i].w;
		frustum[i].z /= frustum[i].w;
		frustum[i].w /= frustum[i].w;
	}

	auto dir = XMLoadFloat3(&m_LightDirection);
	XMMATRIX view = XMMatrixLookAtLH({}, dir, XMVECTOR{ 0,1,0 });

	for (int i = 0; i < 8; i++)
	{
		vecFrustum[i] = XMLoadFloat4(&frustum[i]);
		//to light view
		vecFrustum[i] = XMVector4Transform(vecFrustum[i], view);
		XMStoreFloat4(&frustum[i], vecFrustum[i]);
	}

	float maxX{ frustum[0].x }, minX{ frustum[0].x }
	, maxY{ frustum[0].y }, minY{ frustum[0].y}
	, maxz{ frustum[0].z }, minz{ frustum[0].z};
	for (size_t i = 1; i < 8; i++)
	{
		if (frustum[i].x > maxX) maxX = frustum[i].x;
		else if (frustum[i].x < minX) minX = frustum[i].x;
		if (frustum[i].y > maxY) maxY = frustum[i].y;
		else if (frustum[i].y < minY) minY = frustum[i].y;
		if (frustum[i].z > maxz) maxz = frustum[i].z;
		else if (frustum[i].z < minz) minz = frustum[i].z;
	}

	XMMATRIX projection = XMMatrixOrthographicOffCenterLH(minX, maxX, minY, maxY, minz, maxz);

	XMStoreFloat4x4(&m_LightVP, view * projection);
}

void ShadowMapRenderer::SetLightBasedOnCameraSphere(const GameContext& gameContext)
{
	PIX_PROFILE();

	//calculate diagonal length of frustum
	XMVECTOR vecFrustum[2] = { {-1, -1, 0, 1},{1, 1, 1, 1}};
	XMFLOAT4 frustum[2] = {};
	
	//frustumPoints to world
	auto viewProjInverse = XMLoadFloat4x4(&gameContext.pCamera->GetViewProjectionInverse());

	for (int i = 0; i < 2; i++)
	{
		vecFrustum[i] = XMVector4Transform(vecFrustum[i], viewProjInverse);
		XMStoreFloat4(&frustum[i], vecFrustum[i]);
		frustum[i].x /= frustum[i].w;
		frustum[i].y /= frustum[i].w;
		frustum[i].z /= frustum[i].w;
		frustum[i].w /= frustum[i].w;
	}
	float diagonalLength; 
	XMStoreFloat(&diagonalLength, XMVector3Length({ frustum[0].x - frustum[1].x, frustum[0].y - frustum[1].y, frustum[0].z - frustum[1].z }));
	
	//view Matrix : based on light direction
	auto dir = XMLoadFloat3(&m_LightDirection);
	XMMATRIX view = XMMatrixLookAtLH({}, dir, XMVECTOR{ 0,1,0 });
	//transform cameraposition to light space
	XMFLOAT3 camPos = gameContext.pCamera->GetTransform()->GetWorldPosition();
	XMVECTOR center = {camPos.x, camPos.y, camPos.z, 1};
	center = XMVector4Transform(center, view);

	//create bounding"sphere" using the diagonal as diameter of the orthographic matrix
	XMFLOAT4 boundingSphereCenter;
	XMStoreFloat4(&boundingSphereCenter, center);

	XMMATRIX projection = XMMatrixOrthographicOffCenterLH(boundingSphereCenter.x - diagonalLength/2.0f, boundingSphereCenter.x + diagonalLength/2.0f
		, boundingSphereCenter.y - diagonalLength / 2.0f, boundingSphereCenter.y + diagonalLength / 2.0f
		, boundingSphereCenter.z - diagonalLength / 2.0f, boundingSphereCenter.z + diagonalLength / 2.0f
		);
	//XMMATRIX projection = XMMatrixOrthographicOffCenterLH(minX, maxX, minY, maxY, minZ,maxZ);
	XMStoreFloat4x4(&m_LightVP, view * projection);
}

void ShadowMapRenderer::Begin(const GameContext& gameContext)
{
	PIX_PROFILE();

	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->RSSetViewports(1, &m_Viewport);
	//Reset Texture Register 5 (Unbind)
	ID3D11ShaderResourceView *const pSRV[] = { nullptr,nullptr,nullptr,nullptr };
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->PSSetShaderResources(1, 4, pSRV);

	//TODO: set the appropriate render target that our shadow generator will write to (hint: use the OverlordGame::SetRenderTarget function through SceneManager)
	SceneManager::GetInstance()->GetGame()->GetRenderer()->SetRenderTarget(m_pShadowRT);
	//TODO: clear this render target
	FLOAT clearColor[4] = { 0,0,0,0 };
	m_pShadowRT->Clear(gameContext, clearColor);
	//TODO: set the shader variables of this shadow generator material
	m_pShadowMat->SetLightVP(m_LightVP);
}

void ShadowMapRenderer::End(const GameContext& gameContext) const
{
	PIX_PROFILE();

	UNREFERENCED_PARAMETER(gameContext);
	//restore default render target
	SceneManager::GetInstance()->GetGame()->GetRenderer()->SetRenderTarget(nullptr);
	SceneManager::GetInstance()->GetGame()->GetRenderer()->ResetViewPort();

}

void ShadowMapRenderer::ImGuiUpdate()
{
	PIX_PROFILE();

	//ImGui::Begin("shadowmapping");
	//ImGui::DragFloat("size", &m_Size, 1, 50, 5000);
	//ImGui::DragFloat("near", &m_Near, 0.01f, 0.0f, 500.0f);
	//ImGui::DragFloat("far", &m_Far, 1.0f, 1.0f, 1000.0f);
	//ImGui::SliderFloat3("pos", reinterpret_cast<float*>(&m_LightPosition), 0, 500);
	//ImGui::SliderFloat3("dir", reinterpret_cast<float*>(&m_LightDirection), -1, 1);
	//ImGui::End();
	//SetSize(m_Size);
	//SetLight(m_LightPosition, m_LightDirection);
}


void ShadowMapRenderer::Draw(const GameContext& gameContext, MeshFilter* pMeshFilter, XMFLOAT4X4 world, const std::vector<XMFLOAT4X4>& bones) const
{
	PIX_PROFILE();

	//TODO: update shader variables in material
	m_pShadowMat->SetWorld(world);
	m_pShadowMat->SetBones((float*)bones.data(), (int)bones.size());
	
	//TODO: set the correct inputlayout, buffers, topology (some variables are set based on the generation type Skinned or Static)
	//Set Shader Variables
	VertexBufferData vBData = pMeshFilter->GetVertexBufferData(gameContext, m_pShadowMat->m_InputLayoutIds[pMeshFilter->m_HasAnimations]);
	if (vBData.VertexCount == 0) return;
	
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetInputLayout(m_pShadowMat->m_pInputLayouts[pMeshFilter->m_HasAnimations]);

	//Set Vertex Buffer
	UINT offset = 0;
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetVertexBuffers(0, 1, &vBData.pVertexBuffer, &vBData.VertexStride, &offset);

	//Set Index Buffer
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetIndexBuffer(pMeshFilter->m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//Set Primitive Topology
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//DRAW
	ID3DX11EffectTechnique* tech = m_pShadowMat->m_pShadowTechs[pMeshFilter->m_HasAnimations];
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext());
		GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->DrawIndexed(pMeshFilter->m_IndexCount, 0, 0);
	}

}

void ShadowMapRenderer::DrawCustom(const GameContext& gameContext, ModelComponent* pModelComponent)
{
	PIX_PROFILE();

	//modelComponent provides technique for shadowMapping 'tShadowMapping'
	//check if technique is present, if not, return
	//set wvp as world light viewproj
	UNREFERENCED_PARAMETER(gameContext);
	UNREFERENCED_PARAMETER(pModelComponent);
}

void ShadowMapRenderer::UpdateMeshFilter(const GameContext& gameContext, MeshFilter* pMeshFilter)
{
	PIX_PROFILE();

	//TODO: based on the type (Skinned or Static) build the correct vertex buffers for the MeshFilter (Hint use MeshFilter::BuildVertexBuffer)
	pMeshFilter->BuildVertexBuffer(gameContext, m_pShadowMat->m_InputLayoutIds[pMeshFilter->m_HasAnimations], m_pShadowMat->m_InputLayoutSizes[pMeshFilter->m_HasAnimations], m_pShadowMat->m_InputLayoutDescriptions[pMeshFilter->m_HasAnimations]);
}

ID3D11ShaderResourceView* ShadowMapRenderer::GetShadowMap() const
{
	PIX_PROFILE();

	//TODO: return the depth shader resource view of the shadow generator render target
	//return nullptr;
	return m_pShadowRT->GetDepthShaderResourceView();
}
