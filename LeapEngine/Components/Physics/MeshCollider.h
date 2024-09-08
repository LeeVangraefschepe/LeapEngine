#pragma once
#include "Collider.h"
#include "../../Graphics/Mesh.h"

namespace leap
{
	class MeshCollider final : public Collider
	{
	public:
		MeshCollider() = default;
		virtual ~MeshCollider() = default;

		void SetMesh(const Mesh& mesh);

		MeshCollider(const MeshCollider& other) = delete;
		MeshCollider(MeshCollider&& other) = delete;
		MeshCollider& operator=(const MeshCollider& other) = delete;
		MeshCollider& operator=(MeshCollider&& other) = delete;

	private:
		virtual void SetupShape(physics::IPhysicsMaterial* pMaterial) override;
		virtual void RescaleShape() override;

		std::vector<Vertex> m_Vertices{};
		std::vector<unsigned> m_Indices{};
	};
}