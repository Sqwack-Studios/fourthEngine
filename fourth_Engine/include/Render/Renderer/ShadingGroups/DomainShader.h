#pragma once

struct ID3D11DomainShader;


namespace fth
{
	namespace renderer
	{
		class DomainShader
		{
		public:
			void LoadShader(std::wstring shaderName);
			void Bind();
			void Clear();
		private:
			DxResPtr<ID3D11DomainShader>  m_domainShader;
		};
	}
}