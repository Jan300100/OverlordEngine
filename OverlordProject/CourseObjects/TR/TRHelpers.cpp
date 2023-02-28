#include "stdafx.h"
#include "TRHelpers.h"
#include "../../Materials/Pbr/PbrGroundMaterial.h"
#include "../../Materials/Pbr/PbrMaterial_Shadow.h"
#include "../../Materials/Pbr/PbrPropsMaterial.h"
#include "../../Materials/Pbr/PbrFoliageMaterial.h"
#include "GameObject.h"
#include "TextureData.h"
#include "Components.h"


DirectX::XMFLOAT4X4 CreateTransform(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& rot, const DirectX::XMFLOAT3& scale)
{
	DirectX::XMMATRIX translationMat = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
	DirectX::XMMATRIX rotationMat = DirectX::XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);
	DirectX::XMMATRIX scaleMat = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	DirectX::XMFLOAT4X4 result;
	DirectX::XMStoreFloat4x4(&result, DirectX::XMMatrixMultiply(scaleMat, DirectX::XMMatrixMultiply(rotationMat, translationMat)));
	return result;
}

bool XMINT2Less::operator()(const DirectX::XMINT2& i1, const DirectX::XMINT2& i2) const
{
	if (i1.x == i2.x)
		return i1.y < i2.y;
	return i1.x < i2.x;
}

bool XMINT2Equal::operator()(const DirectX::XMINT2& i1, const DirectX::XMINT2& i2) const
{
	return !(XMINT2Less()(i1, i2) || XMINT2Less()(i1, i2));
}
