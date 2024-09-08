#include "TerrainComponent.h"

#include <Interfaces/ITexture.h>
#include <Interfaces/IRenderer.h>
#include <Interfaces/IMeshRenderer.h>
#include <Vertex.h>
#include <Shaders/Heightmap.h>

#include "../../ServiceLocator/ServiceLocator.h"
#include "../Transform/Transform.h"

#include "../../Graphics/Shader.h"

#include <sstream>

leap::TerrainComponent::TerrainComponent()
{
	// Create a mesh renderer
	m_pRenderer = ServiceLocator::GetRenderer().CreateMeshRenderer();

	// Create a material using the heightmap shader
	m_pMaterial = Material::CreatePtr<graphics::shaders::Heightmap>("Heightmap", true);

	// Apply the material to the mesh renderer
	m_pRenderer->SetMaterial(m_pMaterial->GetInternal());
}

leap::TerrainComponent::~TerrainComponent()
{
	// Delete the currently used mesh
	auto& meshData{ m_Meshes[m_Size] };
	--meshData.useCounter;
	if (meshData.useCounter == 0)
	{
		m_Meshes.erase(m_Size);
	}
	else
	{
		m_pRenderer->UnsetMesh();
	}

	// Delete the mesh renderer
	ServiceLocator::GetRenderer().RemoveMeshRenderer(m_pRenderer);
}

void leap::TerrainComponent::SetSize(unsigned int widthInQuads, float quadSize)
{
	// Apply the quad size
	GetTransform()->SetLocalScale(quadSize, 1.0f, quadSize);

	// Make sure the shader is using UINT values
	m_pMaterial->Set("gRGABAsUINT", true);

	if (m_Size == widthInQuads) return;

	unsigned int prevSize{ m_Size };
	m_Size = widthInQuads;

	ApplySizeTexture();
	ApplySizeMesh(prevSize);
}

void leap::TerrainComponent::LoadHeightmap(const std::string& path, bool isShared, float quadSize)
{
	m_Texture.Load(path, !isShared);

	const auto& textureSize{ m_Texture.GetSize() };
	if (textureSize.x != textureSize.y)
	{
		Debug::LogError("LeapEngine Error : Cannot create terrain with a texture which different width and height values");
		return;
	}

	m_pMaterial->Set("gHeightMap", m_Texture);

	// Make sure the shader is using 8-bit RGBA values
	m_pMaterial->Set("gRGABAsUINT", false);

	// Apply quad size
	GetTransform()->SetLocalScale(quadSize, 1.0f, quadSize);

	const unsigned int meshSize{ textureSize.x - 1 };
	if (m_Size == meshSize) return;

	const unsigned int prevSize{ m_Size };
	m_Size = meshSize;

	ApplySizeMesh(prevSize);
}

std::vector<float> leap::TerrainComponent::GetHeights() const
{
	// Create a vector to store height data
	std::vector<float> heights{};

	// Get the internal vector of bytes from the texture
	const std::vector<unsigned char> textureData{ m_Texture.GetData() };

	// Resize the heightmap to store all the height information from the texture
	heights.resize(textureData.size() / sizeof(float));

	// For each pixel
	for (size_t i{}; i < heights.size(); ++i)
	{
		// Get the adress of the current RGBA data
		const unsigned char* pByteData{ textureData.data() + i * sizeof(unsigned int) };

		// Read the 8-bit per color RGBA value as an unsigned integer 
		const unsigned int intHeight{ *reinterpret_cast<const unsigned int*>(pByteData) };

		// Map the height to a 0.0 -> 1.0 float value and store it in the heightmap
		const float height{ static_cast<float>(intHeight) / UINT_MAX };
		heights[i] = height;
	}

	return heights;
}

void leap::TerrainComponent::SetHeights(const std::vector<float>& heights)
{
	// Create a vector for the RGBA data
	std::vector<unsigned char> textureData{};
	textureData.resize(heights.size() * sizeof(unsigned int));

	// A double is required to prevent 1.0f * UINT_MAX from overflowing to 0
	constexpr double uintMax{ UINT_MAX };

	// For each pixel
	for (size_t i{}; i < heights.size(); ++i)
	{
		// Get the height value
		const float height{ heights[i] };

		// Map the height to a unsigned integer value (0 -> UINT_MAX)
		const unsigned int intHeight{ static_cast<unsigned int>(height * uintMax) };

		// Calculate the index of the current pixel in the RGBA data
		const size_t byteIdx{ i * sizeof(unsigned int) };

		// Store the integer height value in the RGBA pixel data
		memcpy(textureData.data() + byteIdx, &intHeight, sizeof(unsigned int));
	}

	// Store the raw data in the texture
	m_Texture.SetData(textureData);
}

void leap::TerrainComponent::Awake()
{
	m_pRenderer->SetTransform(GetTransform()->GetWorldTransform());

	GetTransform()->OnPositionChanged.AddListener(this, &TerrainComponent::UpdateTransform);
}

void leap::TerrainComponent::UpdateTransform() const
{
	m_pRenderer->SetTransform(GetTransform()->GetWorldTransform());
}

void leap::TerrainComponent::ApplySizeTexture()
{
	m_Texture.Load(m_Size + 1, m_Size + 1);
	m_pMaterial->Set("gHeightMap", m_Texture);
}

void leap::TerrainComponent::ApplySizeMesh(unsigned int prevSize)
{
	if (!m_Meshes.contains(m_Size)) CreateMesh();

	TerrainMesh& mesh{ m_Meshes[m_Size] };
	++mesh.useCounter;

	TerrainMesh& prevMesh{ m_Meshes[prevSize] };
	--prevMesh.useCounter;
	if (prevMesh.useCounter <= 0) m_Meshes.erase(prevSize);

	m_pRenderer->SetMesh(mesh.mesh.GetInternal());
}

void leap::TerrainComponent::CreateMesh()
{
	Mesh mesh{};

	mesh.SetWritable(true);

	std::vector<leap::VertexPosUV> vertices{};
	std::vector<unsigned> indices{};

	for (unsigned int x{}; x < m_Size; ++x)
	{
		for (unsigned int y{}; y < m_Size; ++y)
		{
			const unsigned int startIdx{ static_cast<unsigned>(vertices.size()) };

			vertices.emplace_back(leap::VertexPosUV{ { x, 0.0f, y + 1.0f }, { x, y + 1.0f } });
			vertices.emplace_back(leap::VertexPosUV{ { x + 1.0f, 0.0f, y + 1.0f }, { x + 1.0f, y + 1.0f } });
			vertices.emplace_back(leap::VertexPosUV{ { x + 1.0f, 0.0f, y }, { x + 1.0f, y } });
			vertices.emplace_back(leap::VertexPosUV{ { x, 0.0f, y }, { x, y } });

			indices.emplace_back(startIdx);
			indices.emplace_back(startIdx + 1);
			indices.emplace_back(startIdx + 2);

			indices.emplace_back(startIdx);
			indices.emplace_back(startIdx + 2);
			indices.emplace_back(startIdx + 3);
		}
	}

	mesh.SetData(vertices, std::move(indices));

	m_Meshes[m_Size] = TerrainMesh{ 0, std::move(mesh) };
}
