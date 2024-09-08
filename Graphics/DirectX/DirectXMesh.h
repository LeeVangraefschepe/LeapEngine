#pragma once

#include "../Interfaces/IMesh.h"
#include <string>

struct ID3D11Buffer;

namespace leap::graphics
{
	class DirectXEngine;
	class DirectXMeshRenderer;

	class DirectXMesh final : public IMesh
	{
	public:
		DirectXMesh(DirectXEngine* pEngine);
		DirectXMesh(DirectXEngine* pEngine, const std::string& filePath);

		virtual ~DirectXMesh();

		DirectXMesh(const DirectXMesh& other) = delete;
		DirectXMesh(DirectXMesh&& other) = delete;
		DirectXMesh& operator=(const DirectXMesh& other) = delete;
		DirectXMesh& operator=(DirectXMesh&& other) = delete;

		virtual std::vector<unsigned> GetIndices() override;
		virtual std::vector<Vertex> GetVertices() override;

		virtual bool IsEmpty() override;
		virtual void ReloadMesh(const CustomMesh& mesh) override;
		virtual void Remove() override;

	private:
		void Release();

		friend DirectXMeshRenderer;

		unsigned int m_NrVertices{};
		unsigned int m_VertexSize{};
		ID3D11Buffer* m_pVertexBuffer{};
		unsigned int m_NrIndices{};
		ID3D11Buffer* m_pIndexBuffer{};

		DirectXEngine* m_pEngine{};
	};
}