#pragma once
#include <Renderer/GAResource.h>

struct ID3D11Buffer;

class DX11Resource : public GA::Resource
{
	ID3D11Buffer* m_pBuffer;
public:
	ID3D11Buffer* GetBuffer();
public:
	// Inherited via Resource
	virtual void Create(const Params& createParams) override;
	virtual void Release() override;
};