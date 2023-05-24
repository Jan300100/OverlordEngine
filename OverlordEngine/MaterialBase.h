#pragma once
#include <string>
#include <d3d11.h>

class MaterialBase
{
public:
	MaterialBase();
	virtual ~MaterialBase();
private:
	std::wstring m_file;
	ID3D11InputLayout* m_pInputLayout;


};