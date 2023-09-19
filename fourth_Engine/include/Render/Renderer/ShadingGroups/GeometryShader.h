#pragma once

struct ID3D11GeometryShader;
struct ID3D11InputLayout;
struct D3D11_INPUT_ELEMENT_DESC;

namespace fth
{
	namespace renderer
	{
		class GeometryShader
		{
		public:

			void LoadShader(std::wstring_view shaderName);
			void Bind();
			void Clear();
			inline bool Valid() { return m_geometryShader.valid(); }

		private:
			DxResPtr<ID3D11GeometryShader>  m_geometryShader;
		};
	}
}