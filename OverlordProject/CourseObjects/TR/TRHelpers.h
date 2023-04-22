#pragma once
#include <random>

class GameObject;
struct GameContext;
class GA::Texture2D;

enum class Direction
{
	ZPos = 0, XPos = 1, ZNeg = 2, XNeg = 3
};

struct XMINT2Less
{
	bool operator()(const DirectX::XMINT2& i1, const DirectX::XMINT2& i2) const;
};

struct XMINT2Equal
{
	bool operator()(const DirectX::XMINT2& i1, const DirectX::XMINT2& i2) const;
};

struct XMINT2Hash
{
public:  size_t operator()(const DirectX::XMINT2& val) const { return std::hash<int>()(val.x) ^ std::hash<int>()(val.y); };
};


DirectX::XMFLOAT4X4 CreateTransform(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& rot,const DirectX::XMFLOAT3& scale);