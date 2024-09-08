#include "Mesh.h"

#include <Interfaces/IMesh.h>
#include <Interfaces/IRenderer.h>

#include "../ServiceLocator/ServiceLocator.h"

#include <Debug.h>

leap::Mesh::Mesh()
	: m_pMesh{ std::make_unique<GraphicsObject<graphics::IMesh>>() }
{
}

leap::Mesh::Mesh(const std::string& filePath, bool unique)
	: Mesh{}
{
	Load(filePath, unique);
}

void leap::Mesh::SetWritable(bool isWritable)
{
	// Don't change the state if the state is already correct
	if (isWritable == IsWritable()) return;

	if (!isWritable)
	{
		// Delete any writable mesh
		m_pWritableMesh = nullptr;
		return;
	}

	// Clear any previously loaded mesh
	m_pMesh->Clear();

	// Create a new writable mesh
	m_pWritableMesh = std::make_unique<WritableMesh>();
}

bool leap::Mesh::IsEmpty() const
{
	if (!m_pMesh->IsValid()) return true;
	return m_pMesh->GetObject()->IsEmpty();
}

std::vector<unsigned> leap::Mesh::GetIndices() const
{
	if (m_pMesh->IsValid()) return m_pMesh->GetObject()->GetIndices();
	return std::vector<unsigned>{};
}

std::vector<leap::Vertex> leap::Mesh::GetVertices() const
{
	if (m_pMesh->IsValid()) return m_pMesh->GetObject()->GetVertices();
	return std::vector<Vertex>{};
}

void leap::Mesh::Load(const std::string& filePath, bool unique)
{
	// Delete any writable mesh
	m_pWritableMesh = nullptr;

	// Load mesh
	graphics::IMesh* pMesh{ ServiceLocator::GetRenderer().CreateMesh(filePath, !unique) };
	m_pMesh->SetObject(pMesh);
}

leap::graphics::IMesh* leap::Mesh::GetInternal() const
{
	// If mesh is not writable, return stored mesh
	if (!m_pWritableMesh) return m_pMesh ? m_pMesh->GetObject() : nullptr;

	// If mesh is writable but there is no mesh created yet, create an empty mesh
	if (!m_pMesh->IsValid())
	{
		graphics::IMesh* pEmptyMesh{ ServiceLocator::GetRenderer().CreateMesh() };
		m_pMesh->SetObject(pEmptyMesh);
	}

	graphics::IMesh* pMesh{ m_pMesh->GetObject() };

	// Reload the vertices/indices of the mesh if the writable mesh has changed
	if (m_pWritableMesh->isDirty)
	{
		pMesh->ReloadMesh(m_pWritableMesh->mesh);
		m_pWritableMesh->isDirty = false;
	}

	return pMesh;
}
