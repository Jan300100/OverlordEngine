#include "stdafx.h"
#include "PxConvexMeshLoader.h"
#include "PhysxManager.h"

#include "StringHelper.h"

std::shared_ptr<physx::PxConvexMesh> PxConvexMeshLoader::LoadContent(const std::wstring& assetFile)
{
	PIX_PROFILE();

	auto inputStream = physx::PxDefaultFileInputData(StringHelpers::WStringToString(assetFile).c_str());
	std::shared_ptr<physx::PxConvexMesh> result{ PhysxManager::GetInstance()->GetPhysics()->createConvexMesh(inputStream),
		[](physx::PxConvexMesh* pMesh)
		{
			pMesh->release();
		} };
	return result;
}
