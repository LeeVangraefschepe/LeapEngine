#pragma once

#include <vec3.hpp>
#include <vector>
#include "Vertex.h"

#pragma warning(disable: 4201)
#include "gtc/quaternion.hpp"
#pragma warning(default: 4201)

namespace leap::physics
{
	enum class EShape
	{
		Box,
		Sphere,
		Capsule,
		Mesh
	};

	class IShape
	{
	public:
		virtual ~IShape() = default;

		virtual void SetSize(const glm::vec3& size) = 0;
		virtual void SetRadius(float radius) = 0;
		virtual float GetVolume() = 0;
		virtual void SetTrigger(bool isTrigger) = 0;
		virtual void SetMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices) = 0;
		
		virtual void SetRelativeTransform(const glm::vec3& position, const glm::quat& rotation) = 0;
		virtual glm::vec3 GetRelativePosition() = 0;
	};
}