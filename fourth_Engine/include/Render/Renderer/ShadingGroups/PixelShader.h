#pragma once


struct ID3D11PixelShader;

namespace fth
{
	namespace renderer
	{
		class PixelShader
		{
		public:
			void LoadShader(std::wstring_view shaderName);
			void Bind() const;
			void Reset();

			static void ClearFromPipeline();
		private:
			DxResPtr<ID3D11PixelShader>   m_pixelShader;
		};
	}
}