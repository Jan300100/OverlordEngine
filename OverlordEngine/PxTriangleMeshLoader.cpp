#include "stdafx.h"
#include "PxTriangleMeshLoader.h"
#include "PhysxManager.h"

#include "StringHelper.h"

std::shared_ptr<physx::PxTriangleMesh> PxTriangleMeshLoader::LoadContent(const std::wstring& assetFile)
{
	PIX_PROFILE();

	auto inputStream  = physx::PxDefaultFileInputData(StringHelpers::WStringToString(assetFile).c_str());
	std::shared_ptr<physx::PxTriangleMesh> result{ PhysxManager::GetInstance()->GetPhysics()->createTriangleMesh(inputStream),
		[](physx::PxTriangleMesh* pMesh) 
		{
			pMesh->release();
		}
	};
	return result;
}
