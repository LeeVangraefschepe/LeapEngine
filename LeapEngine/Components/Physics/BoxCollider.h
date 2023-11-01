#pragma once

#include "Collider.h"

#include <vec3.hpp>

namespace leap
{
	class BoxCollider final : public Collider
	{
	public:
		BoxCollider() = default;
		virtual ~BoxCollider() = default;

		BoxCollider(const BoxCollider& other) = delete;
		BoxCollider(BoxCollider&& other) = delete;
		BoxCollider& operator=(const BoxCollider& other) = delete;
		BoxCollider& operator=(BoxCollider&& other) = delete;

		void SetSize(const glm::vec3& size);
		void SetSize(float x, float y, float z);
		void SetSize(float size);

	private:
		virtual void SetupShape(physics::IPhysicsMaterial* pMaterial) override;

		glm::vec3 m_Size{ 1.0f, 1.0f, 1.0f };
	};
}