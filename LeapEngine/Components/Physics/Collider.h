#pragma once

#include "../Component.h"

#include <Interfaces/IShape.h>
#include <Subject.h>

#include <memory>

namespace leap
{
	class Rigidbody;
	class GameObject;
	class PhysicsSync;
	
	namespace physics
	{
		class IPhysicsMaterial;
	}

	class Collider : public Component
	{
	public:
		struct CollisionCallback final
		{
			Collider* pCollider{};
			Collider* pOther{};
		};

		Collider() = default;
		virtual ~Collider() = default;

		Collider(const Collider& other) = delete;
		Collider(Collider&& other) = delete;
		Collider& operator=(const Collider& other) = delete;
		Collider& operator=(Collider&& other) = delete;

		void SetMaterial(const std::shared_ptr<physics::IPhysicsMaterial>& pMaterial);
		void SetTrigger(bool isTrigger);

		Rigidbody* GetRigidbody() const;

	protected:
		virtual void SetupShape(physics::IPhysicsMaterial* pMaterial) = 0;
		virtual void RescaleShape() = 0;

		std::unique_ptr<physics::IShape> m_pShape{};

	private:
		void BaseSetupShape();

		friend PhysicsSync;

		virtual void Awake() override;
		virtual void OnDestroy() override;

		void Move(const Rigidbody* pRigidbody);

		GameObject* m_pOwningObject{};
		std::shared_ptr<physics::IPhysicsMaterial> m_pMaterial{};
		bool m_IsTrigger{};

		friend Rigidbody;
	};
}