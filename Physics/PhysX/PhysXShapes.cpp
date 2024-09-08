#include "PhysXShapes.h"

#include "PhysXEngine.h"
#include "PhysXMaterial.h"

#include <PxPhysics.h>

#include <Debug.h>

#include "PxPhysicsVersion.h"
#include "cooking/PxConvexMeshDesc.h"
#include "cooking/PxCooking.h"

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

leap::physics::PhysXMeshShape::PhysXMeshShape(PhysXEngine* pEngine, void* pOwner, PhysXMaterial* pMaterial)
{
	m_pEngine = pEngine;
	m_pMaterial = pMaterial;
	m_pOwner = pOwner;
}

void leap::physics::PhysXMeshShape::SetMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices)
{
	// Step 1: Convert vertices and indices to PhysX-friendly format
	std::vector<physx::PxVec3> physxVertices;
	std::vector<physx::PxU32> physxIndices;
	for (const Vertex& vertex : vertices)
	{
		physxVertices.emplace_back(vertex.position.x, vertex.position.y, vertex.position.z);
	}
	for (unsigned indice : indices)
	{
		physxIndices.emplace_back(indice);
	}

	// Step 2: Create ConvexMesh using the vertices and indices
	physx::PxConvexMeshDesc convexDesc;
	convexDesc.points.count = static_cast<physx::PxU32>(physxVertices.size());
	convexDesc.points.stride = sizeof(physx::PxVec3);
	convexDesc.points.data = physxVertices.data();

	convexDesc.indices.count = static_cast<physx::PxU32>(physxIndices.size());
	convexDesc.indices.stride = sizeof(physx::PxU32);
	convexDesc.indices.data = physxIndices.data();

	// Request convex meshes 
	convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

	// Get PxCooking instance
	const physx::PxCooking* cooking = m_pEngine->GetCooking();

	// Cook convex mesh
	physx::PxConvexMesh* convexMesh = cooking->createConvexMesh(convexDesc, m_pEngine->GetPhysics()->getPhysicsInsertionCallback());

	if (!convexMesh)
	{
		// Handle the error (convex mesh creation failed)
		Debug::LogError("PhysX 3 : Failed cooking mesh");
		return;
	}

	// Create shape and assign user
	m_pShape = m_pEngine->GetPhysics()->createShape(physx::PxConvexMeshGeometry{ convexMesh }, m_pMaterial->GetInternalMaterial(), true);
	m_pShape->userData = m_pOwner;
}

void leap::physics::PhysXMeshShape::SetRelativeTransform(const glm::vec3& position, const glm::quat& rotation)
{
	const physx::PxQuat pxRotation{ physx::PxQuat{ rotation.x, rotation.y, rotation.z,rotation.w } };
	m_pShape->setLocalPose(physx::PxTransform{ physx::PxVec3{ position.x, position.y, position.z }, physx::PxQuat{ physx::PxHalfPi, physx::PxVec3{ 0.0f, 0.0f, 1.0f } } *pxRotation });
}

glm::vec3 leap::physics::PhysXMeshShape::GetRelativePosition()
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
