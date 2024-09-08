#include "MeshCollider.h"

#include "../../ServiceLocator/ServiceLocator.h"
#include "../../SceneGraph/GameObject.h"

#include "../Transform/Transform.h"

#include <Interfaces/IPhysics.h>
#include <Interfaces/IPhysicsObject.h>
#include <Interfaces/IShape.h>

#include "Debug.h"
#include "Vertex.h"

void leap::MeshCollider::SetMesh(const Mesh& mesh)
{
	m_Indices = mesh.GetIndices();
	m_Vertices = mesh.GetVertices();
}

void leap::MeshCollider::SetupShape(physics::IPhysicsMaterial* pMaterial)
{
	if (m_Vertices.empty() || m_Indices.empty())
	{
		Debug::LogWarning("Failed to create mesh shape. No mesh was given upon creation. Please use SetMesh to assign a mesh to the collider when creating it.");
		GetGameObject()->RemoveComponent<MeshCollider>();
		return;
	}

	physics::IPhysics& physics{ ServiceLocator::GetPhysics() };
	m_pShape = physics.CreateShape(this, physics::EShape::Mesh, pMaterial);
	if (!m_pShape) return;
	m_pShape->SetMesh(m_Vertices, m_Indices);
}

void leap::MeshCollider::RescaleShape()
{
	if (m_pShape) m_pShape->SetSize(GetTransform()->GetWorldScale());
}
