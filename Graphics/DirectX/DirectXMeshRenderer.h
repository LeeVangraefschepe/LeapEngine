#pragma once

#include "../Interfaces/IMeshRenderer.h"

#include "mat4x4.hpp"
#include <Matrix.h>

#include <d3dcommon.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11InputLayout;
struct ID3D11Buffer;

namespace leap::graphics
{
	class DirectXEngine;
	class IMaterial;
	class DirectXMaterial;
	class DirectXMesh;

	class DirectXMeshRenderer final : public IMeshRenderer
	{
	public:
		DirectXMeshRenderer(DirectXEngine* pEngine);
		virtual ~DirectXMeshRenderer();

		DirectXMeshRenderer(const DirectXMeshRenderer& other) = delete;
		DirectXMeshRenderer(DirectXMeshRenderer&& other) = delete;
		DirectXMeshRenderer& operator=(const DirectXMeshRenderer& other) = delete;
		DirectXMeshRenderer& operator=(DirectXMeshRenderer&& other) = delete;

		virtual void Draw() override;
		virtual void Draw(IMaterial* pMaterial) override;
		virtual IMaterial* GetMaterial() override;
		virtual void SetMaterial(IMaterial* pMaterial) override;
		virtual void UnsetMaterial() override;
		virtual void SetTransform(const glm::mat4x4& transform) override;
		virtual void SetMesh(IMesh* pMesh) override;
		virtual IMesh* GetMesh() override;
		virtual void UnsetMesh() override;
		virtual void SetAsPointRenderer() override;
		virtual void SetAsLineRenderer() override;
		virtual void SetAsTriangleRenderer() override;

	private:
		DirectXMesh* m_pMesh{};
		DirectXMaterial* m_pMaterial{};

		DirectXEngine* m_pEngine{};

		glm::mat4x4 m_Transform{ Matrix::Identity4x4() };

		D3D_PRIMITIVE_TOPOLOGY m_TopologyType{ D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST };
	};
}