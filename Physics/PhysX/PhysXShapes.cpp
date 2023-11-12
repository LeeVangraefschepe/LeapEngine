#include "PhysXShapes.h"

#include "PhysXEngine.h"
#include "PhysXMaterial.h"

#include <PxPhysics.h>

#include <Debug.h>

leap::physics::PhysXBoxShape::PhysXBoxShape(PhysXEngine* pEngine, void* pOwner, PhysXMaterial* pMaterial)
{
	physx::PxBoxGeometry geo{ 0.5f, 0.5f, 0.5f };

	m_pShape = pEngine->GetPhysics()->createShape(geo, pMaterial->GetInternalMaterial(), true);
	m_pShape->userData = pOwner;
}

void leap::physics::PhysXBoxShape::SetSize(const glm::vec3& size)
{
	m_pShape->setGeometry(physx::PxBoxGeometry{ size.x / 2.0f, size.y / 2.0f, size.z / 2.0f });
}

void leap::physics::PhysXBoxShape::SetRadius(float)
{
	Debug::LogWarning("PhysXEngine Warning: Cannot set radius on a box shape");
}

float leap::physics::PhysXBoxShape::GetVolume()
{
	const auto& boxGeometry{ m_pShape->getGeometry().box() };
	const physx::PxVec3& halfExtents{ boxGeometry.halfExtents };
	return 8.0f * halfExtents.x * halfExtents.y * halfExtents.z;
}

void leap::physics::PhysXBoxShape::SetRelativeTransform(const glm::vec3& position, const glm::quat& rotation)
{
	m_pShape->setLocalPose(physx::PxTransform{ physx::PxVec3{ position.x, position.y, position.z }, physx::PxQuat{ rotation.x, rotation.y, rotation.z,rotation.w } });
}

glm::vec3 leap::physics::PhysXBoxShape::GetRelativePosition()
{
	const physx::PxVec3 localPose{ m_pShape->getLocalPose().p };
	return glm::vec3{ localPose.x, localPose.y, localPose.z };
}

leap::physics::PhysXSphereShape::PhysXSphereShape(PhysXEngine* pEngine, void* pOwner, PhysXMaterial* pMaterial)
{
	physx::PxSphereGeometry geo{ 0.5f };

	m_pShape = pEngine->GetPhysics()->createShape(geo, pMaterial->GetInternalMaterial(), true);
	m_pShape->userData = pOwner;
}

void leap::physics::PhysXSphereShape::SetRadius(float radius)
{
	m_pShape->setGeometry(physx::PxSphereGeometry{ radius });
}

float leap::physics::PhysXSphereShape::GetVolume()
{
	return 4.0f / 3.0f * glm::pi<float>() * powf(m_pShape->getGeometry().sphere().radius, 3.0f);
}

void leap::physics::PhysXSphereShape::SetRelativeTransform(const glm::vec3& position, const glm::quat& rotation)
{
	m_pShape->setLocalPose(physx::PxTransform{ physx::PxVec3{ position.x, position.y, position.z }, physx::PxQuat{ rotation.x, rotation.y, rotation.z,rotation.w } });
}

glm::vec3 leap::physics::PhysXSphereShape::GetRelativePosition()
{
	const physx::PxVec3 localPose{ m_pShape->getLocalPose().p };
	return glm::vec3{ localPose.x, localPose.y, localPose.z };
}

void leap::physics::PhysXSphereShape::SetSize(const glm::vec3&)
{
	Debug::LogWarning("PhysXEngine Warning: Cannot set size on a sphere shape");
}

leap::physics::PhysXCapsuleShape::PhysXCapsuleShape(PhysXEngine* pEngine, void* pOwner, PhysXMaterial* pMaterial)
{
	physx::PxCapsuleGeometry geo{ 0.5f, 1.0f };

	m_pShape = pEngine->GetPhysics()->createShape(geo, pMaterial->GetInternalMaterial(), true);
	m_pShape->setLocalPose(physx::PxTransform{ {}, physx::PxQuat{ physx::PxHalfPi, physx::PxVec3{ 0.0f, 0.0f, 1.0f } } });
	m_pShape->userData = pOwner;
}

void leap::physics::PhysXCapsuleShape::SetSize(const glm::vec3& size)
{
	m_pShape->setGeometry(physx::PxCapsuleGeometry{ m_pShape->getGeometry().capsule().radius, size.y / 2.0f });
}

void leap::physics::PhysXCapsuleShape::SetRadius(float radius)
{
	m_pShape->setGeometry(physx::PxCapsuleGeometry{ radius, m_pShape->getGeometry().capsule().halfHeight });
}

float leap::physics::PhysXCapsuleShape::GetVolume()
{
	const physx::PxCapsuleGeometry& geo{ m_pShape->getGeometry().capsule() };
	return glm::pi<float>() * powf(geo.radius, 2.0f) * (4.0f / 3.0f * geo.radius * geo.halfHeight * 2.0f);
}

void leap::physics::PhysXCapsuleShape::SetRelativeTransform(const glm::vec3& position, const glm::quat& rotation)
{
	const physx::PxQuat pxRotation{ physx::PxQuat{ rotation.x, rotation.y, rotation.z,rotation.w } };
	m_pShape->setLocalPose(physx::PxTransform{ physx::PxVec3{ position.x, position.y, position.z }, physx::PxQuat{ physx::PxHalfPi, physx::PxVec3{ 0.0f, 0.0f, 1.0f } } * pxRotation });
}

glm::vec3 leap::physics::PhysXCapsuleShape::GetRelativePosition()
{
	const physx::PxVec3 localPose{ m_pShape->getLocalPose().p };
	return glm::vec3{ localPose.x, localPose.y, localPose.z };
}

leap::physics::IPhysXShape::~IPhysXShape()
{
	m_pShape->release();
}

physx::PxShape& leap::physics::IPhysXShape::GetShape()
{
	return *m_pShape;
}

void leap::physics::IPhysXShape::SetTrigger(bool isTrigger)
{
	m_pShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
	m_pShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger);
}
