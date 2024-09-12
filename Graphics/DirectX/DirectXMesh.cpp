#include "DirectXMesh.h"

#include "DirectXEngine.h"

#include "../Data/CustomMesh.h"
#include "../MeshLoader.h"
#include "Vertex.h"

#include <vector>

#include <d3d11.h>

#include <Debug.h>

leap::graphics::DirectXMesh::DirectXMesh(DirectXEngine* pEngine)
	: m_pEngine{ pEngine }
{
}

leap::graphics::DirectXMesh::DirectXMesh(DirectXEngine* pEngine, const std::string& filePath)
	: DirectXMesh(pEngine)
{
	std::vector<Vertex> vertices{};
	std::vector<unsigned int> indices{};

	// Load obj
	MeshLoader::ParseObj(filePath, vertices, indices);

	// Set index amount and vertex size
	m_NrIndices = static_cast<unsigned int>(indices.size());
	m_NrVertices = static_cast<unsigned int>(vertices.size());
	m_VertexSize = sizeof(Vertex);

	// Create Vertex Buffer
	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.Usage = D3D11_USAGE_STAGING;  // Staging allows CPU read/write access
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * m_NrVertices;
	vertexBufferDesc.BindFlags = 0;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
	vertexBufferDesc.MiscFlags = 0;

	// Create vertex buffer without initial data
	HRESULT result = pEngine->GetDevice()->CreateBuffer(&vertexBufferDesc, nullptr, &m_pVertexBuffer);
	if (FAILED(result))
	{
		Debug::LogError("DirectXEngine Error : Failed to create vertex buffer from obj");
		return;
	}

	// Map/Unmap to populate vertex buffer with initial data
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	result = pEngine->GetContext()->Map(m_pVertexBuffer, 0, D3D11_MAP_READ, 0, &mappedResource);
	if (SUCCEEDED(result))
	{
		// Copy vertex data to the buffer
		memcpy(mappedResource.pData, vertices.data(), sizeof(Vertex) * m_NrVertices);
		pEngine->GetContext()->Unmap(m_pVertexBuffer, 0);
	}

	// Create Index Buffer
	D3D11_BUFFER_DESC indexBufferDesc{};
	indexBufferDesc.Usage = D3D11_USAGE_STAGING;  // Staging allows CPU read/write access
	indexBufferDesc.ByteWidth = sizeof(unsigned int) * m_NrIndices;
	indexBufferDesc.BindFlags = 0;  // No bind flags for staging buffers
	indexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
	indexBufferDesc.MiscFlags = 0;

	// Create index buffer without initial data
	result = pEngine->GetDevice()->CreateBuffer(&indexBufferDesc, nullptr, &m_pIndexBuffer);
	if (FAILED(result))
	{
		Debug::LogError("DirectXEngine Error : Failed to create index buffer from obj");
		return;
	}

	// Map/Unmap to populate index buffer with initial data
	result = pEngine->GetContext()->Map(m_pIndexBuffer, 0, D3D11_MAP_READ, 0, &mappedResource);
	if (SUCCEEDED(result))
	{
		// Copy index data to the buffer
		memcpy(mappedResource.pData, indices.data(), sizeof(unsigned int) * m_NrIndices);
		pEngine->GetContext()->Unmap(m_pIndexBuffer, 0);
	}
}

std::vector<unsigned> leap::graphics::DirectXMesh::GetIndices()
{
	// Create vertices buffer to store the CPU buffer in
	std::vector<unsigned> indices{};

	// Ask GPU to map out the vertex buffer
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	const HRESULT result = m_pEngine->GetContext()->Map(m_pIndexBuffer, 0, D3D11_MAP_READ, 0, &mappedResource);

	if (SUCCEEDED(result))
	{
		// Get the first vertex pointer
		const unsigned int* pIndices = static_cast<unsigned int*>(mappedResource.pData);

		// Copy GPU buffer to CPU buffer
		indices.resize(m_NrIndices);
		memcpy(indices.data(), pIndices, m_NrIndices * sizeof(unsigned int));

		// Unmap the buffer after reading
		m_pEngine->GetContext()->Unmap(m_pIndexBuffer, 0);
	}
	else
	{
		Debug::LogError("DirectXEngine Error : Failed to map vertex buffer for reading.");
	}

	// Return the CPU buffer
	return indices;
}

std::vector<leap::Vertex> leap::graphics::DirectXMesh::GetVertices()
{
	// Create vertices buffer to store the CPU buffer in
	std::vector<Vertex> vertices{};

	// Ask GPU to map out the vertex buffer
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	const HRESULT result = m_pEngine->GetContext()->Map(m_pVertexBuffer, 0, D3D11_MAP_READ, 0, &mappedResource);

	if (SUCCEEDED(result))
	{
		// Get the first vertex pointer
		const Vertex* pVertices = static_cast<Vertex*>(mappedResource.pData);

		// Copy GPU buffer to CPU buffer
		vertices.resize(m_NrVertices);
		memcpy(vertices.data(), pVertices, m_NrVertices * sizeof(Vertex));

		// Unmap the buffer after reading
		m_pEngine->GetContext()->Unmap(m_pVertexBuffer, 0);
	}
	else
	{
		Debug::LogError("DirectXEngine Error : Failed to map vertex buffer for reading.");
	}

	// Return the CPU buffer
	return vertices;
}

bool leap::graphics::DirectXMesh::IsEmpty()
{
	return m_NrIndices == 0 || m_VertexSize == 0;
}

void leap::graphics::DirectXMesh::ReloadMesh(const CustomMesh& mesh)
{
	Release();

	// Keep the mesh empty if there is no vertex buffer given in the mesh
	if (mesh.GetVertexBuffer().empty()) return;

	// Set index amoutn and vertex size
	m_NrIndices = static_cast<unsigned int>(mesh.GetIndexBuffer().size());
	m_VertexSize = mesh.GetVertexSize();

	// Create vertex buffer
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = static_cast<unsigned>(mesh.GetVertexBuffer().size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = mesh.GetVertexBuffer().data();

	HRESULT result{ m_pEngine->GetDevice()->CreateBuffer(&bd, &initData, &m_pVertexBuffer) };
	if (FAILED(result))
	{
		Debug::LogError("DirectXEngine Error : Failed to create vertex buffer from obj");
		return;
	}

	// Create index buffer
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(unsigned int) * static_cast<unsigned int>(m_NrIndices);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = mesh.GetIndexBuffer().data();

	result = m_pEngine->GetDevice()->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result)) Debug::LogError("DirectXEngine Error : Failed to create index buffer from obj");
}

void leap::graphics::DirectXMesh::Remove()
{
	m_pEngine->RemoveMesh(this);
}

void leap::graphics::DirectXMesh::Release()
{
	if(m_pIndexBuffer) m_pIndexBuffer->Release();
	m_pIndexBuffer = nullptr;
	if(m_pVertexBuffer) m_pVertexBuffer->Release();
	m_pVertexBuffer = nullptr;
	m_NrIndices = 0;
	m_NrVertices = 0;
	m_VertexSize = 0;
}

leap::graphics::DirectXMesh::~DirectXMesh()
{
	Release();
}