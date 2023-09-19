#pragma once

namespace fth
{
	struct Vertex
	{
		static const Vertex Zero;
		Vertex() = default;
		Vertex(const math::Vector3& position, const math::Vector3& normal, const math::Vector3& tangent, const math::Vector3& bitangent, const math::Vector2& txUV) :
			position(position),
			normal(normal),
			tangent(tangent),
			bitangent(bitangent),
			textureUV(txUV)
		{}
		Vertex(math::Vector3&& position, math::Vector3&& normal, math::Vector3&& tangent, math::Vector2&& bitangent, math::Vector2&& txUV):
			position(position), normal(normal), tangent(tangent), bitangent(bitangent), textureUV(txUV)
		{

		}

		math::Vector3                        position;
		math::Vector3                        normal;
		math::Vector3                        tangent;
		math::Vector3                        bitangent;
		math::Vector2                        textureUV;
	};

	struct TriangleIndexed
	{
		union
		{
			uint32_t indexes[3];

			struct  {
				uint32_t index1;
				uint32_t index2;
				uint32_t index3;
			}triangle;

		};
		TriangleIndexed() = default;
		TriangleIndexed(uint32_t index1, uint32_t index2, uint32_t index3) 
		{
			triangle.index1 = index1;
			triangle.index2 = index2;
			triangle.index3 = index3;
		};

	};
}
