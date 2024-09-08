#include "Collider.h"

#include "../../ServiceLocator/ServiceLocator.h"
#include "../../SceneGraph/GameObject.h"

#include "../Transform/Transform.h"
#include "Rigidbody.h"

#include <Interfaces/IPhysics.h>
#include <Interfaces/IPhysicsObject.h>

void leap::Collider::BaseSetupShape()
{
	SetupShape(m_pMaterial.get());
	if (!m_pShape) return;
	m_pShape->SetTrigger(m_IsTrigger);
}

void leap::Collider::Awake()
{
	if (m_pOwningObject) return;

	BaseSetupShape();
	if (!m_pShape) return;

	physics::IPhysics& physics{ ServiceLocator::GetPhysics() };

	// Try getting a rigidbody 
	Rigidbody* pRigidbody{ GetGameObject()->GetComponent<Rigidbody>() };
	if (!pRigidbody) pRigidbody = GetGameObject()->GetComponentInParent<Rigidbody>();

	// Get the physics object associated with the owning gameobject or the owning gameobject of the closest rigidbody
	m_pOwningObject = pRigidbody == nullptr ? GetGameObject() : pRigidbody->GetGameObject();
	physics::IPhysicsObject* pObject{ physics.Get(m_pOwningObject) };

	// Apply the shape
	pObject->AddShape(m_pShape.get());

	// Set the transform of the physics object if there is no rigidbody (if there is, it is the responsibility of the rigidbody)
	if (!pRigidbody) pObject->SetTransform(GetTransform()->GetWorldPosition(), GetTransform()->GetWorldRotation());
	else
	{
		const glm::vec3 relativePosition{ GetTransform()->GetWorldPosition() - pRigidbody->GetTransform()->GetWorldPosition() };
		const glm::quat relativeRotation{ GetTransform()->GetWorldRotation() * glm::conjugate(pRigidbody->GetTransform()->GetWorldRotation()) };

		m_pShape->SetRelativeTransform(relativePosition, relativeRotation);
	}

	GetTransform()->OnScaleChanged.AddListener(this, &Collider::RescaleShape);
}

void leap::Collider::OnDestroy()
{
	if (!m_pShape) return;
	ServiceLocator::GetPhysics().Get(m_pOwningObject)->RemoveShape(m_pShape.get());
	GetTransform()->OnScaleChanged.RemoveListener(this);
}

void leap::Collider::Move(const Rigidbody* pRigidbody)
{
	// If the rigidbody and collider already share the same physics object, do nothing
	if (pRigidbody->GetGameObject() == m_pOwningObject) return;

	physics::IPhysics& physics{ ServiceLocator::GetPhysics() };

	// Remove the shape from the previous owner
	if (m_pOwningObject) physics.Get(m_pOwningObject)->RemoveShape(m_pShape.get());
	else BaseSetupShape();

	const glm::vec3 relativePosition{ (GetTransform()->GetWorldPosition() - pRigidbody->GetTransform()->GetWorldPosition()) * pRigidbody->GetTransform()->GetWorldRotation() };
	const glm::quat relativeRotation{ glm::conjugate(pRigidbody->GetTransform()->GetWorldRotation()) * GetTransform()->GetWorldRotation() };

	m_pShape->SetRelativeTransform(relativePosition, relativeRotation);

	// Apply the shape to the rigidbody
	m_pOwningObject = pRigidbody->GetGameObject();
	physics.Get(m_pOwningObject)->AddShape(m_pShape.get());
}

void leap::Collider::SetMaterial(const std::shared_ptr<physics::IPhysicsMaterial>& pMaterial)
{
	m_pMaterial = pMaterial;
}

void leap::Collider::SetTrigger(bool isTrigger)
{
	m_IsTrigger = isTrigger;

	if (m_pShape) m_pShape->SetTrigger(isTrigger);
}

leap::Rigidbody* leap::Collider::GetRigidbody() const
{
	return m_pOwningObject->GetComponent<Rigidbody>();
}