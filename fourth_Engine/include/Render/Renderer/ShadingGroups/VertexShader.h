#pragma once


struct ID3D11VertexShader;
struct ID3D11InputLayout;
struct D3D11_INPUT_ELEMENT_DESC;

namespace fth
{
	namespace renderer
	{
		class VertexShader
		{
		public:
			void LoadShader(std::wstring_view shaderName, const D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT descSize);
			void Bind();
			void Reset();

		private:
			DxResPtr<ID3D11VertexShader>  m_vertexShader;
			DxResPtr<ID3D11InputLayout>   m_inputLayout;

		};
	}
}