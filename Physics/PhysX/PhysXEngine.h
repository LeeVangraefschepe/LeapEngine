#pragma once

#include "../Interfaces/IPhysics.h"
#include "../Interfaces/IShape.h"

#include "PhysXSimulationFilterCallback.h"

#include <memory>
#include <unordered_map>

#include <vec3.hpp>
#pragma warning(disable: 4201)
#include "gtc/quaternion.hpp"
#pragma warning(default: 4201)

namespace physx
{
	class PxDefaultErrorCallback;
	class PxDefaultAllocator;
	class PxFoundation;
	class PxPhysics;
	class PxCooking;
	class PxDefaultCpuDispatcher;
}

namespace leap::physics
{
	class IPhysicsScene;
	class IPhysicsMaterial;
	class PhysXObject;
	class PhysXSimulationCallbacks;

	class PhysXEngine final : public IPhysics
	{
	public:
		PhysXEngine();
		virtual ~PhysXEngine();

		PhysXEngine(const PhysXEngine& other) = delete;
		PhysXEngine(PhysXEngine&& other) = delete;
		PhysXEngine& operator=(const PhysXEngine& other) = delete;
		PhysXEngine& operator=(PhysXEngine&& other) = delete;

		virtual void SetSyncFunc(const std::function<void(void*, const glm::vec3&, const glm::quat&)>& setFunc, const std::function<std::pair<glm::vec3, glm::quat>(void*)> getFunc) override;
		virtual void Update(float fixedDeltaTime) override;

		virtual void CreateScene() override;
		virtual IPhysicsObject* Get(void* pOwner) override;
		virtual std::unique_ptr<IShape> CreateShape(void* pOwner, EShape shape, IPhysicsMaterial* pMaterial = nullptr) override;
		virtual std::shared_ptr<IPhysicsMaterial> CreateMaterial() override;

		virtual void SetEnabledDebugDrawing(bool isEnabled) override;
		virtual std::vector<std::pair<glm::vec3, glm::vec3>> GetDebugDrawings() override;

		virtual TSubject<const CollisionData&>& OnCollisionEnter() override { return m_OnCollisionEnter; }
		virtual TSubject<const CollisionData&>& OnCollisionStay() override { return m_OnCollisionStay; }
		virtual TSubject<const CollisionData&>& OnCollisionExit() override { return m_OnCollisionExit; }
		virtual TSubject<const CollisionData&>& OnTriggerEnter() override { return m_OnTriggerEnter; }
		virtual TSubject<const CollisionData&>& OnTriggerStay() override { return m_OnTriggerStay; }
		virtual TSubject<const CollisionData&>& OnTriggerExit() override { return m_OnTriggerExit; }

		virtual bool Raycast(const glm::vec3& start, const glm::vec3& direction, float distance, RaycastHit& hitInfo) override;

		physx::PxPhysics* GetPhysics() const { return m_pPhysics; }
		physx::PxCooking* GetCooking() const { return m_pCooking; }

	private:
		void OnSimulationEvent(const SimulationEvent& e);

		IPhysicsMaterial* GetDefaultMaterial();

		std::unique_ptr<physx::PxDefaultErrorCallback> m_pDefaultErrorCallback{};
		std::unique_ptr<physx::PxDefaultAllocator> m_pDefaultAllocatorCallback{};
		std::unique_ptr<PhysXSimulationCallbacks> m_pSimulationCallbacks{};
		std::unique_ptr<PhysXSimulationFilterCallback> m_pSimulationFilterCallback{};

		physx::PxFoundation* m_pFoundation{};
		physx::PxPhysics* m_pPhysics{};
		physx::PxCooking* m_pCooking{};
		physx::PxDefaultCpuDispatcher* m_pDispatcher{};

		std::unique_ptr<physics::IPhysicsScene> m_pScene{};

		std::unordered_map<void*, std::unique_ptr<PhysXObject>> m_pObjects{};

		std::function<std::pair<const glm::vec3&, const glm::quat&>(void*)> m_SyncGetFunc{};
		std::function<void(void*, glm::vec3, glm::quat)> m_SyncSetFunc{};

		TSubject<const CollisionData&> m_OnCollisionEnter{};
		TSubject<const CollisionData&> m_OnCollisionStay{};
		TSubject<const CollisionData&> m_OnCollisionExit{};
		TSubject<const CollisionData&> m_OnTriggerEnter{};
		TSubject<const CollisionData&> m_OnTriggerStay{};
		TSubject<const CollisionData&> m_OnTriggerExit{};

		bool m_IsDebugDrawingEnabled{};
	};
}