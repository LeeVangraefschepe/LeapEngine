#pragma once

#include <vector>

#include "../Interfaces/IShape.h"
#include "Vertex.h"

namespace physx
{
	class PxShape;
}

namespace leap::physics
{
	class PhysXEngine;
	class PhysXMaterial;

	class IPhysXShape : public IShape
	{
	public:
		IPhysXShape() = default;
		virtual ~IPhysXShape();

		IPhysXShape(const IPhysXShape& other) = delete;
		IPhysXShape(IPhysXShape&& other) = delete;
		IPhysXShape& operator=(const IPhysXShape& other) = delete;
		IPhysXShape& operator=(IPhysXShape&& other) = delete;

		physx::PxShape& GetShape();
		virtual void SetTrigger(bool isTrigger) override;

	protected:
		physx::PxShape* m_pShape{};
	};

	class PhysXBoxShape final : public IPhysXShape
	{
	public:
		PhysXBoxShape(PhysXEngine* pEngine, void* pOwner, PhysXMaterial* pMaterial);
		virtual ~PhysXBoxShape() = default;

		void SetMesh(const std::vector<Vertex>&, const std::vector<unsigned>&) override {}
		virtual void SetSize(const glm::vec3& size) override;
		virtual void SetRadius(float radius) override;
		virtual float GetVolume() override;

		virtual void SetRelativeTransform(const glm::vec3& position, const glm::quat& rotation) override;
		virtual glm::vec3 GetRelativePosition() override;
	};

	class PhysXSphereShape final : public IPhysXShape
	{
	public:
		PhysXSphereShape(PhysXEngine* pEngine, void* pOwner, PhysXMaterial* pMaterial);
		virtual ~PhysXSphereShape() = default;

		void SetMesh(const std::vector<Vertex>&, const std::vector<unsigned>&) override {}
		virtual void SetSize(const glm::vec3& size) override;
		virtual void SetRadius(float radius) override;
		virtual float GetVolume() override;

		virtual void SetRelativeTransform(const glm::vec3& position, const glm::quat& rotation) override;
		virtual glm::vec3 GetRelativePosition() override;
	};

	class PhysXCapsuleShape final : public IPhysXShape
	{
	public:
		PhysXCapsuleShape(PhysXEngine* pEngine, void* pOwner, PhysXMaterial* pMaterial);
		virtual ~PhysXCapsuleShape() = default;

		void SetMesh(const std::vector<Vertex>&, const std::vector<unsigned>&) override {}
		virtual void SetSize(const glm::vec3& size) override;
		virtual void SetRadius(float radius) override;
		virtual float GetVolume() override;

		virtual void SetRelativeTransform(const glm::vec3& position, const glm::quat& rotation) override;
		virtual glm::vec3 GetRelativePosition() override;
	};

	class PhysXMeshShape final : public IPhysXShape
	{
	public:
		PhysXMeshShape(PhysXEngine* pEngine, void* pOwner, PhysXMaterial* pMaterial);
		virtual ~PhysXMeshShape() = default;

		virtual void SetMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices) override;
		virtual void SetSize(const glm::vec3&) override{}
		virtual void SetRadius(float) override{}
		virtual float GetVolume() override { return 0.f; }

		virtual void SetRelativeTransform(const glm::vec3& position, const glm::quat& rotation) override;
		virtual glm::vec3 GetRelativePosition() override;
	private:
		void* m_pOwner{};
		PhysXMaterial* m_pMaterial{};
		PhysXEngine* m_pEngine{};
	};
}