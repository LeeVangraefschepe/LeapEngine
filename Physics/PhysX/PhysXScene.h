#pragma once

#include "../Interfaces/IPhysicsScene.h"

namespace physx
{
	class PxScene;
	class PxRigidActor;
}

namespace leap::physics
{
	class PhysXScene final : public IPhysicsScene
	{
	public:
		PhysXScene(physx::PxScene* pEngine);
		virtual ~PhysXScene();

		PhysXScene(const PhysXScene& other) = delete;
		PhysXScene(PhysXScene&& other) = delete;
		PhysXScene& operator=(const PhysXScene& other) = delete;
		PhysXScene& operator=(PhysXScene&& other) = delete;

		virtual void Simulate(float fixedDeltaTime) override;

		void AddActor(physx::PxRigidActor* pActor);
		void RemoveActor(physx::PxRigidActor* pActor);

	private:
		physx::PxScene* m_pScene{};
	};
}