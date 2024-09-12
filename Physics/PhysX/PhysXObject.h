#pragma once

#include "../Interfaces/IPhysicsObject.h"

#include <vector>
#include <functional>
#include <memory>

#include <vec3.hpp>
#pragma warning(disable: 4201)
#include "gtc/quaternion.hpp"
#pragma warning(default: 4201)

namespace physx
{
	class PxRigidActor;
}

namespace leap::physics
{
	class PhysXEngine;
	class IPhysicsScene;
	class IPhysXShape;

	class PhysXObject final : public IPhysicsObject
	{
	public:
		PhysXObject(void* pOwner);
		virtual ~PhysXObject();

		void Update(PhysXEngine* pEngine, IPhysicsScene* pScene);
		void Apply(const std::function<void(void*, const glm::vec3&, const glm::quat&)>& setFunc, const std::function<std::pair<const glm::vec3&, const glm::quat&>(void*)> getFunc) const;

		virtual void AddShape(IShape* pShape) override;
		virtual void RemoveShape(IShape* pShape) override;
		virtual Rigidbody* SetRigidbody(bool hasRigidbody) override;
		virtual bool IsValid() override;

		virtual void SetTransform(const glm::vec3& position, const glm::quat& rotation) override;
		virtual glm::vec3 GetPosition() override;
		virtual glm::quat GetRotation() override;

	private:
		void UpdateObject(PhysXEngine* pEngine, IPhysicsScene* pScene);
		void UpdateTransform();
		void UpdateRigidbody();
		void CalculateCenterOfMass() const;

		void OnRigidBodyUpdateRequest();

		std::vector<IPhysXShape*> m_pShapes{};
		physx::PxRigidActor* m_pActor{};
		void* m_pOwner{};
		std::unique_ptr<Rigidbody> m_pRigidbody{};

		glm::vec3 m_Position{};
		glm::quat m_Rotation{};

		bool m_IsObjectDirty{ true };
		bool m_IsTransformDirty{ true };
		bool m_NewFrame{ false };

		inline static const glm::quat OFFSET_SET{ 0.7071068f, 0, 0, -0.7071068f };
		inline static const glm::quat OFFSET_GET{ 0.7071068f, 0, 0, 0.7071068f };
	};
}