#pragma once

struct ID3D11HullShader;

namespace fth
{
	namespace renderer
	{
		class HullShader
		{
		public:
			void LoadShader(std::wstring shaderName);
			void Bind();
			void Clear();
		private:
			DxResPtr<ID3D11HullShader>  m_hullShader;
		};
	}
}