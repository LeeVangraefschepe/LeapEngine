#pragma once
#include <vector>
#include "Vertex.h"

namespace leap::graphics
{
	class CustomMesh;

	class IMesh
	{
	public:
		virtual ~IMesh() = default;

		virtual bool IsEmpty() = 0;
		virtual std::vector<unsigned> GetIndices() = 0;
		virtual std::vector<Vertex> GetVertices() = 0;
		virtual void ReloadMesh(const CustomMesh& mesh) = 0;
		virtual void Remove() = 0;
	};
}
