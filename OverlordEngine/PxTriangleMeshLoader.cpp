#include "stdafx.h"
#include "PxTriangleMeshLoader.h"
#include "PhysxManager.h"

#include "StringHelper.h"

physx::PxTriangleMesh* PxTriangleMeshLoader::LoadContent(const std::wstring& assetFile)
{
	PIX_PROFILE();

	auto inputStream  = physx::PxDefaultFileInputData(StringHelpers::WStringToString(assetFile).c_str());
	return PhysxManager::GetInstance()->GetPhysics()->createTriangleMesh(inputStream);
}

void PxTriangleMeshLoader::Destroy(physx::PxTriangleMesh*) {}
