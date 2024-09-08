#pragma once

#include "vec4.hpp"
#include "vec3.hpp"
#include "vec2.hpp"

namespace leap
{
	struct Vertex final
	{
		glm::vec3 position{};
		glm::vec3 normal{};
		glm::vec2 uv{};
		glm::vec3 tangent{};
	};

	struct VertexPosUV
	{
		glm::vec3 position{};
		glm::vec2 uv{};
	};
}