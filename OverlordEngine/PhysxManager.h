#pragma once
#include "Singleton.h"

class PhysxAllocator;
class PhysxErrorCallback;
class GameScene;
class OverlordGame;

class PhysxManager final : public Singleton<PhysxManager>
#if defined(_M_IX86)
	, public physx::PxVisualDebuggerConnectionHandler
#endif
{
public:
	friend class Singleton<PhysxManager>;

	void Init(void* pDevice);

	physx::PxPhysics* GetPhysics() const { return m_pPhysics; }
	physx::PxScene* CreateScene(GameScene* pScene) const;

#if defined(_M_IX86)
	virtual void onPvdSendClassDescriptions(physx::PxVisualDebuggerConnection& connection) override;
	virtual void onPvdConnected(physx::PxVisualDebuggerConnection& connection) override;
	virtual void onPvdDisconnected(physx::PxVisualDebuggerConnection& connection) override;
#endif
	bool ToggleVisualDebuggerConnection() const;

	PhysxManager(const PhysxManager& other) = delete;
	PhysxManager(PhysxManager&& other) noexcept = delete;
	PhysxManager& operator=(const PhysxManager& other) = delete;
	PhysxManager& operator=(PhysxManager&& other) noexcept = delete;

private:
	PhysxManager();
	virtual ~PhysxManager();

	void CleanUp();

	PhysxAllocator* m_pDefaultAllocator;
	PhysxErrorCallback* m_pDefaultErrorCallback;
	physx::PxFoundation* m_pFoundation;
#if defined (_M_IX86)
	physx::PxProfileZoneManager* m_pProfileZoneManager;
#endif
	physx::PxPhysics* m_pPhysics;
	physx::PxDefaultCpuDispatcher* m_pDefaultCpuDispatcher;
	physx::PxCudaContextManager* m_pCudaContextManager;
};
