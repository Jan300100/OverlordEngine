#include "stdafx.h"
#include "PxConvexMeshLoader.h"
#include "PhysxManager.h"

#include "StringHelper.h"

physx::PxConvexMesh* PxConvexMeshLoader::LoadContent(const std::wstring& assetFile)
{
	PIX_PROFILE();

	auto inputStream = physx::PxDefaultFileInputData(StringHelpers::WStringToString(assetFile).c_str());
	return PhysxManager::GetInstance()->GetPhysics()->createConvexMesh(inputStream);
}

void PxConvexMeshLoader::Destroy(physx::PxConvexMesh* ) {}
